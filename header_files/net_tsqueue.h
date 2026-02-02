#pragma once
#include<mutex>
#include "net_common.h"
#include "net_message.h"


// implementing thread safe queue for different connections and messages pushed to the queue
namespace olc{
    namespace net{
        template<typename T>
        class Tsqueue{
            public:
            Tsqueue() = default;
            Tsqueue(const Tsqueue<T>&) = delete; // disable copy constructor   
            virtual ~Tsqueue() {clear();}
            public:
            // returns and maintains item at front of queue
            const T& front() {
                std::scoped_lock lock(muxQueue);
                return deqQueue.front(); // doubly ended queue
            }

            // return and maintains item at back of queue
            const T& back() {
                std::scoped_lock lock(muxQueue);
                return deqQueue.back();
            }

            //adds and item to the back of the queue
            void push_back(const T& item){
                std::scoped_lock lock(muxQueue);
                deqQueue.emplace_back(std::move(item));
                std::unique_lock<std::mutex> ul(muxBlocking);
				cvBlocking.notify_one();
            }

            // adds and item to the front of the queue
            void push_front(const T& item){
                std::scoped_lock lock(muxQueue);
                deqQueue.emplace_front(std::move(item));

                std::unique_lock<std::mutex> ul(muxBlocking);
				cvBlocking.notify_one();
            }

            //clear the queue
            void clear(){
                std::scoped_lock lock(muxQueue);
                deqQueue.clear();
            }

            // remove and returns item from front of queue
            T pop_front(){
                std::scoped_lock lock(muxQueue);
                T item = std::move(deqQueue.front());
                deqQueue.pop_front();
                return item;
            }

            // remove and returns item from back of queue
            T pop_back(){
                std::scoped_lock lock(muxQueue);
                T item = std::move(deqQueue.back());
                deqQueue.pop_back();
                return item;
            }

            // returns the number of items in the queue
            size_t size() {
                std::scoped_lock lock(muxQueue);
                return deqQueue.size();
            }

            // returns true if the queue is empty
            bool empty() {
                std::scoped_lock lock(muxQueue);
                return deqQueue.empty();
            }
            void wait()
			{
				while (empty())
				{
					std::unique_lock<std::mutex> ul(muxBlocking);
					cvBlocking.wait(ul);
				}
			}

            private:
                std::mutex muxQueue;
                std::deque<T> deqQueue;
                std::condition_variable cvBlocking;
			    std::mutex muxBlocking;
        };

        
    }
}