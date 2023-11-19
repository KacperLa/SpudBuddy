#include<main.h>
#include "iostream"
#include <sys/resource.h>



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

// json set_pid(const double new_P, const double new_I, const double new_D){
//     //int state = fsm_handle::
    
//     double P, I, D = 0.0;
//     P = fsm_handle::get_kp();
//     I = fsm_handle::get_ki();
//     D = fsm_handle::get_kd();
//     json j = {{"success", true},{"message", {{"P", P}, {"I", I}, {"D", D}}}};
//     return j;
// }

// json get_pid(){
//     double P, I, D = 0.0;
//     P = fsm_handle::get_kp();
//     I = fsm_handle::get_ki();
//     D = fsm_handle::get_kd();
//     json j = {{"success", true},{"message", {{"P", P}, {"I", I}, {"D", D}}}};
//     return j;
// }


// void zmq_sockets_poll()
// {
//     std::vector<zmq::poller_event<zmq::socket_t>> rep_events(1);
//     zmq::message_t message;
//     auto n = rep_poller.wait_all(rep_events, timeout);
//     if (n) {
//         printf("GOT ZMQ poller event.");
//         if (zmq::event_flags::pollin == rep_events[0].events) {
//             printf("recv.");
//             socket_rep.recv(&message);
//             printf("recv done.");

//             //zmq_sockets_log_message(&message);
//             //default response
//             json reply_json = default_response;
//             json parsed_message = zmq_sockets_parse_message(&message);

//             if (parsed_message.at("request") == "status") {
//                 reply_json = status_of_robot();
//             } else if (parsed_message.at("request") == "request_transition"){
//                 if (parsed_message.contains("action")){
//                     reply_json = request_transition(parsed_message.at("action"));
//                 }
//             } else if (parsed_message.at("request") == "request_pid_coeffs"){
//                 reply_json = {{"success", true},{"message", fsm_handle::getControllerCoeffs()}};
//             } else if (parsed_message.at("request") == "set_pid_coeffs"){
//                 if (parsed_message.contains("pP") &&
//                     parsed_message.contains("pI") &&
//                     parsed_message.contains("pD") &&
//                     parsed_message.contains("vP") &&
//                     parsed_message.contains("vI") &&
//                     parsed_message.contains("vD") &&
//                     parsed_message.contains("yP") &&
//                     parsed_message.contains("yI") &&
//                     parsed_message.contains("yD") &&
//                     parsed_message.contains("pitchZero")){
//                     fsm_handle::setControllerCoeffs(parsed_message);
//                 }
//                 reply_json = {{"success", true},{"message", fsm_handle::getControllerCoeffs()}};
//             } 
            
//             // else if (parsed_message.at("request") == "set_pid"){
//             //     if (parsed_message.contains("P") && parsed_message.contains("I") && parsed_message.contains("D")){
//             //         reply_json = set_pid(parsed_message.at("P"), parsed_message.at("I"), parsed_message.at("D"));
//             //     }
//             // } else if (parsed_message.at("request") == "get_pid"){
//             //     reply_json = get_pid();
//             // }

//             std::string out_str = reply_json.dump();
//             std::string msg = "[MAIN] Replying with: " + out_str;
//             logger.pushEvent(msg);
//             zmq::message_t reply(out_str.c_str(), out_str.size());
//             socket_rep.send(reply);
//         }
//     }
// }

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
    logger.pushEvent(msg);
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
        setpriority(PRIO_PROCESS, getpid(), 1);

        signal(SIGINT, signal_handler);
        signal(SIGABRT, signal_handler);
        signal(SIGUSR1, signal_handler);
        signal(SIGUSR2, signal_handler);

        logger.startThread();

        ZEDReader imu("/dev/i2c-1", "IMU", logger);
        imu.startThread();
        
        int nodes[2] = {fsm_handle::leftNode, fsm_handle::rightNode};
        bool nodeRev[2] = {true, false};
        DriveSystem drive_system(nodes, nodeRev, 2, "DriveSystem", logger);
        drive_system.startThread();

        fsm_handle::set_logger(&logger);
        fsm_handle::setDriveSystem(&drive_system);
        fsm_handle::start();

        JoystickState js_state;
        IMUState imu_state;
        bool imu_error = false;
        bool drive_system_error = false;

        robot_state_t        actual_state;
        sytemState_t         shared_actual_state;
        systemDesired_t      shared_desired_state;
        systemDesired_t      shared_desired_state_prev = shared_desired_state;
        controllerSettings_t shared_controller_settings;
        controllerSettings_t shared_controller_settings_prev = shared_controller_settings; 
        slamState_t          slam_state;
        std::cout << "size of desired: " << sizeof(systemDesired_t) << std::endl;
        std::chrono::duration<double> loop_time(1.0 / 20.0); // 20 Hz
        auto last_publish = std::chrono::high_resolution_clock::now();
        auto last_run = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> main_loop(1.0 / 20.0); // 20 Hz

        // shared memory
        SData<sytemState_t>         shared_actual_map("shared_actual_map",     logger, shared_actual_file,   semaphore_actual_file,   true);
        SData<systemDesired_t>      shared_desired_map("shared_desired_map",   logger, shared_desired_file,  semaphore_desired_file,  false);
        SData<controllerSettings_t> shared_settings_map("shared_settings_map", logger, shared_settings_file, semaphore_settings_file, false);

        shared_actual_map.startThread();
        shared_desired_map.startThread();
        shared_settings_map.startThread();

        while(!time_to_quit){
                last_run = std::chrono::high_resolution_clock::now();
    
                actual_state  = fsm_handle::get_state();
                imu_error     = imu.getState(imu_state);

                shared_desired_map.getData(shared_desired_state);
                shared_settings_map.getData(shared_controller_settings);

                if ((imu_error || fsm_handle::requestDriveSystemStatus()) &&
                    (actual_state.state != RobotStates::ERROR && actual_state.state != RobotStates::IDLE)) 
                    {
                    std::string err_msg = "[MAIN] imu_error = " + std::to_string(imu_error) + " drive stystem error = " + std::to_string(drive_system_error);
                    std::cout << err_msg << std::endl;
                    logger.pushEvent(err_msg);
                    fsm_handle::dispatch(FAIL());
                } else {
                    fsm_handle::updateIMU(imu_state);
                }

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
                    std::cout << "deadZone: " << std::to_string(shared_controller_settings.dead_zone) << std::endl;
                }
                
                
                cmd_data new_cmd;
                new_cmd.v = (fabs(shared_desired_state.joystick.y) > 5) ? shared_desired_state.joystick.y / 100.0f : 0.0f;
                new_cmd.w = (fabs(shared_desired_state.joystick.x) > 5) ? shared_desired_state.joystick.x / -100.0f : 0.0f;
                fsm_handle::dispatch(new_cmd);

                fsm_handle::dispatch(Update());

                // calc position guess
                position_t dr_pos;
                drive_system.calcDeadRec(imu_state.angles.yaw);
                drive_system.getDRAbsolute(dr_pos.x, dr_pos.y);
                fsm_handle::setDRPosition(dr_pos);

                imu.getState(slam_state);
                position_t slam_pos;
                slam_pos = {slam_state.x, slam_state.y};
                fsm_handle::setSlamPosition(slam_pos);
                
                // determine which position to use
                if (slam_state.tracking_state){
                    actual_state.positionStatus = (int)PositionState::SLAM;
                    drive_system.requestDRReset();
                } else {
                    actual_state.positionStatus = (int)PositionState::DEAD_RECKONING;
                    float rel_x, rel_y;
                    drive_system.getDRReletive(rel_x, rel_y);
                    slam_pos.x += rel_x;
                    slam_pos.y += rel_y;
                } 

                fsm_handle::setPosition(slam_pos);

                if ((std::chrono::high_resolution_clock::now() - last_publish) > loop_time){
                    shared_actual_state.actual = actual_state;
                    drive_system.requestVbusVoltage();
                    drive_system.getState(shared_actual_state.driveSystem.axis_0, 0);
                    drive_system.getState(shared_actual_state.driveSystem.axis_1, 1);
                    fsm_handle::getControllerSettings(shared_actual_state.controller_settings);
                    
                    // std::cout << "x: " << std::to_string(shared_actual_state.actual.positionSlam.x) << " y: " << std::to_string(shared_actual_state.actual.positionSlam.y) << std::endl;

                    shared_actual_map.setData(shared_actual_state);
                    // std::cout << "x: " << std::to_string(shared_desired_state.joystick.x) << " y: " << std::to_string(shared_desired_state.joystick.y) << std::endl;
                    
                    last_publish = std::chrono::high_resolution_clock::now();
                }

                if ((std::chrono::high_resolution_clock::now() - last_run) > main_loop){
                    auto loop_dur = std::chrono::high_resolution_clock::now() - last_run;
                    auto loop_dur_in_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(loop_dur);

                    std::ostringstream stream;
                    stream << loop_dur_in_seconds.count() << "s";
                    std::string duration_string = stream.str();
                    std::string msg = "[MAIN] Main loop over time. actual: " + duration_string;
                    logger.pushEvent(msg);
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }

	    fsm_handle::dispatch(SHUTDOWN());
        drive_system.stopThread();
        imu.stopThread();

        logger.stopThread();
        return 0;
}
