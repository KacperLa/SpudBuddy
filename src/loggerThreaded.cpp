
#include <loggerThreaded.h>
#include "common.h"

LogThreaded::LogThreaded() : 
    Log()
{
    this->startThread();
    std::cout << "[log] thread has been started." << std::endl; 
}

LogThreaded::~LogThreaded()
{
    this->stopThread();
}

bool LogThreaded::open() {
    socket_pub = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_pub == -1) {
        std::cerr << "[log} Error connecting to server" << std::endl;
        return false;
    }

    // Set up server address
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(LOG_SERVER_IP.c_str());
 
    // Connect to server
    if (connect(socket_pub, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "[log] Error connecting to server" << std::endl;
        return false;
    }

    return true;
}

void LogThreaded::closeSocket() {
    std::cout << "[LOG] closing socket..." << std::endl;
    if (socket_pub != -1) {
        close(socket_pub);
        socket_pub = -1;
    }
}

bool LogThreaded::pullEvent(std::string& data){
    return log_queue.wait_dequeue_timed(data, std::chrono::milliseconds(1000));
}

void LogThreaded::pushEvent(std::string  data){
    // log_queue.enqueue(data);
    std::cout << data << std::endl;
}

bool LogThreaded::publishMessage(const std::string& message) {
    // Serialize the message using MessagePack
    
    // print message locally
    std::cout << message << std::endl;

    MsgPack message_out = MsgPack::object {
        { "data", message },
        { "timestamp", get_time_nano() }
    };

    //serialize
    std::string msgpack_bytes = message_out.dump();
    
    // Check if socket is still open
    if (socket_pub == -1) {
        std::cerr << "[log] Socket is closed" << std::endl;
        return false;
    }

    // Send the serialized message over the socket
    ssize_t bytes_sent = send(socket_pub, msgpack_bytes.data(), msgpack_bytes.size(), 0);
    if (bytes_sent == -1) {
        std::cerr << "Error sending message: " << strerror(errno) << std::endl;
        std::cout << "Size of msgpack_bytes: " << msgpack_bytes.size() << std::endl;
        return false;
    }
    
    return true;
}

void LogThreaded::loop(){
    sigset_t set;
    sigfillset(&set);
    pthread_sigmask(SIG_BLOCK, &set, nullptr);
    
    // if (!open()) {
    //     std::cerr << "[log] Error opening socket" << std::endl;
    //     running.store(false, std::memory_order_relaxed);
    // }

    std::string message;

    // Read the queue events
    while (running.load(std::memory_order_relaxed)) {
        if (pullEvent(message)) {
            std::cout << message << std::endl;
        //    publishMessage(message);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    closeSocket();
}

void LogThreaded::startThread() {
    // Set the running flag to true
    running.store(true, std::memory_order_relaxed);
    // Create a new thread
    object_thread = std::thread([this](){ this->loop(); });
    std::cout << "[log] thread has started." << std::endl;
}

void LogThreaded::stopThread() {
    // Set the running flag to false
    running.store(false, std::memory_order_relaxed);
    std::cout << "[Log] joining thread..." << std::endl;

    // Wait for the thread to finish
    if (object_thread.joinable()) {
        std::cout << "[log] thread is still running." << std::endl;
        object_thread.join();
    } else {
        std::cout << "[log] thread is already dead." << std::endl;
    }
    std::cout << "[log] thread has been joined." << std::endl;
}






