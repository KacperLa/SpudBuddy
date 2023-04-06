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

#include<Logging.h>


class ronThread {
public:
    explicit ronThread(const std::string name, Log & logger);
    virtual ~ronThread();
    

    void startThread();
    void stopThread();
protected:
    virtual void loop() = 0;

    void log(std::string strToLog);
    
    std::string threadName;

    Log& logger;

    std::atomic<bool> running {false};
    std::thread object_thread;
};

#endif // RONTHREAD_H