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

    bool isMemoryMapped();

protected:
    virtual void loop() override;
    bool openMap();

    bool isDataNew();

    void closeMap();
    void producer();
    void consumer();

    bool futexWait(std::atomic<int>* addr, int val, int timeoutMilliseconds);
    void futexWakeAll(std::atomic<int>* addr);

    // struct of shared data
    struct shared_data_t {
        std::atomic<int> producer_futex;
        std::atomic<int> consumer_futex;
        T triple_buffer[3];
        std::atomic<std::uint8_t> buffer_index[3];
        // buffer index: 1 = producer 
        // buffer index: 2 = producer
        // depending on the current index
        // the producer will write to one
        // of the two producer buffers
        // index   % 2     = active producer buffer
        // index+1 % 2     = inactive producer buffer
        // buffer_index: 3 = consumer
        // shared mutex
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

    const std::int64_t time_to_sleep_ms {100};
};

#include "sdata.cpp"
#endif // SDATA_H