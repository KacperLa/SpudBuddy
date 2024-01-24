#include "joystick.h"

ronThread::ronThread(const std::string name, Log* logger) : 
    threadName(name),
    logger(logger)
    {}

ronThread::~ronThread() {}

void ronThread::log(std::string strToLog) {
    std::cout << "[" << threadName << "] " << strToLog << std::endl;
    // logger.pushEvent("[" + threadName + "] " +  strToLog);
}

void ronThread::startThread() {
    // Set the running flag to true
    running.store(true, std::memory_order_relaxed);
    // Create a new thread
    object_thread = std::thread([this](){ this->loop(); });
    log("thread has started.");
}

void ronThread::stopThread() {
    // Set the running flag to false
    running.store(false, std::memory_order_relaxed);
    log("joining thread...");

    // Wait for the thread to finish
    if (object_thread.joinable()) {
        log("thread is still running.");
        object_thread.join();
    } else {
        log("thread is already dead.");
    }
    log("thread has been joined.");
}