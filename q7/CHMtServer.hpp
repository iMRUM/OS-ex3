#ifndef CHMTSERVER_HPP
#define CHMTSERVER_HPP
#include "../utils/CHServer.hpp"
#include <map>
#include <mutex>
#include <thread>
std::map<int, std::thread> threads;
std::mutex mtx;
#endif //CHMTSERVER_HPP
