
#include <thread>
#include <sys/mman.h>

#include <pthread.h>
#include <sched.h>

#include<joystick.h>
#include<FSM.h>
#include<sdata.h>
#include "DriveSystem.h"
#include "shared_structs.h"
#include<loggerThreaded.h>
#include<logger.h>

// #ifdef BUILD_FOR_SIM
    #include "ZEDReader.h"
// #else
//     #include "DrakeReader.h"
// #endif

bool time_to_quit = false;

using fsm_handle = Robot;

// Enum of positional tracking states
enum class PositionState {
    NONE,
    DEAD_RECKONING,
    SLAM,
    GPS
};


// location of shared memory
const char* shared_actual_file   = "/tmp/robot_actual";
const char* shared_desired_file  = "/tmp/robot_desired";
const char* shared_settings_file = "/tmp/robot_settings";

LogThreaded* logger_threaded;

const std::int64_t timeout{10};
const std::int64_t publish_loop (1000000.0 / 1000.0); // 1000 Hz
const std::int64_t main_loop    (1000000.0 / 5000.0); // 4000 Hz
