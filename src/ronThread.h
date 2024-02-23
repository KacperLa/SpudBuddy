#ifndef RONTHREAD_H
#define RONTHREAD_H

#include <string>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include "fcntl.h"
#include "unistd.h"
#include <signal.h>
#include <logger.h>

class ronThread {
public:
    explicit ronThread(const std::string name, Log* logger, bool realTime);
    virtual ~ronThread();
    
    void startThread();
    void stopThread();
protected:
    virtual void loop() = 0;

    static void* loopParent(void* arg);

    void log(std::string strToLog);
    
    std::string threadName;

    std::mutex thread_lock;

    Log* logger;

    bool realTime;

    std::atomic<bool> running {false};
    std::thread object_thread;
};

#endif // RONTHREAD_H