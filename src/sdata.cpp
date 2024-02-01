#include "sdata.h"
#include <errno.h>

namespace bip = boost::interprocess;

template <typename T>
bool SData<T>::futexWait(int* addr, int val, int timeoutMilliseconds) {
    struct timespec timeout;
    timeout.tv_sec = timeoutMilliseconds / 1000;
    timeout.tv_nsec = (timeoutMilliseconds % 1000) * 1000000;

    int ret;
    do {
        ret = syscall(SYS_futex, addr, FUTEX_WAIT, val, &timeout, NULL, 0);
        if (ret == -1) {
            if (errno == EINTR) {
                // Interrupted by a signal, continue waiting
                continue;
            } else if (errno == ETIMEDOUT) {
                // Timeout occurred
                return false;
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
void SData<T>::futexWakeAll(int* addr) {
    int ret;
    do {
        ret = syscall(SYS_futex, addr, FUTEX_WAKE, INT_MAX, NULL, NULL, 0);
    } while (ret == -1 && errno == EINTR);
}

//Template class initialization for the shared memory file and semaphore
template <typename T>
SData<T>::SData(const std::string name, Log* logger, const std::string& mapped_file, bool isProducer) : 
    ronThread(name, logger),
    mapped_file(mapped_file),
    local_data(T()),
    m_new_data(false),
    memory_mapped(false),
    isProducer(isProducer)
{
    this->startThread();
}

//Template class initialization for the shared memory file and semaphore
template <typename T>
SData<T>::SData(const std::string& mapped_file, bool isProducer) : 
    ronThread("none", static_cast<Log*>(simplelog)),
    mapped_file(mapped_file),
    local_data(T()),
    memory_mapped(false),
    isProducer(isProducer)
{
    this->startThread();
}

// Destructor
template <typename T>
SData<T>::~SData() {
    this->stopThread();
}

template <typename T>
bool SData<T>::openMap() {
    log("Opening memory mapped file");    
    // open the mapped memory file descriptor
    int fd = open(mapped_file.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        log("Error opening/creating memory-mapped file");
        return 1;
    }

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

    // Map the memory
    shared_data = static_cast<shared_data_t*>(mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0u));
    if (shared_data == MAP_FAILED) {
        log("Error mapping memory");
        close(fd);
        return true;
    } else {
        log("Memory mapped successfully");
    }

    // print index and futex of shared data
    log("Index: "+std::to_string(shared_data->index.load(std::memory_order_acquire)));
    log("Futex: "+std::to_string(shared_data->futex));

    if (isProducer)
    {
        shared_data->index.store(0, std::memory_order_release);
        shared_data->futex = 0;
        shared_data->triple_buffer[0] = T();
        shared_data->triple_buffer[1] = T();
        shared_data->triple_buffer[2] = T();

        shared_data->producer_index = 0U;
        shared_data->consumer_index = 1U;
        shared_data->free_index     = 2U;
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
    bool expected = true;
    if (m_new_data.compare_exchange_weak(expected, false, std::memory_order_acquire))
    {
        std::lock_guard<std::mutex> lock(thread_lock);
        std::memcpy(&data, &local_data, sizeof(T));
        return true;
    }
    return false;
}

template <typename T>
bool SData<T>::isDataNew()
{
    bool expected = true;
    if (m_new_data.compare_exchange_weak(expected, false, std::memory_order_acquire))
    {
        std::swap(shared_data->producer_index,
                  shared_data->free_index);
        return true;
    }
    return false;
}

template<typename T>
void SData<T>::setData(const T& data) {
    {
        std::lock_guard<std::mutex> lock(thread_lock);
        std::memcpy(&shared_data->triple_buffer[shared_data->producer_index], 
                    &data, 
                    sizeof(T));
        m_new_data.store(true, std::memory_order_release);
    }
    cv.notify_all();
}

// Producer loop
template <typename T>
void SData<T>::producer() {
    static bool dataReady;
    {
        std::unique_lock<std::mutex> lock(thread_lock);
        dataReady = (cv.wait_for(lock, 
                    std::chrono::milliseconds(time_to_sleep_ms),
                    [this] { return isDataNew(); }
                    ));
    }
    if (dataReady)    
    {
        shared_data->mutex.lock();
        std::swap(shared_data->free_index,
                  shared_data->consumer_index);
        shared_data->mutex.unlock();

        shared_data->index.fetch_add(1, std::memory_order_release);
        shared_data->futex = shared_data->index.load(std::memory_order_acquire);
        futexWakeAll(&shared_data->futex);
    }
    else 
    {
        log("No new data");
    }
}

template <typename T>
void SData<T>::consumer() {
    T data;
    if (futexWait(&shared_data->futex, (shared_data->index.load(std::memory_order_acquire)), time_to_sleep_ms))
    {
        shared_data->mutex.lock_sharable();
        std::memcpy(&data, 
                    &shared_data->triple_buffer[shared_data->consumer_index],
                    sizeof(T));
        shared_data->mutex.unlock_sharable();

        {
            std::lock_guard<std::mutex> lock(thread_lock);
            std::memcpy(&local_data, &data, sizeof(T));
            m_new_data.store(true, std::memory_order_release);
        }
    }
}

template <typename T>
void SData<T>::loop() {
    if (openMap()) {
        log("Error opening mmap");
        return;
    }   else {
        log("Memory mapped successfully");
        memory_mapped = true;
    }

    while (running.load(std::memory_order_relaxed)){
        if (isProducer) {
            producer();
        } else {
            consumer();
        }
    }
    closeMap();
}

    