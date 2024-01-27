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
#include <shared_mutex>
#include <condition_variable>

#include <linux/futex.h>
#include <sys/syscall.h>

#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/interprocess_sharable_mutex.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>


// import boost chrone
#include <boost/chrono.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <ronThread.h>

#include <loggerSimple.h>

namespace bip = boost::interprocess;

template <typename T>
class SData : public ronThread
{
public:
    SData(const std::string name, Log* logger, const std::string& mapped_file, bool isProducer);
    
    SData(const std::string& mapped_file, bool isProducer);

    virtual ~SData();

    void getData(T& data);

    void setData(const T& data);

protected:
    virtual void loop() override;
    bool openMap();

    bool isDataNew(T& data);

    void closeMap();
    void producer();
    void consumer();

    bool futexWait(int* addr, int val, int timeoutSeconds);
    void futexWakeAll(int* addr);

    // struct of shared data
    struct shared_data_t {
        std::atomic<int> index{0};
        int futex = 0;
        T data[2];
        // shared mutex
        boost::interprocess::interprocess_sharable_mutex mutex;
    };

    // shared memory
    std::string mapped_file;
    int fd;

    // Semaphore for shared memory
    std::string semaphore_file;
    sem_t* semaphore;

    // Shared data structure type from template
    shared_data_t* shared_data;

    // Private instace of shared data
    T data_private;
    
    // isProducer flag
    bool isProducer;
    
    LogSimple* simplelog;
    
    // conditional variable
    std::condition_variable cv;

    // newData flag
    bool newData = false;

    // local index
    int index{0};

    const std::int64_t time_to_sleep_ms {100};
};

#include "sdata.cpp"
#endif // SDATA_H