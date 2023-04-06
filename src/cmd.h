#ifndef CMD_H
#define CMD_H

#include <string>
#include <iostream>
#include <mutex>
#include <atomic>
#include <chrono>
#include "fcntl.h"
#include "unistd.h"
#include <poll.h>
#include <vector>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <libraries/json/json.hpp>
using json = nlohmann::json;

#include<ronThread.h>
#include<joystick.h>

class Cmd : public ronThread
{
public:
    Cmd(const std::string name, Log& logger);
    void getState(JoystickState& data);

protected:
    virtual void loop() override;
    bool open();
    void close();

    bool readEvent(json& event);

    void updateState(const JoystickState& data);

    // Socket specific
    zmq::context_t context;
    zmq::socket_t socket_sub{context, zmq::socket_type::sub};
    
    std::string socket_sub_address {"tcp://0.0.0.0:6681"};

    const std::chrono::milliseconds timeout{10};

    zmq::poller_t<zmq::socket_t> sub_poller;


    JoystickState js_state;
    std::mutex joystick_state_lock_;
};

#endif // CMD_H