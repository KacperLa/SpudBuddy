#include "joystick.h"

Joystick::Joystick(const std::string &device) : device(device) {}

Joystick::~Joystick() {}

bool Joystick::open() {
    // Open the joystick device
    joystick = ::open(device.c_str(), O_RDONLY);

    // Check if the joystick was opened successfully
    return joystick != -1;
}

bool Joystick::readEvent(js_event &event) {
    // Read a joystick event
    return read(joystick, &event, sizeof(event)) > 0;
}

void Joystick::close() {
    // Close the joystick device
    ::close(joystick);
}

void Joystick::getState(JoystickState & data){
    std::lock_guard<std::mutex> lock(joystick_state_lock);
    data = js_state;
}

void Joystick::updateState(JoystickState data){
    std::lock_guard<std::mutex> lock(joystick_state_lock);
    js_state = data;
}

void Joystick::ReadJoystickLoop() {
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