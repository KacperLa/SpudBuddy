
#include <thread>
#include <sys/mman.h>

#include<joystick.h>
#include<IMUReader.h>
#include<FSM.h>
#include<sdata.h>
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
    driveSystemState driveSystem;
};

struct systemDesired_t {
    int state {0};
    JoystickState joystick;
};

// location of shared memory
const char* shared_actual_file  = "/tmp/robot_actual";
const char* shared_desired_file = "/tmp/robot_desired";

// location of semaphore
const char* semaphore_actual_file  = "/robot_actual_sem";
const char* semaphore_desired_file = "/robot_desired_sem";


Log logger;

const std::chrono::milliseconds timeout{10};
