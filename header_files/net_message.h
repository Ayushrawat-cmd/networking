#pragma once
#include "net_common.h"

namespace olc{
    namespace net{

        //Message Header is sent at start of every message. The template allows us to use different types of headers
        template<typename T>
        struct message_header{
            T id{}; // set default value
            uint32_t size = 0; // set 32 bits... the size should never change
        };

        template <typename T>
        struct message{
            message_header<T> header;// message header of any type
            std::vector<uint8_t> body;

            //return size of entire mesage packet in bytes
            size_t size() const{
                return body.size();
            }

            //override the << operator
            friend std::ostream& operator<<(std::ostream& os, const message<T>& msg){
                return os << "ID: "<< int(msg.header.id) << " Size: " << msg.header.size;
            }

            // pushes any POD-like data into the message buffer
            template <typename DataType>
            friend message<T> & operator<< (message<T>& msg,const DataType& data){
                static_assert(std::is_standard_layout<DataType>::value, "DataType must be standard layout");   // check if DataType is standard layout

                // cache current size of vector, as this will be the point we insert the data
                size_t i = msg.body.size();

                //resize the vector by the size of DataType being pushed
                msg.body.resize(msg.body.size() + sizeof(DataType));

                //physically copy the data into the buffer
                std::memcpy(msg.body.data() + i, &data, sizeof(DataType)); // stores the address of the data into the vector

                
                // recalculate the size of the message and save into header size
                msg.header.size = msg.size();
                
                return msg;
                // Advantage here is that it handles most of the datatypes in the vector
                // disadvantage the vector will have to be resized to fit the data
            }

            template<typename DataType>
            friend message<T> &operator>>(message<T>& msg, DataType& data){
                static_assert(std::is_standard_layout<DataType>::value, "DataType must be standard layout");   // check if DataType is standard layout

                // cache the location towards the end of the vector where the pulled data starts
                size_t i  = msg.body.size() - sizeof(DataType);

                //phyciall copy the data from the vector into the user variable
                std::memcpy(&data, msg.body.data() + i, sizeof(DataType));
                
                // shrink the vector to remove read bytes, and rest end poisiton not vostly reallocation as the 
                msg.body.resize(i);

                msg.header.size = msg.size();
                return msg;
            }
        };

        //forward declare the connection
        template <typename T>
        class Connection ;

        template <typename T>
        struct owned_message
        {
            std::shared_ptr<Connection<T>> remote = nullptr; // pointer to remote connection
            message<T> msg; // encapsulate the message

            friend std::ostream& operator<<(std::ostream& os, const owned_message<T>& msg){
                return os << msg.msg;
            }
        };
    }
}