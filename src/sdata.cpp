#include "sdata.h"
#include <errno.h>

//Template class initialization for the shared memory file and semaphore
template <typename T>
SData<T>::SData(const std::string name, Log& logger, const std::string& mapped_file, const std::string& semaphore_file, bool isProducer) : 
    ronThread(name, logger),
    mapped_file(mapped_file),
    semaphore_file(semaphore_file),
    data_private(T()),
    isProducer(isProducer)
{
}

template <typename T>
bool SData<T>::openMap() {
    logger.pushEvent("Opening memory mapped file");
    // open the mapped memory file descriptor
    int fd = open(mapped_file.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        logger.pushEvent("Error opening/creating memory-mapped file");
        return 1;
    }

    // Truncate the file to the size of shared_data (if it already exists)
    if (ftruncate(fd, sizeof(T)) == -1) {
        logger.pushEvent("Error truncating file");
        close(fd);
        return 1;
    } else {
        logger.pushEvent("File truncated successfully");
    }
    
    // Map the memory
    shared_data = static_cast<T*>(mmap(NULL, sizeof(T), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0u));
    if (shared_data == MAP_FAILED) {
        logger.pushEvent("Error mapping memory");
        close(fd);
        return 1;
    } else {
        logger.pushEvent("Memory mapped successfully");
    }

    // Create/open semaphore
    semaphore = sem_open(semaphore_file.c_str(), O_CREAT, S_IRUSR | S_IWUSR, 1);
    if (semaphore == SEM_FAILED) {
        logger.pushEvent("Semaphore creation failed: " + semaphore_file + ". Error: " + std::strerror(errno));
        return 1;
    } else {
        logger.pushEvent("Semaphore created successfully: " + semaphore_file);
    }
    // ensure the semaphore is unlocked 
    if (sem_post(semaphore) == -1) {
        logger.pushEvent("Error releasing semaphore");
        return 1;
    } else {
        logger.pushEvent("Semaphore released successfully");
    }

    return 0;
}

template <typename T>
void SData<T>::closeMap() {
    // close memory mapped file
    if (munmap(shared_data, sizeof(T)) == -1) {
        logger.pushEvent("Error un-mmapping the file");
    }

    if (sem_close(semaphore) == -1) {
        logger.pushEvent("Error closing semaphore");
    }

    if (sem_unlink(semaphore_file.c_str()) == -1) {
        logger.pushEvent("Error unlinking semaphore");
    }
}

template <typename T>
bool SData<T>::getData(T& data) {
    std::lock_guard<std::mutex> lock(thread_lock);
    data = data_private;
}

template<typename T>
void SData<T>::setData(T data) {
    std::lock_guard<std::mutex> lock(thread_lock);
    newData = true;
    data_private = data;
}

// get ready flag
template <typename T>
bool SData<T>::dataReady() {
    std::lock_guard<std::mutex> lock(thread_lock);
    return newData;
}

// Producer loop
template <typename T>
void SData<T>::producer() {
    // Check if new data is ready to be written 
    if (dataReady()) {
        // wait for the semaphore to be available
        if (sem_wait(semaphore) == -1) {
            log("Error waiting for semaphore");
            return;
        }
        // copy the shared data to the private data
        getData(*shared_data);
        // // release the semaphore
        if (sem_post(semaphore) == -1) {
            log("Error releasing semaphore");
            return;
        }
    }
}

// Consumer loop
template <typename T>
void SData<T>::consumer() {
    // wait for the semaphore to be available
    if (sem_wait(semaphore) == -1) {
        log("Error waiting for semaphore");
        return;
    }
    // copy the shared_data to the private data
    setData(*shared_data);
    // release the semaphore
    if (sem_post(semaphore) == -1) {
        log("Error releasing semaphore");
        return;
    }
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
            // log("Consumer");
            consumer();
        }
        std::this_thread::sleep_for(time_to_sleep);
    }
    closeMap();
}

    