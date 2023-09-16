import mmap
import struct

# # Define the format string to match the structure in C++
# # In this case, 'i' corresponds to a 4-byte integer (int)
format_string = 'ffddddddfffiffddddddfffiffibffibffii'
memory_size = struct.calcsize(format_string)  # Use the format string for your struct

# Open the file for reading
with open('/tmp/robot_state', 'r+b') as f:
    # Memory-map the file, size 0 means whole file
    mmapped_file = mmap.mmap(f.fileno(), memory_size, mmap.MAP_SHARED)

    # Read the binary data from the mmap object
    binary_data = mmapped_file.read(struct.calcsize(format_string))

    # Unpack the binary data into a Python object
    # Here, we're expecting a single integer (4 bytes)
    for value in struct.unpack(format_string, binary_data):
        print("Value read from shared memory:", value)