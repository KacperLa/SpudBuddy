
#include <thread>
#include <sys/mman.h>

#include<joystick.h>
#include<IMUReader.h>
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

// location of semaphore
const char* semaphore_actual_file   = "/robot_actual_sem";
const char* semaphore_desired_file  = "/robot_desired_sem";
const char* semaphore_settings_file = "/robot_settings_sem";

LogThreaded* logger_threaded;

const std::int64_t timeout{10};
