#include "IMUReader.h"

IMUReader::IMUReader(const std::string &device) : device(device) {}

IMUReader::~IMUReader() {}

bool IMUReader::open() {
    return bno055.imu_init(config.BNO055_I2C_BUS.c_str(), BNO055_I2C_ADDRESS, imu);
}

bool IMUReader::readEvent(js_event &event) {
    // Read a joystick event
    BNO055::euler_angles angles;
    int ret = bno055.get_euler_angles(&angles);
    imu_state.roll = angles.roll;
    imu_state.pitch = angles.pitch;
    imu_state.yaw = angles.yaw;

    return read(joystick, &event, sizeof(event)) > 0;
}

void IMUReader::close() {
    // Close the joystick device
    ::close(joystick);
}

void IMUReader::getState(JoystickState & data){
    std::lock_guard<std::mutex> lock(joystick_state_lock);
    data = js_state;
}

void IMUReader::updateState(JoystickState data){
    std::lock_guard<std::mutex> lock(joystick_state_lock);
    js_state = data;
}

void IMUReader::ReadJoystickLoop() {
    // Open the joystick device
    if (!open()) {
        std::cout << "error opening joystick." << std::endl;
        return;
    }

    // Read the joystick events
    js_event event;
    int x = 0;
    int y = 0;
    while (readEvent(event)) {
        // Check the type of the event
        if (event.type == JS_EVENT_AXIS) {
            std::cout << "got event"<< std::endl;
            // Check the axis of the event
            if (event.number == 0) {
                // X axis
                x = event.value;
            } else if (event.number == 1) {
                // Y axis
                y = event.value;
            }

            // Add the new values to the queue
            JoystickState data{std::chrono::high_resolution_clock::now(), x, y};
            updateState(data);
        }
        usleep(10);
    }

    // Close the joystick device
    close();
}