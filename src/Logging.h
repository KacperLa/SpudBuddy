#ifndef LOGGING_H
#define LOGGING_H


#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <mutex>
#include <thread>
#include <vector>    
#include <chrono>

#include <csignal>

#include <libraries/json/json.hpp>
using json = nlohmann::json;

#include <msgpack11.hpp>


#include <chrono>
#include <string>
#include <atomic>

#include <fcntl.h>
#include <unistd.h>

#include <libraries/readerwriterqueue/readerwriterqueue.h>
#include <libraries/readerwriterqueue/atomicops.h>

using namespace moodycamel;
using namespace msgpack11;

struct LogMessage {
    std::string log;
    double timestamp;
};

class Log
{
public:
    Log();
    ~Log();

    bool open();

    void pushEvent(std::string  data);

    void startThread();
    void stopThread();

protected:
    bool pullEvent(std::string& data);
    bool publishMessage(const std::string& message);

    void closeSocket();
    void loop();

    BlockingReaderWriterQueue<std::string> log_queue{1000}; 

    struct sockaddr_in pub_address;

    int socket_pub;

    static constexpr int PORT    = 3000;  // Choose a port number
    static constexpr int BACKLOG = 5;     // Maximum number of pending connections

    const std::string LOG_SERVER_IP = "0.0.0.0";  // Set your server's IP address here
    
    std::mutex thread_lock;

    std::atomic<bool> running {false};
    std::thread object_thread;
};

#endif