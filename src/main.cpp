#include<main.h>
#include "iostream"
#include <sys/resource.h>
#include "common.h"
#include <sched.h>

#include <controllerThread.h>

void signal_handler(const int signal)
{
    /**
     *  Signal        x86/ARM     Alpha/   MIPS   PARISC   Notes
     *              most others   SPARC
     *  ─────────────────────────────────────────────────────────────────
     *  SIGHUP           1           1       1       1
     *  SIGINT           2           2       2       2
     *  SIGQUIT          3           3       3       3
     *  SIGILL           4           4       4       4
     *  SIGTRAP          5           5       5       5
     *  SIGABRT          6           6       6       6
     *  SIGIOT           6           6       6       6
     *  SIGBUS           7          10      10      10
     *  SIGEMT           -           7       7      -
     *  SIGFPE           8           8       8       8
     *  SIGKILL          9           9       9       9
     *  SIGUSR1         10          30      16      16
     *  SIGSEGV         11          11      11      11
     *  SIGUSR2         12          31      17      17
     *  SIGPIPE         13          13      13      13
     *  SIGALRM         14          14      14      14
     *  SIGTERM         15          15      15      15
     *  SIGSTKFLT       16          -       -        7
     *  SIGCHLD         17          20      18      18
     *  SIGCLD           -          -       18      -
     *  SIGCONT         18          19      25      26
     *  SIGSTOP         19          17      23      24
     *  SIGTSTP         20          18      24      25
     *  SIGTTIN         21          21      26      27
     *  SIGTTOU         22          22      27      28
     *  SIGURG          23          16      21      29
     *  SIGXCPU         24          24      30      12
     *  SIGXFSZ         25          25      31      30
     *  SIGVTALRM       26          26      28      20
     *  SIGPROF         27          27      29      21
     *  SIGWINCH        28          28      20      23
     *  SIGIO           29          23      22      22
     *  SIGPOLL                                            Same as SIGIO
     *  SIGPWR          30         29/-     19      19
     *  SIGINFO          -         29/-     -       -
     *  SIGLOST          -         -/29     -       -
     *  SIGSYS          31          12      12      31
     *  SIGUNUSED       31          -       -       31
     *
     * Posix signal SIGKILL result is immediate program termination.
     *
     */

    std::string msg = "[MAIN] System signal: (" + std::to_string(signal) + ")";
    logger_threaded->pushEvent(msg);
    switch (signal) {

    case SIGINT:
        time_to_quit = true;
        break;

    case SIGABRT:
        // Deactivate the propulsion system immediately.
        time_to_quit = true;
        break;

    case SIGUSR1:
        //std::cout << "Received canopy detection signal." << std::endl;
        time_to_quit = true;
        break;

    case SIGUSR2:
        // TODO
        time_to_quit = true;
        break;

    case SIGTERM:
        // TODO
        time_to_quit = true;
        break;
    }
}

int main(int argc, char *argv[])
{
        std::cout << "Starting up.." << std::endl;
        std::cout << __cplusplus << std::endl;
        // setpriority(PRIO_PROCESS, getpid(), 1);

        struct sched_param param;
        param.sched_priority = 99;
        int canSetRealTimeThreadPriority = (sched_setscheduler(pthread_self(), SCHED_FIFO, &param) == 0);

        if (!canSetRealTimeThreadPriority)
        {
            std::cout << "Failed to set real-time thread priority." << std::endl;
        }

        signal(SIGINT, signal_handler);
        signal(SIGABRT, signal_handler);
        signal(SIGUSR1, signal_handler);
        signal(SIGUSR2, signal_handler);

        logger_threaded = new LogThreaded();

        Log* logger = static_cast<Log*>(logger_threaded);

        ZEDReader imu("/dev/i2c-1", "RONIMU", logger);
        imu.startThread();
        // sleep for a bit
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));        
        
        int nodes[4] = {leftNode, rightNode, leftShoulder, rightShoulder};
        bool nodeRev[4] = {true, false, false, true};
        driveSystem drive_system(nodes, nodeRev, 4, "driveSystem", logger);
        drive_system.startThread();

        controllerThread controller("Controller", logger, drive_system);
        controller.startThread();

        JoystickState js_state;
        imuData_t imu_state;
        std::uint64_t imu_time_prev = 0;
        bool imu_error = false;
        bool drive_system_error = false;

        systemDesired_t      shared_desired_state;

        controllerSettings_t shared_controller_settings;
        controllerSettings_t shared_controller_settings_prev = shared_controller_settings; 
        imuData_t            shared_imu_state;

        
        auto last_publish = get_time_nano();
        auto last_run     = get_time_nano();

        // shared memory
        SData<imuData_t>             shared_imu_map(logger, shared_imu_file,      true);
        // SData<systemDesired_t>       shared_desired_map("shared_desired_map",   logger, shared_desired_file,  false);
        // SData<controllerSettings_t>  shared_settings_map("shared_settings_map", logger, shared_settings_file, false);

        while (!shared_imu_map.isMemoryMapped()) //||
            //    !shared_desired_map.isMemoryMapped() ||
            //    !shared_settings_map.isMemoryMapped())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        position_t dr_pos;
        position_t pos;
        float rel_x, rel_y;

        bool mesh_saved = false;
        int mesh_save_count = 0;
        
        while(!time_to_quit){
            last_run = get_time_nano();

            imu.getIMUData(imu_state);
            if (imu_state.timestamp != imu_time_prev)
            {
                imu_time_prev = imu_state.timestamp;
                imu_state.timestamp = get_time_nano();
                shared_imu_map.setData(imu_state);
                // logger->pushEvent("Updated Controller IMU time Diff: " + std::to_string(get_time_nano() - imu_state.timestamp)); 
            }

            // // request a transition only if the desired state has changed
            // if (shared_desired_state.state != shared_desired_state_prev.state){
            //     request_transition(shared_desired_state.state);
            //     shared_desired_state_prev.state = shared_desired_state.state;
            //     std::cout << "The desired state has changed to: " << std::to_string(shared_desired_state.state) << std::endl;
            // }


            // calc position guess
            // drive_system.calcDeadRec(imu_state.angles.yaw);
            // drive_system.getDRAbsolute(dr_pos.x, dr_pos.y);

            // pos = slam_state.position;
            // // determine which position to use
            // if (slam_state.tracking_state){
            //     actual_state.positionStatus = (int)PositionState::SLAM;
            //     drive_system.requestDRReset();
            // } else {
            //     actual_state.positionStatus = (int)PositionState::DEAD_RECKONING;
            //     drive_system.getDRReletive(rel_x, rel_y);
            //     pos.x += rel_x;
            //     pos.y += rel_y;
            // } 


            // if ((get_time_nano() - last_publish) > publish_loop){
            //     // shared_actual_map.setData(shared_actual_state);                    
            //     // logger->pushEvent("roll: " + std::to_string(shared_actual_state.robot.angles.roll) + " pitch: " + std::to_string(shared_actual_state.robot.angles.pitch) + " yaw: " + std::to_string(shared_actual_state.robot.angles.yaw));
            //     last_publish = get_time_nano();
            // }
            
            // if ((get_time_nano() - last_run) > main_loop)
            // {
            //     auto loop_dur_in_seconds = (get_time_nano() - last_run) / 1000000.0;
            //     auto main_loop_in_seconds = main_loop / 1000000000.0;
            //     logger->pushEvent("[MAIN] Main loop over time. actual: " + std::to_string(loop_dur_in_seconds) + " s should be: " + std::to_string(main_loop_in_seconds) + " s");
            // } 
            // else
            // {
                std::this_thread::sleep_for(std::chrono::nanoseconds(main_loop - (get_time_nano() - last_run)));
            // }
        }

	    fsm_handle::dispatch(SHUTDOWN());
        drive_system.stopThread();
        imu.stopThread();

        return 0;
}
