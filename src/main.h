
#include <thread>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <libraries/json/json.hpp>
using json = nlohmann::json;

#include<joystick.h>
#include<IMUReader.h>
#include<FSM.h>
#include<cmd.h>
#include<Logging.h>
#include "DriveSystem.h"

bool time_to_quit = false;

using fsm_handle = Robot;

// Socket specific
zmq::context_t context;
zmq::socket_t socket_pub{context, zmq::socket_type::pub};
zmq::socket_t socket_rep{context, zmq::socket_type::rep};

Log logger{context};

json default_response = {{"success", false}, {"message", "Endpoint now valid"}};

const std::chrono::milliseconds timeout{10};

zmq::poller_t<zmq::socket_t> rep_poller;

std::string socket_pub_address {"tcp://0.0.0.0:6581"};
std::string socket_rep_address {"tcp://0.0.0.0:6556"};

