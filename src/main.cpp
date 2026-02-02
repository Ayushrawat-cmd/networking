#include "net_common.h"


std::vector<char>vBuffer(1*512);

void asyncReadSomeData(boost::asio::ip::tcp::socket &socket){
    socket.async_read_some(boost::asio::buffer(vBuffer.data(), vBuffer.size()), [&](std::error_code ec, std::size_t length){
        // std::cout<<"rawat"<<std::endl;
        if(!ec){
            std::cout<<std::endl<<  std::endl<<"Reading "<<length<<std::endl<<std::endl<<std::endl;
            for (int i =0; i<length; i++){
                std::cout<<vBuffer.at(i);
            }
            asyncReadSomeData(socket);
        }
        else
        {
            std::cerr<<ec.message()<<std::endl;
        }
    });
}

int main(){
    boost::system::error_code ec;
    boost::asio::io_context io_context; // instance of asio

    // give some fake task to asio to finish
    // boost::asio::io_context::work idlework(io_context);

    // the main thread blockin while asynchronous operation is running so we have to create another thread for main context running 
    // std::thread thr_io_context = std::thread([&io_context](){io_context.run();});
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address("51.38.81.49", ec), 80); // creating the endpoint 
    boost::asio::ip::tcp::socket socket = boost::asio::ip::tcp::socket(io_context); // creating the socket
    socket.connect(endpoint, ec); // connecting to the endpoint
    if (ec)
    {
        std::cerr << "Connection failed: " << ec.message() << "\n";
        return 1;
    }
    else{
        std::cout << "Connected\n";
    }
    if(socket.is_open()){
        
        std::string request =
        "GET / HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Connection: close\r\n"
        "\r\n";

        socket.write_some(boost::asio::buffer(request.data(), request.size()), ec); // sending the request in the form of packets in buffer and it will send back error or response

        // socket.wait(socket.wait_read);

        // size_t bytes=socket.available(); // if it send back the response then it will return the number of bytes
        // std::cout<<bytes<<std::endl;
        // std::cout<<bytes<<std::endl;
        // if(bytes>0){
        //     std::vector<char>buffer (bytes);
        //     socket.read_some(boost::asio::buffer(buffer.data(), buffer.size()), ec); // it is in synchronous mode
        //     for(char c : buffer){
        //         std::cout<<c;
        //     }

        // }
        // using namespace std::chrono_literals;
        // std::this_thread::sleep_for(2000ms);
        asyncReadSomeData(socket);
        io_context.run();

    }
    // system("pause");
    
    return 0;
}

// #include <boost/asio.hpp>
// #include <iostream>
// #include <vector>

// int main() {
//     boost::asio::io_context io;
//     boost::system::error_code ec;

//     boost::asio::ip::tcp::socket socket(io);
//     boost::asio::ip::tcp::endpoint ep(
//         boost::asio::ip::make_address("127.0.0.1"), 8080);

//     socket.connect(ep, ec);
//     if (ec) {
//         std::cerr << "Connect failed: " << ec.message() << "\n";
//         return 1;
//     }

//     std::cout << "Connected\n";

//     std::string req =
//         "GET / HTTP/1.1\r\n"
//         "Host: localhost\r\n"
//         "Connection: close\r\n"
//         "\r\n";

//     boost::asio::write(socket, boost::asio::buffer(req), ec);

//     std::vector<char> buf(1024);
//     while (true) {
//         size_t n = socket.read_some(boost::asio::buffer(buf), ec);

//         if (ec == boost::asio::error::eof)
//             break; // server closed connection

//         if (ec) {
//             std::cerr << "Read error: " << ec.message() << "\n";
//             break;
//         }

//         std::cout.write(buf.data(), n);
//     }

//     return 0;
// }