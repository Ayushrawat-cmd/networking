#include<iostream>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include "olc_net.h"
/*
server asio
asio context will wait for the connection to establish and accept the connection and socket connection will be created  and also can reject the connection
if accepth the connection then contain the socket connection in server
and also can read the header and body of the message of ay connection 

we know the size of the header and read of header expires then replace it with read body will start and its all coming incoming queue
*/


enum class CustomMsgTypes : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage,
};


class CustomClient : public olc::net::ClientInterface<CustomMsgTypes>{
    // public:
    //     bool fireBullet(float x, float y){
    //         olc::net::message<CustomMsgTypes> msg;
    //         msg.header.id = CustomMsgTypes::FireBullet;
    //         msg<<x<<y;
    //         return send(msg);
            
    //     }
    public:
        void pingServer(){
            std::cout<<"pinging server"<<std::endl;
            olc::net::message<CustomMsgTypes> msg;
            msg.header.id = CustomMsgTypes::ServerPing;

            std::chrono::system_clock::time_point time_now = std::chrono::system_clock::now(); // to calc round trip time

            msg<<time_now;
            send(msg);
        }

};
int readKey()
{
    char ch;
    if (read(STDIN_FILENO, &ch, 1) > 0)
        return ch;
    return -1;
}



int main(){
    CustomClient c;
    c.connect("127.0.0.1", 6000);
    // c.fireBullet(2.0f, 3.0f);
    bool bQuit = false;
    bool key[3] = {false, false, false};
    bool old_key[3] = {false, false, false};
    while(!bQuit){
        int key = readKey();
        if(key == '1'){
            c.pingServer();
        }
        if(key == '3'){
            bQuit = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        // c.pingServer();
        // if(GetForegroundWindow() == GetConsoleWindow()){
        //     key[0] = GetAsyncKeyState('1') & 0x8000;
        //     key[1] = GetAsyncKeyState('2') & 0x8000;
        //     key[2] = GetAsyncKeyState('3') & 0x8000;
        // }
        // if(key[0] && !old_key[0]){
        //     c.PingServer();
        // }
        // if(key[2] && !old_key[2]){
        //     bQuit = true;
        // }
        // for(int i =0; i<3; i++){
        //     old_key[i] = key[i];
        // }
        if(c.isConnected()){
            if(!c.getRecvQueue().empty()){
                auto msg = c.getRecvQueue().pop_front().msg;
                switch (msg.header.id)
                {
                case CustomMsgTypes::ServerPing:{
                    std::chrono::system_clock::time_point time_now = std::chrono::system_clock::now(); // to calc round trip time
                    std::chrono::system_clock::time_point time_then;
                    msg>>time_then;
                    printf("Round trip time: %2f ms\n",std::chrono::duration<double, std::milli>(time_now- time_then).count());
                }
                    /* code */
                    break;
                
                default:
                    break;
                }
            }
        }
        else{
            bQuit = true;
            std::cout<<"Server down"<<std::endl;
        }
    }
    return 0;

    // olc::net::message<CustomMsgTypes> msg;
    // msg.header.id = CustomMsgTypes::FireBullet;
    // int a = 1;
    // bool b = true;
    // float c = 3.14159f;
    // struct {
    //     float x;
    //     float y;
    // }d[5];
    // // std::cout<<msg<<std::endl;
    // msg<<a<<b<<c<<d;
    // // msg<<a<<b<<c<<d;
    // std::cout<<msg<<std::endl;
    // // for(auto i: msg.body){
    // //     std::cout<<i;
    // // }
    // a=99;
    // b=false;
    // c=3.149f;
    // // msg<<a<<b<<c;
    // // for(auto i: msg.body){
    // //     std::cout<<i;
    // // }
    // // cout<<
    // msg>>d>>c>>b>>a;
    // std::cout<<" "<<a<<" "<<b<<" "<<c<<std::endl;
    // return 0;
}