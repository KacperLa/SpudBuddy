
#include <thread>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <libraries/json/json.hpp>
using json = nlohmann::json;
#include <sys/mman.h>

#include<joystick.h>
#include<IMUReader.h>
#include<FSM.h>
#include<cmd.h>
#include<Logging.h>
#include "DriveSystem.h"
#include "ZEDReader.h"

bool time_to_quit = false;

using fsm_handle = Robot;

// DriveSystem specific
struct driveSystemState {
    DriveState axis_0;
    DriveState axis_1;
};

struct sytemState_t {
    robot_state_t actual;
    robot_state_t desired;
    driveSystemState driveSystem;
    JoystickState joystick;
};

// location of shared memory
const char* shared_state_file = "/tmp/robot_state";

// Socket specific
zmq::context_t context;
zmq::socket_t socket_pub{context, zmq::socket_type::pub};
zmq::socket_t socket_rep{context, zmq::socket_type::rep};

Log logger;

json default_response = {{"success", false}, {"message", "Endpoint now valid"}};

const std::chrono::milliseconds timeout{10};

zmq::poller_t<zmq::socket_t> rep_poller;

std::string socket_pub_address {"ipc:///tmp/ron3000"};
std::string socket_rep_address {"ipc:///tmp/ron3001"};

