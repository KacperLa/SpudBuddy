#ifndef SDATA_H
#define SDATA_H

#include <string>
#include <cstring>
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
#include <shared_mutex>
#include <condition_variable>

#include <linux/futex.h>
#include <sys/syscall.h>
#include <sys/stat.h>

#include <boost/interprocess/sync/interprocess_sharable_mutex.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>

#include <shared_structs.h>

// import boost chrone
#include <boost/chrono.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <loggerSimple.h>

namespace bip = boost::interprocess;

template <typename T>
class SData
{
public:
    SData(Log* logger, const std::string& mapped_file, bool isProducer);
    
    SData(const std::string& mapped_file, bool isProducer);

    virtual ~SData();

    bool getData(T* data);
    bool waitOnStateChange(T* data);

    void setData(const T* data);
    T* getBuffer();
    int getBufferIndex();
    void trigger();

    bool isMemoryMapped();

protected:
    bool openMap();
    void log(std::string event);

    bool isDataNew();

    void closeMap();

    bool futexWait(std::atomic<int>* addr, int val);
    void futexWakeAll(std::atomic<int>* addr);
    
    struct timespec timeout {0, 100000000}; // 100 milliseconds

    // struct of shared data
    struct shared_data_t {
        std::atomic<int> producer_futex;
        std::atomic<int> consumer_futex;
        T triple_buffer[3];
      
        boost::interprocess::interprocess_sharable_mutex mutex;
    };

    // shared memory
    std::string mapped_file;
    int fd;
    std::atomic<bool> memory_mapped;

    std::atomic<std::uint8_t> local_data_new_index;
    T local_data[2] {T(), T()};

    // Shared data structure type from template
    shared_data_t* shared_data;

    // isProducer flag
    bool isProducer;
    
    LogSimple* simplelog;
    
    // conditional variable
    std::condition_variable cv;

    // newData flag
    std::atomic<bool> m_new_data;

    // local index
    int index{0};
};

#include "sdata.cpp"
#endif // SDATA_H