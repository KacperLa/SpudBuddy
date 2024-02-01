#include "joystick.h"

ronThread::ronThread(const std::string name, Log* logger) : 
    threadName(name),
    logger(logger)
    {}

ronThread::~ronThread() {}

void ronThread::log(std::string strToLog) {
    std::cout << "[" << threadName << "] " << strToLog << std::endl;
    // logger->pushEvent("[" + threadName + "] " +  strToLog);
}

void* ronThread::loopParent(void* arg) {
    // apply signal mask
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);
    
    ronThread* self = static_cast<ronThread*>(arg);
    self->log("thread has started.");
    self->loop();
    self->log("thread has stopped.");
    return NULL;
}

void ronThread::startThread() {
    // Set the running flag to true
    running.store(true, std::memory_order_relaxed);
    // Create a new thread   
    pthread_t object_thread;
    if (pthread_create(&object_thread, NULL, loopParent, this))
    {
        fprintf(stderr, "Error creating thread\n");
    }
    pthread_setname_np(object_thread, threadName.c_str());

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