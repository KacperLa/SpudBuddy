import posix_ipc
import struct
import mmap

class mmap_writer:
    def set_up(self):        
        # Open the mapped file for writting
        with open(self.map_path , 'w+b') as f:
            # Pre-allocate file size
            f.write(b'\0' * self.memory_size)
            f.flush()
            self.mmapped_file = mmap.mmap(f.fileno(), self.memory_size, mmap.MAP_SHARED)

        print("mmap %s allocated." %self.map_path)

    def __init__(self, semaphore_name, map_path, format_mapping):
        self.map_path = map_path
        self.semaphore = posix_ipc.Semaphore(semaphore_name, posix_ipc.O_CREAT, initial_value=0)
        self.format_mapping = format_mapping
        self.memory_size = struct.calcsize(''.join(format_mapping.values())) 
        self.mmapped_file = None

        self.attempts = 5
        self.timeout = 1
        self.set_up()
        
    def write(self, data, timeout=150):
        success, message = False, None
        format_string = ''.join(self.format_mapping.values())

        for attempt in range(self.attempts):
            try:
                self.semaphore.acquire(self.timeout)
                self.mmapped_file.seek(0)
                self.mmapped_file.write(struct.pack(format_string, *data.values()))
                self.semaphore.release()
                message = "Success"
                break
            except posix_ipc.BusyError:
                print("Semaphore is busy, trying again.")
                continue
            print("No response")
        else:
            success = False
            message = "semaphore is busy, please try again." 
        return success, message

    def tear_down(self):
        self.mmapped_file.close()
        self.semaphore.close()
        print("mmap %s closed." %self.map_path)

class mmap_reader:
    def set_up(self):
        # Open the mapped file for reading
        with open(self.map_path , 'r+b') as f:
            self.mmapped_file = mmap.mmap(f.fileno(), self.memory_size, mmap.MAP_SHARED)

        print("mmap %s allocated." %self.map_path)

    def __init__(self, semaphore_name, map_path, format_mapping):
        self.map_path = map_path
        self.semaphore = posix_ipc.Semaphore(semaphore_name, posix_ipc.O_CREAT, initial_value=0)
        self.format_mapping = format_mapping
        self.memory_size = struct.calcsize(''.join(format_mapping.values())) 
        self.mmapped_file = None

        self.attempts = 5
        self.timeout = 1
        self.set_up()

    def read(self, data):
        format_string = ''.join(self.format_mapping.values())

        for attempt in range(self.attempts):
            try:
                self.semaphore.acquire(self.timeout)
                self.mmapped_file.seek(0)
                data_raw = struct.unpack(format_string, self.mmapped_file.read(self.memory_size))
                self.semaphore.release()
                # parse datq_raw into data
                for key in self.format_mapping.keys():
                    data[key] = data_raw[list(self.format_mapping.keys()).index(key)]
                return data
                message = "Success"
                break
            except posix_ipc.BusyError:
                print("Semaphore is busy, trying again.")
                continue
            print("No response")
        return None