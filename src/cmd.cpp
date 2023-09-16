#include "cmd.h"

Cmd::Cmd(const std::string name, Log& logger) : 
    ronThread(name, logger)
{
}

bool Cmd::open() {
    // Open and bind zmq sockets
    socket_sub.connect(socket_sub_address);
    socket_sub.set(zmq::sockopt::linger, 0);
    socket_sub.set(zmq::sockopt::subscribe, "");

    sub_poller.add(socket_sub, zmq::event_flags::pollin);
    return 1;
}

bool Cmd::readEvent(json& event) {
    bool ret = 0;
    std::vector<zmq::poller_event<zmq::socket_t>> sub_events(1);
    zmq::message_t message;
    auto n = sub_poller.wait_all(sub_events, timeout);
    if (n) {
        if (zmq::event_flags::pollin == sub_events[0].events) {
            const auto ok = socket_sub.recv(message);
            std::string str = std::string(static_cast<char*>(message.data()), message.size());
            
            log("cmd got a request: " + str);
            
            event = json::parse(str);

            ret = 1;
        }
    } 
    return ret;
}

void Cmd::close() {
    socket_sub.close();
}

void Cmd::getState(JoystickState& data) {
    std::lock_guard<std::mutex> lock(thread_lock);
    data = js_state;
}

void Cmd::updateState(const JoystickState& data) {
    std::lock_guard<std::mutex> lock(thread_lock);
    js_state = data;
}

void Cmd::loop() {
    // Open the zmq socket
    if (!open()) {
        log("Error opening socket");
        return;
    }

    // Read the joystick events
    json event;
    while (running.load(std::memory_order_relaxed)){
        if (readEvent(event)) {
            JoystickState data{std::stof(static_cast<std::string>(event.at("x"))), std::stof(static_cast<std::string>(event.at("y"))), std::chrono::high_resolution_clock::now()};
            updateState(data);
        }
        // no need to sleep becasue the poll has a timeout.
    }

    // Close the zmq socker
    close();
}

    