#include<main.h>
#include "iostream"
#include <sys/resource.h>
#include "common.h"
#include <sched.h>


void request_transition(int action){
    typedef enum {
        REQ_START = 1,
        REQ_RESET = 2,
        REQ_FAIL = 3
    } trans_t;

    switch (action){
        case REQ_START:
            fsm_handle::dispatch(START());
            break;
        case REQ_RESET:
            fsm_handle::dispatch(RESET());
            break;
        case REQ_FAIL:
            fsm_handle::dispatch(FAIL());
            break;
        default:
            break;
    }
}

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
        param.sched_priority = 1;
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

        ZEDReader imu("/dev/i2c-1", "IMU", logger);
        imu.startThread();
        
        int nodes[2] = {fsm_handle::leftNode, fsm_handle::rightNode};
        bool nodeRev[2] = {true, false};
        DriveSystem drive_system(nodes, nodeRev, 2, "DriveSystem", logger);
        drive_system.startThread();

        fsm_handle::set_logger(logger);
        fsm_handle::setDriveSystem(&drive_system);
        fsm_handle::start();

        JoystickState js_state;
        IMUState imu_state;
        bool imu_error = false;
        bool drive_system_error = false;

        robot_state_t        actual_state;
        systemState_t         shared_actual_state;
        systemDesired_t      shared_desired_state;
        systemDesired_t      shared_desired_state_prev = shared_desired_state;
        controllerSettings_t shared_controller_settings;
        controllerSettings_t shared_controller_settings_prev = shared_controller_settings; 
        slamState_t          slam_state;
        auto last_publish = get_time_micro();
        auto last_run     = get_time_micro();

        // shared memory
        SData<systemState_t>         shared_actual_map("shared_actual_map",     logger, shared_actual_file,   true);
        SData<systemDesired_t>       shared_desired_map("shared_desired_map",   logger, shared_desired_file,  false);
        SData<controllerSettings_t>  shared_settings_map("shared_settings_map", logger, shared_settings_file, false);

        while (!shared_actual_map.isMemoryMapped() ||
               !shared_desired_map.isMemoryMapped() ||
               !shared_settings_map.isMemoryMapped())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        position_t dr_pos;
        position_t slam_pos;
        float rel_x, rel_y;


        while(!time_to_quit){
                last_run = get_time_micro();
    
                imu_error = imu.getState(imu_state);
                fsm_handle::updateIMU(imu_state);

                shared_actual_state = fsm_handle::getActualState();

                shared_desired_map.getData(shared_desired_state);
                shared_settings_map.getData(shared_controller_settings);

                // request a transition only if the desired state has changed
                if (shared_desired_state.state != shared_desired_state_prev.state){
                    request_transition(shared_desired_state.state);
                    shared_desired_state_prev.state = shared_desired_state.state;
                    std::cout << "The desired state has changed to: " << std::to_string(shared_desired_state.state) << std::endl;
                }

                // update desired positon if it has changed
                if (memcmp(&shared_desired_state.position, &shared_desired_state_prev.position, sizeof(position_t)) != 0){
                    fsm_handle::setDesiredPosition(shared_desired_state.position);
                    shared_desired_state_prev.position = shared_desired_state.position;
                    std::cout << "The desired position has changed." << std::endl;
                }

                // update controller settings only if they have changed
                if (memcmp(&shared_controller_settings, &shared_controller_settings_prev, sizeof(controllerSettings_t)) != 0){
                    fsm_handle::setControllerSettings(shared_controller_settings);
                    shared_controller_settings_prev = shared_controller_settings;
                    std::cout << "The controller settings have changed." << std::endl;
                    // print the settings
                    std::cout << "pP: " << std::to_string(shared_controller_settings.pitch_p) << std::endl;
                    std::cout << "pI: " << std::to_string(shared_controller_settings.pitch_i) << std::endl;
                    std::cout << "pD: " << std::to_string(shared_controller_settings.pitch_d) << std::endl;
                    std::cout << "vP: " << std::to_string(shared_controller_settings.velocity_p) << std::endl;
                    std::cout << "vI: " << std::to_string(shared_controller_settings.velocity_i) << std::endl;
                    std::cout << "vD: " << std::to_string(shared_controller_settings.velocity_d) << std::endl;
                    std::cout << "yP: " << std::to_string(shared_controller_settings.yaw_rate_p) << std::endl;
                    std::cout << "yI: " << std::to_string(shared_controller_settings.yaw_rate_i) << std::endl;
                    std::cout << "yD: " << std::to_string(shared_controller_settings.yaw_rate_d) << std::endl;
                    std::cout << "pitchZero: " << std::to_string(shared_controller_settings.pitch_zero) << std::endl;
                    std::cout << "oP: " << std::to_string(shared_controller_settings.yaw_p) << std::endl;
                    std::cout << "oI: " << std::to_string(shared_controller_settings.yaw_i) << std::endl;
                    std::cout << "oD: " << std::to_string(shared_controller_settings.yaw_d) << std::endl;            
                    std::cout << "dP: " << std::to_string(shared_controller_settings.positon_p) << std::endl;
                    std::cout << "dI: " << std::to_string(shared_controller_settings.positon_i) << std::endl;
                    std::cout << "dD: " << std::to_string(shared_controller_settings.positon_d) << std::endl;
                    std::cout << "deadZone: " << std::to_string(shared_controller_settings.dead_zone_pos) << std::endl;
                }
                
                fsm_handle::dispatch(Update());

                // calc position guess
                drive_system.calcDeadRec(imu_state.angles.yaw);
                drive_system.getDRAbsolute(dr_pos.x, dr_pos.y);
                fsm_handle::setDRPosition(dr_pos);

                imu.getState(slam_state);
                slam_pos = {slam_state.x, slam_state.y, slam_state.z};
                fsm_handle::setSlamPosition(slam_pos);
                
                // determine which position to use
                if (slam_state.tracking_state){
                    actual_state.positionStatus = (int)PositionState::SLAM;
                    drive_system.requestDRReset();
                } else {
                    actual_state.positionStatus = (int)PositionState::DEAD_RECKONING;
                    drive_system.getDRReletive(rel_x, rel_y);
                    slam_pos.x += rel_x;
                    slam_pos.y += rel_y;
                } 

                fsm_handle::setPosition(slam_pos);

                if ((get_time_micro() - last_publish) > publish_loop){
                    drive_system.requestVbusVoltage();
                    fsm_handle::getControllerSettings(shared_actual_state.controller);
                    shared_actual_map.setData(shared_actual_state);                    
                    // logger->pushEvent("roll: " + std::to_string(shared_actual_state.robot.angles.roll) + " pitch: " + std::to_string(shared_actual_state.robot.angles.pitch) + " yaw: " + std::to_string(shared_actual_state.robot.angles.yaw));
                    last_publish = get_time_micro();
                }
                
                if ((get_time_micro() - last_run) > main_loop)
                {
                    auto loop_dur_in_seconds = (get_time_micro() - last_run) / 1000000.0;
                    auto main_loop_in_seconds = main_loop / 1000000.0;
                    logger->pushEvent("[MAIN] Main loop over time. actual: " + std::to_string(loop_dur_in_seconds) + " s should be: " + std::to_string(main_loop_in_seconds) + " s");
                } 
                else
                {
                    std::this_thread::sleep_for(std::chrono::microseconds(main_loop-(get_time_micro() - last_run)));
                }
        }

	    fsm_handle::dispatch(SHUTDOWN());
        drive_system.stopThread();
        imu.stopThread();

        return 0;
}
