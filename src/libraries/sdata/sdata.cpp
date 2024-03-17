#include "sdata.h"
#include <errno.h>
#include <climits>

// logging function
void MappedBuffer::log(std::string message)
{
    std::cout << message << std::endl;
}

bool MappedBuffer::futexWait(std::atomic<int>* addr, int val) {
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

void MappedBuffer::futexWakeAll(std::atomic<int>* addr) {
    int ret;
    do {
        ret = syscall(SYS_futex, addr, FUTEX_WAKE, INT_MAX, NULL, NULL, 0);
    } while (ret == -1 && errno == EINTR);
}

// class initialization for the shared memory file and semaphore
MappedBuffer::MappedBuffer(const std::string mapped_file, bool isProducer, std::size_t blob_size) : 
    mapped_file(mapped_file),
    memory_mapped(false),
    isProducer(isProducer),
    blob_size(blob_size),
    map_size(sizeof(shared_data_t) + blob_size*3)
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
MappedBuffer::~MappedBuffer()
{
}

bool MappedBuffer::openMap()
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
            if (currentSize != map_size )
            {
                if (ftruncate(fd, map_size) == -1)
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
    shared_data = static_cast<shared_data_t*>(mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0u));
    if (shared_data == MAP_FAILED) {
        log("Error mapping memory");
        close(fd);
        return true;
    } else {
        log("Memory mapped successfully");
    }

  
    shared_data->producer_futex.store(1, std::memory_order_release);

    m_data[0] = static_cast<void*>(shared_data + sizeof(shared_data_t) + 1);
    m_data[1] = static_cast<void*>(shared_data + sizeof(shared_data_t) + 1 + blob_size*1);
    m_data[2] = static_cast<void*>(shared_data + sizeof(shared_data_t) + 1 + blob_size*2);
    return 0;
}

void MappedBuffer::closeMap() {
    // close memory mapped file
    if (munmap(shared_data, map_size) == -1) {
        log("Error un-mmapping the file");
    }
}

bool MappedBuffer::isMemoryMapped() {
    return memory_mapped.load(std::memory_order_acquire);
}

bool MappedBuffer::getData(void *data) {
    int current_buffer_index = ((shared_data->producer_futex.load(std::memory_order_acquire) - 1) % 3);
    std::memcpy(data,
                m_data[current_buffer_index],
                sizeof(blob_size));
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

bool MappedBuffer::waitOnStateChange(void *data)
{
    int current_index = shared_data->producer_futex.load(std::memory_order_acquire);
    std::atomic_wait(&shared_data->producer_futex, current_index);
    return getData(data);
    // if (futexWait(&shared_data->producer_futex, current_index))
    // {
    //     std::memcpy(&data,
    //                 m_data[current_index % 3],
    //                 sizeof(blob_size));
    //     if ((current_index % 3) == (shared_data->producer_futex.load(std::memory_order_acquire) % 3))
    //     {
    //         log("Buffer index potential corruption");
    //         return false;
    //     }
    //     else
    //     {
    //         return true;
    //     }
    // }
    // else
    // {
    //     return false;
    // }
}

void MappedBuffer::setData(const void* data)
{
    std::memcpy(m_data[shared_data->producer_futex.load(std::memory_order_acquire)  % 3], 
                data, 
                sizeof(blob_size));
    shared_data->producer_futex.fetch_add(1, std::memory_order_release);
    // futexWakeAll(&shared_data->producer_futex);
    // trigger an atomic wak eall
    shared_data->producer_futex.notify_all();   
}

void MappedBuffer::setData()
{
    shared_data->producer_futex.fetch_add(1, std::memory_order_release);
    shared_data->producer_futex.notify_all();   
}

void* MappedBuffer::getBuffer()
{
    return m_data[shared_data->producer_futex.load(std::memory_order_acquire) % 3];
} 
    
