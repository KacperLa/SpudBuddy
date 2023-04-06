#include <Logging.h>

Log::Log(zmq::context_t& ctx) :
    context(ctx)
{

}

Log::~Log(){

}

bool Log::open(){
    // Open and bind zmq socket
    socket_pub.bind(socket_pub_address);
    socket_pub.set(zmq::sockopt::linger, 0);
    return 0;
}

void Log::close(){
    std::cout << "[LOG] closing socket..." << std::endl;
    socket_pub.close();
}

bool Log::pullEvent(std::string& data){
    return log_queue.wait_dequeue_timed(data, std::chrono::milliseconds(1000));
}

void Log::pushEvent(std::string  data){
    log_queue.enqueue(data); 
    std::cout << data << std::endl;
}

bool Log::publish_message(std::string message){
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double>>(duration).count();
    
    json j = {{"log", message}, {"timestamp", timestamp}};
    std::string out_str = j.dump();
    zmq::message_t out_msg(out_str.c_str(), out_str.size());
    try {
        socket_pub.send(out_msg);
        return 0;
    } catch (const zmq::error_t& e) {
        if (e.num() == EINTR) {
            // System call was interrupted, retry the operation
            return 1;
        } else {
            // Some other error occurred, handle it appropriately
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
    }
}

void Log::processEventLoop(){
    sigset_t set;
    sigfillset(&set);
    pthread_sigmask(SIG_BLOCK, &set, nullptr);
    
    if (open()) {
        std::cerr << "Error opening zmq socket" << std::endl;
        running.store(false, std::memory_order_relaxed);
    }

    // Read the queue events
    while (running.load(std::memory_order_relaxed)) {
        std::string message;

        if (pullEvent(message)) {
           publish_message(message);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    // Close the zmq socket
    close();
}

void Log::startProcessing(){
    // Create a new thread to read the log events
    process_thread = std::thread([this](){ this->processEventLoop(); });

    // Set the running flag to true
    running.store(true, std::memory_order_relaxed);
}

void Log::stopProcessing(){
    // Set the running flag to false
    running.store(false, std::memory_order_relaxed);
    std::cout << "[LOG] joining thread..." << std::endl;

    // Wait for the thread to finish
    if (process_thread.joinable()) {
        std::cout << "[LOG] thread is still running." << std::endl;
        process_thread.join();
    } else {
        std::cout << "[LOG] thread is already dead." << std::endl;
    }

    std::cout << "[LOG] thread has been joined." << std::endl;
}
