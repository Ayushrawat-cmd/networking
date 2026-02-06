#include<iostream>
#include "olc_net.h"

enum class CustomMsgTypes: uint32_t
{   
    ServerAccept,
    ServerDeny,
    ServerPing,
    MessageAll, ServerMessage
};

class CustomServer: public olc::net::ServerInterface<CustomMsgTypes>
{
    public:
        CustomServer(uint16_t port): olc::net::ServerInterface<CustomMsgTypes>(port){}
    public:
        virtual bool onClientConnect(std::shared_ptr<olc::net::Connection<CustomMsgTypes>> client){
            
            return true;
        }
        virtual void onClientDisconnect(std::shared_ptr<olc::net::Connection<CustomMsgTypes>> client){
            std::cout<<"Client Disconnected"<<std::endl;
        }
        virtual void onMessage(std::shared_ptr<olc::net::Connection<CustomMsgTypes>> client,  olc::net::message<CustomMsgTypes>& msg){
            
            switch (msg.header.id)
            {
            case CustomMsgTypes::ServerPing:
            {
                std::cout<<"["<<client->GetId()<<"]: server ping"<<std::endl;
                
                // bounce back to client
                client->send(msg);
            }
                /* code */
                break;
            
            default:
                break;
            }
        }
};

int main(){
    CustomServer server(6000);
    server.start();
    while(1){
        // std::cout<<"Server Running"<<std::endl;
        server.update(10,true);
    }
    return 0;
}