#pragma once

#include "net_common.h"
#include "net_message.h"
#include "net_connection.h"
#include "net_tsqueue.h"

namespace olc{
    namespace net{
        template<typename T>
        class ServerInterface{
            protected:
                //thread safe queue for incoming message packets
                Tsqueue<owned_message<T>> m_recvQueue;

                //container of all connections
                // std::deque<std::shared_ptr<Connection<T>>> m_deqconnections;
                std::unordered_map<uint8_t,std::shared_ptr<Connection<T>>>m_mapconnections;
                
                // ordered of declaration is important - it is also the order of initialization 
                boost::asio::io_context m_ioContext;
                std::thread m_threadContext;

                boost::asio::ip::tcp::acceptor m_acceptor;

                //clients will be identified in the wider system via an id
                uint32_t nIdCounter = 10000;

            public:
                ServerInterface(uint16_t port)
                    :m_acceptor(m_ioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)){ // accept connection from version 4 ip add, port
                }
                virtual ~ServerInterface(){
                    stop();
                }   
                bool start(){
                    try{
                        waitForClientConnection(); // this will run on another thread
                        m_threadContext = std::thread([this](){m_ioContext.run();}); // run the asio context basically infinite loop
                    }
                    catch(const std::exception& e){
                        std::cerr<<"Server Exception: "<< e.what()<<std::endl;
                        return false;
                    }
                    std::cout<<"Server started on port: "<<m_acceptor.local_endpoint().port()<<""<<std::endl;
                    return true; 
                }  

                void stop(){
                    m_ioContext.stop();
                    // tidy way to stop the context thread
                    if(m_threadContext.joinable())
                        m_threadContext.join();
                    
                    std::cout<<"Server stopped"<<std::endl;
                }

                //async - instruct asio to wait for connection 
                void waitForClientConnection(){
                    m_acceptor.async_accept(
                        [this](std::error_code ec, boost::asio::ip::tcp::socket socket){
                            if(!ec){
                                std::cout<<"Client connected: "<<" " <<socket.remote_endpoint() <<std::endl;
                                std::shared_ptr<Connection<T>> newconn = std::make_shared<Connection<T>>(Connection<T>::owner::server, m_ioContext, std::move(socket),m_recvQueue); // new connection by client

                                // give the user server a chance to deny connection
                                if(onClientConnect(newconn)){
                                    // m_deqconnections.push_back(std::move(newconn));
                                    m_mapconnections[nIdCounter] = std::move(newconn);
                                    m_mapconnections[nIdCounter]->connectToClient(this,nIdCounter++);
                                    // m_deqconnections.back()->connectToClient(nIdCounter++);
                                    std::cout<<"["<<m_mapconnections[nIdCounter-1]->GetId()<<"] Connection approved"<<std::endl;
                                }
                            }
                            else{
                                std::cerr<<"accept failed: "<<ec.message()<<std::endl;
                            }
                            waitForClientConnection();
                        }
                    );
                }
                //send a message to a specific client
                void messageClient(std::shared_ptr<Connection<T>> client, const message<T>& msg){
                    if(client && client->isConnected()){
                        client->send(msg);
                    }
                    else{
                        onClientDisconnect(client);
                        client.reset();
                        m_mapconnections.erase(client->GetId());
                        // m_deqconnections.erase(std::remove(m_deqconnections.begin(), m_deqconnections.end(), client), m_deqconnections.end());
                    }
                }
                //send message to all clients
                void messageAllClients(const message<T>& msg, std::shared_ptr<Connection<T>> except = nullptr){
                    bool bInvalidClientExists = false;
                    for(auto& client : m_mapconnections){
                        if (client.second->isConnected()){
                            if(client.second != except){
                                client.second->send(msg);
                            }

                        }
                        else{
                            bInvalidClientExists = true;
                            onClientDisconnect(client.second);
                            client.second.reset();
                            m_mapconnections.erase(client.second->GetId());
                            
                        }
                    }

                    // if(bInvalidClientExists){
                    //     m_deqconnections.erase(std::remove(m_deqconnections.begin(), m_deqconnections.end(), nullptr), m_deqconnections.end());// remove null elements
                    // }

                }

                // constraint the number of messsage can be done in one go
                void update(size_t maxMessages = -1, bool bWait = true){
                    // if(bWait){

                    //     m_recvQueue.wait();
                    // }
                    if (bWait) m_recvQueue.wait(); // big adv. of reducing cpu utilisagtion by making thread to sleep if no message
                    size_t nMessageCount =0;
    
                    while(nMessageCount < maxMessages && !m_recvQueue.empty()){
                        auto msg = m_recvQueue.pop_front();
                        // pass to message handler
                        onMessage(msg.remote, msg.msg);
                        nMessageCount++;
                    }
                }
            protected:
                // called when a client connects
                virtual bool onClientConnect(std::shared_ptr<Connection<T>> client){
                    return false;
                }

                // called when a client disconnects
                virtual void onClientDisconnect(std::shared_ptr<Connection<T>> client){
                    
                }

                // called when a message is received
                virtual void onMessage(std::shared_ptr<Connection<T>> client, message<T>& msg){
                    
                }
            public:
                virtual void onClientValidated(std::shared_ptr<Connection<T>> client){
                       
                }
            
            

        };
    }
}