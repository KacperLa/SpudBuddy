#include "sdata.h"
#include <errno.h>

namespace bip = boost::interprocess;

//Template class initialization for the shared memory file and semaphore
template <typename T>
SData<T>::SData(const std::string name, Log* logger, const std::string& mapped_file, bool isProducer) : 
    ronThread(name, logger),
    mapped_file(mapped_file),
    data_private(T()),
    isProducer(isProducer)
{
    this->startThread();
}

//Template class initialization for the shared memory file and semaphore
template <typename T>
SData<T>::SData(const std::string& mapped_file, bool isProducer) : 
    ronThread("none", static_cast<Log*>(simplelog)),
    mapped_file(mapped_file),
    data_private(T()),
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

    // Truncate the file to the size of shared_data (if it already exists)
    if (ftruncate(fd, sizeof(shared_data_t)) == -1) {
        log("Error truncating file");
        close(fd);
        return 1;
    } else {
        log("File truncated successfully");
    }
    
    // Map the memory
    shared_data = static_cast<shared_data_t*>(mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0u));
    if (shared_data == MAP_FAILED) {
        log("Error mapping memory");
        close(fd);
        return 1;
    } else {
        log("Memory mapped successfully");
    }

    return 0;
}

template <typename T>
void SData<T>::closeMap() {
    // close memory mapped file
    if (munmap(shared_data, sizeof(shared_data_t)) == -1) {
        log("Error un-mmapping the file");
    }

    if (sem_close(semaphore) == -1) {
        log("Error closing semaphore");
    }

    if (sem_unlink(semaphore_file.c_str()) == -1) {
        log("Error unlinking semaphore");
    }
}

template <typename T>
void SData<T>::getData(T& data) {
    std::unique_lock<std::mutex> lock(thread_lock);
    data = data_private;
}

template <typename T>
bool SData<T>::isDataNew(T& data) {
    // std::unique_lock<std::mutex> lock(thread_lock);
    data = data_private;
    if (newData) {
        newData = false;
        return true;
    }
    return false;
}

template<typename T>
void SData<T>::setData(const T& data) {
    std::cout << "Setting data" << std::endl;   
    std::unique_lock<std::mutex> lock(thread_lock);
    data_private = data;
    newData = true;
    cv.notify_all();
}

// Producer loop
template <typename T>
void SData<T>::producer() {
    static T data;
    static bool dataReady;
    {
        std::unique_lock<std::mutex> lock(thread_lock);
        dataReady = (cv.wait_for(lock, 
                    std::chrono::milliseconds(time_to_sleep),
                    [this] { return isDataNew(data); }
                    ));
    }

    if (dataReady)    
    {          
        shared_data->data[(shared_data->index.load(std::memory_order_acquire) + 1) % 2] = data;
        // lock the boost mutex from the shared data
        // befoer incrementing the index
        boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(shared_data->mutex);
        // increment the index
        shared_data->index.fetch_add(1, std::memory_order_release);
        // log("Index updated"+std::to_string(shared_data->index.load(std::memory_order_acquire)));
        // notify the consumer
        shared_data->cv.notify_all();
    }
    else 
    {
        log("No new data");
    }
}

template <typename T>
void SData<T>::consumer() {
        boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(shared_data->mutex);  
        boost::posix_time::ptime timeout = boost::posix_time::microsec_clock::universal_time() + boost::posix_time::milliseconds(time_to_sleep);
        if (shared_data->cv.timed_wait(lock,
                             timeout,
                             [this]{ return (shared_data->index.load(std::memory_order_acquire) != index);}))
        
        {
            // copy the shared_data to the private data
            setData(shared_data->data[shared_data->index.load(std::memory_order_acquire) % 2]);
            // save the index
            index = shared_data->index.load(std::memory_order_acquire);
        }
        // else
        // {
        //     log("Consumer no new data, time out");
        // }
}

template <typename T>
void SData<T>::loop() {
    if (openMap()) {
        log("Error opening mmap");
        return;
    }   else {
        log("Memory mapped successfully");
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

    