#pragma once
#include "net_common.h"
#include "net_message.h"
#include "net_connection.h"
#include "net_tsqueue.h"


namespace olc{
    namespace net{
        template<typename T>
        class ClientInterface{
            public:

            ClientInterface()
            {
                // initialize the socket with io context, so it can do stuff
            }
            virtual ~ClientInterface(){
                disconnect();
            }

            public:
                // connecto to server with hostname . ip-address and port
                bool connect(const std::string &host, const uint16_t port){
                    try{
                        //resolve hostname into tangiable physcial address
                        boost::asio::ip::tcp::resolver resolver(m_ioContext);
                        boost::asio::ip::tcp::resolver::results_type m_endpoints = resolver.resolve(host, std::to_string(port));
                        //create connection
                        m_connection = std::make_unique<Connection<T>>(
                            Connection<T>::owner::client, m_ioContext, boost::asio::ip::tcp::socket(m_ioContext), m_recvQueue
                        ); // todo
                        // std::cout<<"Client: "<<m_connection->GetId()<<std::endl;
                        m_connection->connectToServer(m_endpoints);
                        thr_context = std::thread([this](){m_ioContext.run();}); 

                        
                        // //resolve the hostname/ ipaddress into tangiable physical address
                        // boost::asio::ip::tcp::resolver resolver(m_ioContext);
                        // boost::asio::ip::tcp::resolver::results_type m_endpoints = resolver.resolve(host, std::to_string(port));
                        
                        //tell the connection to connect to the server
                        // m_connection->connect(m_endpoints);
                    }
                    catch(const std::exception& e){
                        std::cerr<<"Client Exception: "<< e.what()<<std::endl;
                        return false;
                    }
                    return true;

                }

                //disconnect from the server
                void disconnect(){
                    if(isConnected()){
                        m_connection->disconnect();
                    }
                    //either way, we are also done with asio context...
                    m_ioContext.stop();
                    //join the thread
                    if(thr_context.joinable())
                        thr_context.join();
                    
                    m_connection.release();
                }

                //check if client is acctually connecto a server
                bool isConnected() const{
                    if(m_connection)
                        return m_connection->isConnected();
                    return false;
                }
                
                void send(const message<T>& msg){
                    if(!isConnected())
                        return;
                    m_connection->send(msg);
                }
                Tsqueue<owned_message<T>>& getRecvQueue(){
                    return m_recvQueue;
                }

            protected:
                //asio context handles the data transfer
                boost::asio::io_context m_ioContext;
                // needs a thread of it onw to execute its work commands
                std::thread thr_context;
                // this is the hardware socket that is connected to the server
                // boost::asio::ip::tcp::socket m_socket;
                // this is the connection to the server which is a single connection object
                std::unique_ptr<Connection<T>> m_connection;
                
            private:
                Tsqueue<owned_message<T>> m_recvQueue;
        };
    }
}

