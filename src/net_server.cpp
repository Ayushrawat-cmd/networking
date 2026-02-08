#include<iostream>
#include "olc_net.h"

enum class CustomMsgTypes : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage,
};


class CustomServer: public olc::net::ServerInterface<CustomMsgTypes>
{
    public:
        CustomServer(uint16_t port): olc::net::ServerInterface<CustomMsgTypes>(port){}
    public:
        virtual bool onClientConnect(std::shared_ptr<olc::net::Connection<CustomMsgTypes>> client){
            olc::net::message<CustomMsgTypes> msg;
            msg.header.id = CustomMsgTypes::ServerAccept;
            std::cout<<msg<<std::endl;
            client->send(msg);
            return true;
        }
        virtual void onClientDisconnect(std::shared_ptr<olc::net::Connection<CustomMsgTypes>> client){
            std::cout<<"Client Disconnected ["<<client->GetId() <<"]"<<std::endl;
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
            case CustomMsgTypes::MessageAll:
            {
                std::cout<<"["<<client->GetId()<<"]: message all"<<std::endl;
                // bounce back to client
                olc::net::message<CustomMsgTypes> new_msg;
                new_msg.header.id = CustomMsgTypes::ServerMessage;
                new_msg<<client->GetId();
                messageAllClients(new_msg, client);
            }
                /* code */
                break;
            case CustomMsgTypes::ServerMessage:
            {
                std::cout<<"["<<client->GetId()<<"]: server message"<<std::endl;
                // bounce back to client
                messageAllClients(msg, client);
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