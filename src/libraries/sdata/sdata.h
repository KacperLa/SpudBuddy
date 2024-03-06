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


class MappedBuffer
{
public:    
    MappedBuffer(const std::string mapped_file, bool isProducer, std::size_t blob_size);

    virtual ~MappedBuffer();

    bool getData(void *data);
    bool waitOnStateChange(void *data);

    void setData(const void *data);

    bool isMemoryMapped();

protected:
    bool openMap();

    bool isDataNew();

    void closeMap();
    void producer();
    void consumer();
    void log(std::string message);

    bool futexWait(std::atomic<int>* addr, int val);
    void futexWakeAll(std::atomic<int>* addr);
    
    struct timespec timeout {0, 100000000}; // 100 milliseconds

    // struct of shared data
    struct shared_data_t {
        std::atomic<int> producer_futex;
        };

    // shared memory
    std::string mapped_file;
    int fd;
    std::atomic<bool> memory_mapped;

    // size of shared structure
    std::size_t blob_size;

    // pointer to each buffer for local data
    void* m_data[3];

    std::atomic<std::uint8_t> local_data_new_index;

    // Shared data structure type from template
    shared_data_t* shared_data;
    std::size_t map_size;

    // isProducer flag
    bool isProducer;
        
    // conditional variable
    std::condition_variable cv;

    // newData flag
    std::atomic<bool> m_new_data;

    // local index
    int index{0};
};

template <typename T>
class SData
{
public:
    SData(const std::string mapped_file, bool isProducer) :
        mapped_file(mapped_file),
        isProducer(isProducer)
    {
    }

    void setData(T *data)
    {
        sdata->setData(static_cast<void*>(data));
    }

    bool getData(T *data)
    {
        return sdata->getData(static_cast<void*>(data));
    }

    bool waitOnStateChange(T *data)
    {
        return sdata->waitOnStateChange(static_cast<void*>(data));
    }

    bool isMemoryMapped()
    {
        return sdata->isMemoryMapped();
    }

private:
    const std::string mapped_file;
    const bool isProducer;
    MappedBuffer *sdata = new MappedBuffer(mapped_file, isProducer, sizeof(T));
};
#endif // SDATA_H