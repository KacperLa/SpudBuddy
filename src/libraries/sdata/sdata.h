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
#include <errno.h>

#include <linux/futex.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <climits>

template <typename T>
class SData
{
public:    
    SData(const std::string mapped_file, bool isProducer) :
        mapped_file(mapped_file),
        memory_mapped(false),
        isProducer(isProducer),
        blob_size(blob_size)
    {
    }

    virtual ~SData();

    template <typename T>
    bool openMap()
    {
        log("Opening memory mapped file");    
        // open the mapped memory file descriptor
        int fd;
        // prepare the file descriptor with /tmp/
        mapped_file = "/tmp/" + mapped_file;

        fd = open(mapped_file.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
   
        if (fd == -1) {
            log("Error opening/creating memory-mapped file");
            return 1;
        }
       
        m_map_size = sizeof(shared_data_t);

        // Check if file needs to be truncated
        struct stat fileStat;
        if (stat(mapped_file.c_str(), &fileStat) == 0)
        {
            off_t currentSize = fileStat.st_size;
            if (currentSize != sizeof(shared_data_t))
            {
                if (ftruncate(fd, m_map_size) == -1)
                {
                    log("Error truncating Memmory Map");
                    close(fd);
                    return true;
                } 
                else
                {
                    log("Memmory Map truncated successfully");
                }
            }
            else
            {
                log("Memmory Map already at the correct size");
            }
        }
        else
        {
            log("Error getting Memmory Map stat");
            close(fd);
            return true;
        }

        // Map the memory
        m_shared_data = static_cast<shared_data_t*>(mmap(NULL, m_map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0u));
        if (m_shared_data == MAP_FAILED) {
            log("Error mapping memory");
            close(fd);
            return true;
        } else {
            log("Memory mapped successfully");
        }

        shared_data->producer_futex.store(1, std::memory_order_release);

        return 0;
    }

    template <typename T>
    bool getData(T * data) {
        int current_buffer_index = ((m_shared_data->producer_futex.load(std::memory_order_acquire) - 1) % 3);
        std::memcpy(data,
                    m_shared_data[current_buffer_index],
                    sizeof(T));
        if (current_buffer_index == (m_shared_data->producer_futex.load(std::memory_order_acquire) % 3))
        {
            log("Buffer index potential corruption");
            return false;
        }
        return true;
    }

    template <typename T>
    void setData(const T* data)
    {
        std::memcpy(m_data[shared_data->producer_futex.load(std::memory_order_acquire)  % 3], 
                    data,
                    sizeof(T));
        shared_data->producer_futex.fetch_add(1, std::memory_order_release);
        // futexWakeAll(&shared_data->producer_futex);
        // trigger an atomic wak eall
    }

    template <typename T>
    bool waitOnStateChange(T * data)
    {
        int current_index = m_shared_data->producer_futex.load(std::memory_order_acquire);
        if (futexWait(&m_shared_data->producer_futex, current_index))
        {
            std::memcpy(data,
                        m_shared_data[current_index % 3],
                        sizeof(T));
            if ((current_index % 3) == (m_shared_data->producer_futex.load(std::memory_order_acquire) % 3))
            {
                log("Buffer index potential corruption");
                return false;
            }
            else
            {
                return true;
            }
        }
        else
        {
            return false;
        }
    }

    T getBuffer()
    {
        return &m_shared_data[m_shared_data->producer_futex.load(std::memory_order_acquire) % 3];
    } 

    void* getBuffer();

    int getBufferIndex()
    {
        return shared_data->producer_futex.load(std::memory_order_acquire) % 3;
    }

    bool isMemoryMapped() {
        return memory_mapped.load(std::memory_order_acquire);
    }

protected:

    void log(std::string message)
    {
        std::cout << message << std::endl;
    }

    void closeMap()
    {
        // close memory mapped file
        if (munmap(m_shared_data, m_map_size) == -1) {
            log("Error un-mmapping the file");
        }
    }

    bool futexWait(std::atomic<int>* addr, int val) {
        int ret;
        do {
            ret = syscall(SYS_futex, addr, FUTEX_WAIT, val, &timeout, NULL, 0);
            if (ret == -1) {
                if (errno == EINTR) {
                    // Interrupted by a signal, continue waiting
                    continue;
                } else if (errno == ETIMEDOUT) {
                    // log("Timeout in futexWait");
                    return false;
                } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // The operation was interrupted, the timeout expired, or the futex was not the expected value
                    log("EAGAIN or EWOULDBLOCK in futexWait");
                    return true;
                } else {
                    // Other futex error occurred
                    log("Error in futexWait: " + std::to_string(errno));
                    return false;
                }
            }
        } while (ret != 0);

        return true;
    }

    void futexWakeAll(std::atomic<int>* addr) {
        int ret;
        do {
            ret = syscall(SYS_futex, addr, FUTEX_WAKE, INT_MAX, NULL, NULL, 0);
        } while (ret == -1 && errno == EINTR);
    }

    struct timespec timeout {0, 100000000}; // 100 milliseconds

    // struct of shared data
    struct shared_data_t {
        std::atomic<int> producer_futex;
        T data_1[3];
    };

    // shared memory
    std::string mapped_file;
    int fd;
    std::atomic<bool> memory_mapped;

    // size of shared structure
    std::size_t blob_size;

    std::atomic<std::uint8_t> local_data_new_index;

    // Shared data structure type from template
    shared_data_t* shared_data;
    std::size_t m_map_size;

    // isProducer flag
    bool isProducer;
        
    // conditional variable
    std::condition_variable cv;

    // newData flag
    std::atomic<bool> m_new_data;

    // local index
    int index{0};
};

#endif // SDATA_H