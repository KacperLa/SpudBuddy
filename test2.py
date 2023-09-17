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

    # Unpack the binary data into a Python object
    # Here, we're expecting a single integer (4 bytes)
    while True:
        binary_data = mmapped_file[:]
        x = struct.unpack('f', mmapped_file[0:4])[0]
        y = struct.unpack('f', mmapped_file[4:8])[0]
        roll  =  struct.unpack('d', mmapped_file[8:16])[0]
        pitch = struct.unpack('d', mmapped_file[16:24])[0]
        yaw   =   struct.unpack('d', mmapped_file[24:32])[0]
        gyro_x = struct.unpack('d', mmapped_file[32:40])[0]
        gyro_y = struct.unpack('d', mmapped_file[40:48])[0]
        gyro_z = struct.unpack('d', mmapped_file[48:56])[0]
        state  = struct.unpack('i', mmapped_file[56:60])[0]

        # .read(struct.calcsize(format_string))
        # if len(binary_data) > 0:
        #     x = struct.unpack('f', binary_data[0:4])[0]
        #     y = struct.unpack('f', binary_data[4:8])[0]
        print("x: ", x,"y: ", y, "roll: ", roll, "pitch: ", pitch, "yaw: ", yaw, "gyro_x: ", gyro_x, "gyro_y: ", gyro_y, "gyro_z: ", gyro_z, "state: ", state)
            # for value in struct.unpack(format_string, binary_data):
            #     print("Value read from shared memory:", value)