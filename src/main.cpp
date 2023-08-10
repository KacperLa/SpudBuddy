#include<main.h>
#include "iostream"
#include <sys/resource.h>

using fsm_handle = Robot;

void zmq_sockets_open()
{
    // Open and bind zmq sockets
    socket_pub.bind(socket_pub_address);
    socket_pub.set(zmq::sockopt::linger, 0);

    socket_rep.bind(socket_rep_address);
    socket_rep.set(zmq::sockopt::linger, 0);
    rep_poller.add(socket_rep, zmq::event_flags::pollin);
}

void zmq_sockets_close()
{
    socket_pub.close();
    socket_rep.close();
    context.shutdown();
}

void zmq_sockets_initialize()
{
    zmq_sockets_open();
}

json status_of_robot(){
    int state = fsm_handle::get_state().state;
    json j = {{"success", true},{"message", state}};
    return j;
}

json request_transition(int action){
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
    int current_state = fsm_handle::get_state().state;
    json j = {{"success", true},{"message", current_state}};
    return j;
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

json zmq_sockets_parse_message(zmq::message_t *message)
{
    std::string str = std::string(static_cast<char*>(message -> data()), message -> size());
    json j = json::parse(str);
    std::string msg = "[MAIN] ZMQ req/rep got a request: " + str;
    logger.pushEvent(msg);
    return j;
}

void zmq_sockets_poll()
{
    std::vector<zmq::poller_event<zmq::socket_t>> rep_events(1);
    zmq::message_t message;
    auto n = rep_poller.wait_all(rep_events, timeout);
    if (n) {
        printf("GOT ZMQ poller event.");
        if (zmq::event_flags::pollin == rep_events[0].events) {
            socket_rep.recv(&message);
            //zmq_sockets_log_message(&message);
            //default response
            json reply_json = default_response;
            json parsed_message = zmq_sockets_parse_message(&message);

            if (parsed_message.at("request") == "status") {
                reply_json = status_of_robot();
            } else if (parsed_message.at("request") == "request_transition"){
                if (parsed_message.contains("action")){
                    reply_json = request_transition(parsed_message.at("action"));
                }
            } else if (parsed_message.at("request") == "request_pid_coeffs"){
                reply_json = {{"success", true},{"message", fsm_handle::getControllerCoeffs()}};
            } else if (parsed_message.at("request") == "set_pid_coeffs"){
                if (parsed_message.contains("pP") &&
                    parsed_message.contains("pI") &&
                    parsed_message.contains("pD") &&
                    parsed_message.contains("vP") &&
                    parsed_message.contains("vI") &&
                    parsed_message.contains("vD") &&
                    parsed_message.contains("yP") &&
                    parsed_message.contains("yI") &&
                    parsed_message.contains("yD") &&
                    parsed_message.contains("pitchZero")){
                    fsm_handle::setControllerCoeffs(parsed_message);
                }
                reply_json = {{"success", true},{"message", fsm_handle::getControllerCoeffs()}};
            } 
            
            // else if (parsed_message.at("request") == "set_pid"){
            //     if (parsed_message.contains("P") && parsed_message.contains("I") && parsed_message.contains("D")){
            //         reply_json = set_pid(parsed_message.at("P"), parsed_message.at("I"), parsed_message.at("D"));
            //     }
            // } else if (parsed_message.at("request") == "get_pid"){
            //     reply_json = get_pid();
            // }

            std::string out_str = reply_json.dump();
            std::string msg = "[MAIN] Replying with: " + out_str;
            logger.pushEvent(msg);
            zmq::message_t reply(out_str.c_str(), out_str.size());
            socket_rep.send(reply);
        }
    }
}

void zmq_sockets_publish_state(RobotState actual_state, RobotState desired_state, JoystickState &js, json j0, json j1){
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = now.time_since_epoch();
        auto timestamp = std::chrono::duration_cast<std::chrono::duration<double>>(duration).count();
        
        json j = {
                    {"robotState",
                        {
                            {"desired",
                                {
                                    {"angles",
                                        {
                                            {"yaw", desired_state.angles.yaw},
                                            {"pitch", desired_state.angles.pitch},
                                            {"roll", desired_state.angles.roll}
                                        }
                                    },
                                    {"rates",
                                        {
                                            {"gyro_yaw", desired_state.rates.gyro_yaw},
                                            {"gyro_pitch", desired_state.rates.gyro_pitch},
                                            {"gyro_roll", desired_state.rates.gyro_roll}
                                        }
                                    },
                                    {"velocity", desired_state.velocity},
                                    {"leftVelocity", desired_state.leftVelocity},
                                    {"rightVelocity", desired_state.rightVelocity}, 
                                    {"state", desired_state.state},
                                }
                            },
                            {"actual",
                                {
                                    {"angles",
                                        {
                                            {"yaw", actual_state.angles.yaw},
                                            {"pitch", actual_state.angles.pitch},
                                            {"roll", actual_state.angles.roll}
                                        }
                                    },
                                    {"rates",
                                        {
                                            {"gyro_yaw", actual_state.rates.gyro_yaw},
                                            {"gyro_pitch", actual_state.rates.gyro_pitch},
                                            {"gyro_roll", actual_state.rates.gyro_roll}
                                        }
                                    },
                                    {"velocity", actual_state.velocity},
                                    {"leftVelocity", actual_state.leftVelocity},
                                    {"rightVelocity", actual_state.rightVelocity}, 
                                    {"state", actual_state.state},
                                }
                            }
                        }
                    },
                    {"DriveSystem",
                        {
                            {"axis_0", j0},
                            {"axis_1", j1}
                        }
                    },
                    {"js", 
                        {
                            {"x", js.x},
                            {"y", js.y}
                        }
                    }, 
                    {"timestamp", timestamp}
                };
        std::string out_str = j.dump();
        zmq::message_t message(out_str.c_str(), out_str.size());
        socket_pub.send(message);
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

        zmq_sockets_initialize();

        logger.startProcessing();

        // Joystick joystick("Joystick", logger);
        // joystick.addDevice("/dev/input/js0");
        // joystick.startThread();

        Cmd joystick("webJoystick", logger);
        joystick.startThread();

        IMUReader imu("/dev/i2c-1", "IMU", logger);
        imu.startThread();

        fsm_handle::set_logger(&logger);
        fsm_handle::start();

        JoystickState js_state;
        IMUState imu_state;
        bool imu_error = false;
        bool drive_system_error = false;
        RobotState actual_state;
        RobotState desired_state;

        std::chrono::duration<double> loop_time(1.0 / 20.0); // 20 Hz
        auto last_publish = std::chrono::high_resolution_clock::now();
        auto last_run = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> main_loop(1.0 / 20.0); // 20 Hz

        while(!time_to_quit){
                last_run = std::chrono::high_resolution_clock::now();

                joystick.getState(js_state);
                imu_error = imu.getState(imu_state);
                fsm_handle::updateIMU(imu_state);
              
                drive_system_error = fsm_handle::requestDriveSystemStatus();
                actual_state = fsm_handle::get_state();
                if ((imu_error || drive_system_error) && (actual_state.state != RobotStates::ERROR && actual_state.state != RobotStates::IDLE)){
                    std::string err_msg = "[MAIN] imu_error = " + std::to_string(imu_error) + " drive stystem error = " + std::to_string(drive_system_error);
                    std::cout << err_msg << std::endl;
                    logger.pushEvent(err_msg);
                    fsm_handle::dispatch(FAIL());
                }
                
                cmd_data new_cmd;
                new_cmd.v = js_state.y / 100.0f;
                new_cmd.o = (js_state.x / 100.0f);
                fsm_handle::dispatch(new_cmd);

                fsm_handle::dispatch(Update());

                zmq_sockets_poll();

                if ((std::chrono::high_resolution_clock::now() - last_publish) > loop_time){
                        zmq_sockets_publish_state(actual_state, desired_state, js_state, fsm_handle::RequestAxisData(0), fsm_handle::RequestAxisData(1));
                        last_publish = std::chrono::high_resolution_clock::now();
                        //system("clear");
                        // std::cout << "\033[2J\033[1;1H";
                        // std::cout << "Robot: current state: " << robot_state << std::endl;
                        // std::cout << "IMU: error state: " << imu_error << std::endl;
                        
                        // std::cout << "CAN: error state: " << drive_system_error << std::endl;
                        // std::string msg = "imu: pitch=" + std::to_string(imu_state.angles.pitch);
                        // logger.pushEvent(msg);
                        // std::cout << "imu: yaw=" << imu_state.angles.yaw << std::endl;
                        //std::cout << "joystick: x=" << js_state.x << " , a=" << js_state.y << std::endl;

                        json j = fsm_handle::RequestAxisData(1);
                        std::string out_j = j.dump();
                        //std::cout << "axis 1 state: " << out_j << std::endl;
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
        imu.stopThread();
        joystick.stopThread();
        logger.stopProcessing();
        zmq_sockets_close();
        return 0;
}
