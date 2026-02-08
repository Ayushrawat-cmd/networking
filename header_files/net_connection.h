#pragma once
#include "net_common.h"
#include "net_message.h"
#include "net_tsqueue.h"  

namespace olc
{
    namespace net
    {
        template<typename T>
        class ServerInterface;

        template <typename T>
        class Connection : public std::enable_shared_from_this<Connection<T>>{ // it bsically allows the make the shared pointer internally
            public:
            enum class owner{
                server, 
                client
            };
            Connection(owner parent, boost::asio::io_context& ioContext, boost::asio::ip::tcp::socket socket, Tsqueue<owned_message<T>>& recvQueue):m_socket(std::move(socket)), m_ioContext(ioContext), m_recvQueue(recvQueue){
                m_nOwnerType = parent;

                // validation check
                if(m_nOwnerType == owner::server){
                    m_Handshake_out = std::chrono::system_clock::now().time_since_epoch().count();
                    m_Handshake_check = scramble(m_Handshake_out);
                }
                else{
                    m_Handshake_in = 0;
                    m_Handshake_check = 0;
                }
            }
            virtual ~Connection(){}

            uint32_t GetId() const{
                return id;
            }
            // private:
                // void readValidation(olc::net::ServerInterface<T>*server);

            public:
                void connectToClient(olc::net::ServerInterface<T>* server, uint32_t nId){
                    if(m_nOwnerType == owner::server){
                        if(m_socket.is_open()){
                            id = nId;
                            
                            // a client has attempted to connect to server, but we wishe the client to validate itself
                            // so first handshake data need to be validated
                            writeValidation();

                            // Next, issue a task to sit asynchronously and wait for the client to validate itself and read that data to validate
                            readValidation(server);
                            // readHeader();
                        }
                    }
                }
                void connectToServer(const boost::asio::ip::tcp::resolver::results_type& endpoints){ // call by the clients
                    // std::cout<<m_nOwnerType.name()<<std::endl;
                    if(m_nOwnerType == owner::client){
                        
                        boost::asio::async_connect(m_socket, endpoints, [this](std::error_code ec, boost::asio::ip::tcp::endpoint endpoint){
                            if(!ec){
                                readValidation();
                                // readHeader();
                            }
                            else{
                                std::cerr<<"connect failed: "<<ec.message()<<std::endl;
                            }
                        });
                    }
                }
                void disconnect() {// call by the clients and servers
                    if(isConnected()){
                        boost::asio::post(m_ioContext, [this](){
                            m_socket.close();
                        });
                        // return true;
                    }
                    // return false;
                }
                bool isConnected() const{
                    return m_socket.is_open();
                }
            
            public:
                void send(const message<T>& msg){
                    boost::asio::post(m_ioContext, [this, msg](){
                        // sendInternal(msg);
                        bool bWritingMessage = !m_sendQueue.empty();
                        m_sendQueue.push_back(msg);
                        if(!bWritingMessage){ // if we are not already writing a message to prevent asio workload
                            writeHeader(); // basically restart the process
                        }
                    
                    });
                }

            
            private:
                
                //Encrypt the message
                uint64_t scramble(uint64_t n_input){
                    uint64_t out  = n_input ^ 0xDEADBEEFC0DECAFE;
                    out = (out & 0XF0F0F0F0F0F0F0F0) >> 4 | (out & 0x0F0F0F0F0F0F0F0F) << 4;
                    return out ^ 0xC0DEFACE12345678;

                }
                // used by both client and server to write the validation packet
                void writeValidation(){
                    boost::asio::async_write(m_socket, boost::asio::buffer(&m_Handshake_out, sizeof(uint64_t)),
                    [this](boost::system::error_code ec, std::size_t bytes_transferred){
                        if(!ec){
                            if(m_nOwnerType == owner::client){
                                readHeader();
                            }
                            else{
                                std::cout<<"Handshake sent by server"<<" "<<m_Handshake_out<<std::endl;
                            }
                            // readValidation(client);
                        }
                        else{
                            std::cerr<<"write validation failed: "<<ec.message()<<std::endl;
                            disconnect();
                        }
                    });
                }

                void readValidation( olc::net::ServerInterface<T>* server = nullptr){
                    boost::asio::async_read(m_socket, boost::asio::buffer(&m_Handshake_in, sizeof(uint64_t)),
                    [this, server](boost::system::error_code ec, std::size_t bytes_transferred){
                        if(!ec){
                            if(m_nOwnerType == owner::server){
                                if(m_Handshake_in == m_Handshake_check){
                                    // the incoming data from the client is should be the same checked data stored
                                    std::cout<<"Handshake success"<<std::endl;
                                    server->onClientValidated(this->shared_from_this());

                                    // sit and reading from the header
                                    readHeader();
                                    
                                }
                                else{
                                    std::cerr<<"Handshake failed"<<std::endl;
                                    disconnect();
                                }
                            }
                            else if(m_nOwnerType == owner::client){
                                //connection is a client need to scramble it and send to server
                                std::cout<<"Handshake received by client"<<" "<<m_Handshake_in<<std::endl;
                                m_Handshake_out = scramble(m_Handshake_in);
                                writeValidation();
                            }
                        }
                        else{
                            std::cerr<<"read validation failed: "<<ec.message()<<std::endl;
                            disconnect();
                        }
                    });
                        // }
                    
                }

                // If this function is called, we are expecting asio to wait until it receives
				// enough bytes to form a header of a message. We know the headers are a fixed
				// size, so allocate a transmission buffer large enough to store it. In fact, 
				// we will construct the message in a "temporary" message object as it's 
				// convenient to work with.
                void readHeader(){
                    boost::asio::async_read(m_socket, boost::asio::buffer(&m_recvMessage.header, sizeof(message_header<T>)),
                    [this](boost::system::error_code ec, std::size_t bytes_transferred){
                        if(!ec){
                            if(m_recvMessage.header.size > 0){
                                // ...it does, so allocate enough space in the messages' body
								// vector, and issue asio with the task to read the body.
                                // std::cout<<"Message header received, size: "<<m_recvMessage.header.size<<std::endl;
                                m_recvMessage.body.resize(m_recvMessage.header.size);
                                readBody();
                            }
                            else{
                                addToIncomingMessageQueue();
                            }
                        }
                        else{
                            std::cerr<<"read header failed: "<<ec.message()<<std::endl;
                            disconnect();
                        }
                    });
                }

                void readBody(){
                    // auto self = this->shared_from_this();
                    boost::asio::async_read(m_socket, boost::asio::buffer(m_recvMessage.body.data(), m_recvMessage.body.size()),
                    [this](std::error_code ec, std::size_t bytes_transferred){
                        if(!ec){
                            addToIncomingMessageQueue();
                        }
                        else{
                            std::cerr<<"read body failed: "<<ec.message()<<std::endl;
                            disconnect();
                        }
                    });
                }

                void writeHeader(){
                    boost::asio::async_write(m_socket, boost::asio::buffer(&m_sendQueue.front().header, sizeof(message_header<T>)),
                    [this](boost::system::error_code ec, std::size_t bytes_transferred){
                        if(!ec){
                            if(m_sendQueue.front().body.size() > 0){
                                // std::cout<<"Message header sent, size: "<<m_sendQueue.front().header.size<<std::endl;
                                writeBody();
                                
                            }
                            else{
                                m_sendQueue.pop_front();
                                if(!m_sendQueue.empty()){
                                    writeHeader();
                                }
                            }
                        }
                        else{
                            std::cerr<<"write header failed: "<<ec.message()<<std::endl;
                            disconnect();
                        }
                    });
                }

                void writeBody() {
                    boost::asio::async_write(
                        m_socket, 
                        boost::asio::buffer(m_sendQueue.front().body),
                        [this](boost::system::error_code ec, std::size_t bytes_transferred) {
                            if (!ec) {

                                m_sendQueue.pop_front();
                                if (!m_sendQueue.empty()) {
                                    writeHeader();
                                }
                            }
                            else {
                                std::cerr << "write body failed: " << ec.message() << std::endl;
                                disconnect();
                            }
                        });
                }

                void addToIncomingMessageQueue(){
                    if(m_nOwnerType == owner::server){
                        m_recvQueue.push_back({this->shared_from_this(), m_recvMessage}); // remote socket and message
                    }
                    else{
                        m_recvQueue.push_back({nullptr, m_recvMessage}); // for client
                    }
                    readHeader();
                }
            
            protected:
                // each connection has a unique socket to a remote
                boost::asio::ip::tcp::socket m_socket;

                // this context is shared with the whole asio instance
                boost::asio::io_context& m_ioContext;

                // this queue holds all messages to be send to the remote side of this connection basically to the client side
                Tsqueue<message<T>> m_sendQueue;                

                //this queue holds all the messages that have been recieved from the remote side of this connections. note it is a reference as the "owner" of this connection is expected to provide a queue
                Tsqueue<owned_message<T>>& m_recvQueue;

                message<T> m_recvMessage;

                owner m_nOwnerType = owner::server;

                uint32_t id = 0;

                //Handshake validation
                uint64_t m_Handshake_out = 0;
                uint64_t m_Handshake_in = 0;
                uint64_t m_Handshake_check = 0;

            
        };
        
    }
}