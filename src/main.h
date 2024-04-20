
#include <thread>
#include <sys/mman.h>

#include <pthread.h>
#include <sched.h>

#include <joystick.h>
#include <libraries/sdata/include/sdata.hpp>
using namespace sdata;

#include "driveSystem.h"
#include "shared_structs.h"
#include<loggerThreaded.h>
#include<logger.h>

// #ifdef BUILD_FOR_SIM
    #include "ZEDReader.h"
// #else
//     #include "DrakeReader.h"
// #endif

bool time_to_quit = false;


// Enum of positional tracking states
enum class PositionState {
    NONE,
    DEAD_RECKONING,
    SLAM,
    GPS
};



LogThreaded* logger_threaded;

const std::uint64_t timeout{10};
const std::uint64_t publish_loop (1000000000 / 100);
const std::chrono::nanoseconds main_loop(1000000000 / 1000);
const std::chrono::nanoseconds dead_reckoning_loop(1000000000 / 10);
