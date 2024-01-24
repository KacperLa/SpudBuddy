#include "joystick.h"

Joystick::Joystick(const std::string name, Log* logger) : ronThread(name, logger) {
}

void Joystick::addDevice(const std::string new_device)
{
    device = new_device;
}

bool Joystick::open() {
    // Open the joystick device
    joystick_ = ::open(device.c_str(), O_RDONLY);
    
    pfd.fd = joystick_;
    pfd.events = POLLIN;
    // Check if the joystick was opened successfully
    return joystick_ != -1;
}

bool Joystick::readEvent(js_event& event) {
    // Read a joystick event
    if (poll(&pfd, 1, 1000) == -1) {
        return 0;
    }    

    if (pfd.revents & POLLIN) {
        // Data is available, read it
        return read(joystick_, &event, sizeof(event)) > 0;
    }
    return 0;
}

void Joystick::close() {
    // Close the joystick device
    log("[joystick] closing fd");
    ::close(joystick_);
}

void Joystick::getState(JoystickState& data) {
    std::lock_guard<std::mutex> lock(thread_lock);
    data = js_state;
}

void Joystick::updateState(const JoystickState& data) {
    std::lock_guard<std::mutex> lock(thread_lock);
    js_state = data;
}

void Joystick::loop() {
    // Open the joystick device
    if (!open()) {
        log("[joystick] Error opening joystick");
        return;
    }

    // Read the joystick events
    js_event event;
    float x = 0;
    float y = 0;
    while (running.load(std::memory_order_relaxed)){
        if (readEvent(event)) {
            // Check the type of the event
            if (event.type == JS_EVENT_AXIS) {
                // Check the axis of the event
                if (event.number == 0) {
                    // X axis
                    x = event.value / 32767.0f;
                } else if (event.number == 1) {
                    // Y axis
                    y = event.value / 32767.0f;
                }

                // Add the new values to the queue
                JoystickState data{x, y, 0};
                updateState(data);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    // Close the joystick device
    close();
}
