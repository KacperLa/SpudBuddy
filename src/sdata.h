#ifndef SDATA_H
#define SDATA_H

#include <string>
#include <iostream>
#include <mutex>
#include <atomic>
#include <chrono>
#include "fcntl.h"
#include "unistd.h"
#include <poll.h>
#include <vector>
#include <sys/mman.h>
#include <semaphore.h>

#include<ronThread.h>

template <typename T>
class SData : public ronThread
{
public:
    SData(const std::string name, Log& logger, const std::string& mapped_file, const std::string& semaphore_file, bool isProducer);
    
    bool getData(T& data);

    void setData(T data);

protected:
    virtual void loop() override;
    bool openMap();
    bool dataReady();

    void closeMap();
    void producer();
    void consumer();

    // shared memory
    std::string mapped_file;
    int fd;

    // Semaphore for shared memory
    std::string semaphore_file;
    sem_t* semaphore;

    // Shared data structure type from template
    T* shared_data;

    // Private instace of shared data
    T data_private;
   
    // isProducer flag
    bool isProducer;
    
    // newData flag
    bool newData = false;

    const std::int64_t time_to_sleep{10};
};

#include "sdata.cpp"
#endif // SDATA_H