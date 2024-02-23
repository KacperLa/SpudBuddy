#include "sdata.h"
#include <errno.h>

namespace bip = boost::interprocess;

template <typename T>
bool SData<T>::futexWait(std::atomic<int>* addr, int val) {
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

template <typename T>
void SData<T>::futexWakeAll(std::atomic<int>* addr) {
    int ret;
    do {
        ret = syscall(SYS_futex, addr, FUTEX_WAKE, INT_MAX, NULL, NULL, 0);
    } while (ret == -1 && errno == EINTR);
}

//Template class initialization for the shared memory file and semaphore
template <typename T>
SData<T>::SData(Log* logger, const std::string& mapped_file, bool isProducer) : 
    ronThread(mapped_file, logger, true),
    mapped_file(mapped_file),
    m_new_data(false),
    memory_mapped(false),
    isProducer(isProducer)
{
    if (openMap()) {
        log("Error opening mmap");
        return;
    }   else {
        log("Memory mapped successfully");
        memory_mapped = true;
    }
}

//Template class initialization for the shared memory file and semaphore
template <typename T>
SData<T>::SData(const std::string& mapped_file, bool isProducer) : 
    ronThread(mapped_file, static_cast<Log*>(simplelog), true),
    mapped_file(mapped_file),
    memory_mapped(false),
    isProducer(isProducer)
{
    if (openMap()) {
        log("Error opening mmap");
        return;
    }
    else
    {
        log("Memory mapped successfully");
        memory_mapped = true;
    }
}

// Destructor
template <typename T>
SData<T>::~SData()
{
}

template <typename T>
bool SData<T>::openMap()
{
    log("Opening memory mapped file");    
    // open the mapped memory file descriptor
    int fd;
    // prepare the file descriptor with /tmp/
    mapped_file = "/tmp/" + mapped_file;


    // if (isProducer)
    // {
        fd = open(mapped_file.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    // } else {
    //     fd = open(mapped_file.c_str(), O_RDWR);
    // }

    if (fd == -1) {
        log("Error opening/creating memory-mapped file");
        return 1;
    }

    // if (isProducer)
    // {
        // Check if file needs to be truncated
        struct stat fileStat;
        if (stat(mapped_file.c_str(), &fileStat) == 0)
        {
            off_t currentSize = fileStat.st_size;
            if (currentSize != sizeof(shared_data_t))
            {
                if (ftruncate(fd, sizeof(shared_data_t)) == -1)
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
    // }

    // Map the memory
    shared_data = static_cast<shared_data_t*>(mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0u));
    if (shared_data == MAP_FAILED) {
        log("Error mapping memory");
        close(fd);
        return true;
    } else {
        log("Memory mapped successfully");
    }

    if (isProducer)
    {
        shared_data->producer_futex.store(0, std::memory_order_release);
        shared_data->consumer_futex.store(0, std::memory_order_release);
        shared_data->triple_buffer[0] = T();
        shared_data->triple_buffer[1] = T();
        shared_data->triple_buffer[2] = T();
    }
    return 0;
}

template <typename T>
void SData<T>::closeMap() {
    // close memory mapped file
    if (munmap(shared_data, sizeof(shared_data_t)) == -1) {
        log("Error un-mmapping the file");
    }
}

template <typename T>
bool SData<T>::isMemoryMapped() {
    return memory_mapped.load(std::memory_order_acquire);
}

template <typename T>
bool SData<T>::getData(T& data) {
    int current_buffer_index = ((shared_data->producer_futex.load(std::memory_order_acquire) - 1) % 3);
    std::memcpy(&data, 
                &shared_data->triple_buffer[current_buffer_index],
                sizeof(T));
    if (current_buffer_index == (shared_data->producer_futex.load(std::memory_order_acquire) % 3))
    {
        log("Buffer index potential corruption");
        return false;
    }
    else
    {
        return true;
    }
}

template <typename T>
bool SData<T>::waitOnStateChange(T& data)
{
    int current_index = shared_data->producer_futex.load(std::memory_order_acquire);
    if (futexWait(&shared_data->producer_futex, current_index))
    {
        std::memcpy(&data,
                    &shared_data->triple_buffer[current_index % 3],
                    sizeof(T));
        if ((current_index % 3) == (shared_data->producer_futex.load(std::memory_order_acquire) % 3))
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

template<typename T>
void SData<T>::setData(const T& data)
{
    std::memcpy(&shared_data->triple_buffer[shared_data->producer_futex.load(std::memory_order_acquire)  % 3], 
                &data, 
                sizeof(T));
    shared_data->producer_futex.fetch_add(1, std::memory_order_release);
    futexWakeAll(&shared_data->producer_futex);
}

// Producer loop
template <typename T>
void SData<T>::producer() {
    // if (futexWait(&shared_data->producer_futex, shared_data->producer_futex.load(std::memory_order_acquire), time_to_sleep_ms))
    // {
    //     // shared_data->mutex.lock();
    //     // atomicly swap the free producer buffer with the consumer buffer
    //     shared_data->buffer_index[2].store(shared_data->buffer_index[(shared_data->producer_futex.load(std::memory_order_acquire) + 1) % 2].exchange(shared_data->buffer_index[2], std::memory_order_acquire));
    //     // shared_data->mutex.unlock();

    //     shared_data->consumer_futex.fetch_add(1, std::memory_order_release);
    //     futexWakeAll(&shared_data->consumer_futex);
    // }
    // else 
    // {
    //     log("No new data");
    // }
}

template <typename T>
void SData<T>::consumer() {
    // if (futexWait(&shared_data->consumer_futex, (shared_data->consumer_futex.load(std::memory_order_acquire)), time_to_sleep_ms))
    // {
    //     int current_buffer_index = shared_data->buffer_index[2].load(std::memory_order_acquire);
    //     // shared_data->mutex.lock_sharable();
    //     std::memcpy(&local_data[local_data_new_index.load(std::memory_order_acquire) % 2], 
    //                 &shared_data->triple_buffer[current_buffer_index],
    //                 sizeof(T));
    //     if (current_buffer_index == shared_data->buffer_index[2].load(std::memory_order_acquire))
    //     {
    //         local_data_new_index.fetch_add(1, std::memory_order_release);  
    //     }
    //     else 
    //     {
    //         log("Buffer index mismatch");
    //     }
    // }
}

template <typename T>
void SData<T>::loop() {
    while (running.load(std::memory_order_relaxed)){
        if (isProducer) {
            producer();
        } else {
            consumer();
        }
    }
}

    