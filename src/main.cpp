#include<main.h>
#include "iostream"

//using fsm_handle = Robot;

static void __signal_handler(__attribute__ ((unused)) int dummy)
{
        running=0;
        return;
}

void zmq_sockets_open()
{
    // Open and bind zmq sockets
    socket_pub.bind(socket_pub_address);
    socket_pub.set(zmq::sockopt::linger, 0);

    socket_rep.bind(socket_rep_address);
    socket_rep.set(zmq::sockopt::linger, 0);
    rep_poller.add(socket_rep, zmq::event_flags::pollin);
}

void zmq_sockets_initialize()
{
    zmq_sockets_open();
}

// json status_of_robot(){
//     int state = fsm_handle::get_state();
//     json j = {{"success", true},{"message", state}};
//     return j;
// }

// json set_state(int state){
//         // states = {
//         //     "RESET": 1,
//         //     "START": 2,
//         //     "ESTOP": 3
//         // }
//         switch (state){
//             case 1:
//                 fsm_handle ::dispatch(RESET());
//                 break;
//             case 2:
//                 fsm_handle ::dispatch(START());
//                 break;
//             case 3:
//                 fsm_handle ::dispatch(ERROR());
//                 break;
//             default:
//                 break;
//         }
//     int current_state = fsm_handle::get_state();
//     json j = {{"success", true},{"message", current_state}};
//     return j;
// }

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

// json zmq_sockets_parse_message(zmq::message_t *message)
// {
//     std::string str = std::string(static_cast<char*>(message -> data()), message -> size());
//     json j = json::parse(str);
//     std::cout << "ZMQ req/rep got a request: " << j.at("request") << std::endl;
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
//             socket_rep.recv(&message);
//             //zmq_sockets_log_message(&message);
//             //default response
//             json reply_json = default_response;
//             json parsed_message = zmq_sockets_parse_message(&message);

//             if (parsed_message.at("request") == "status") {
//                 reply_json = status_of_robot();
//             } else if (parsed_message.at("request") == "set_state"){
//                 if (parsed_message.contains("state")){
//                     reply_json = set_state(parsed_message.at("state"));
//                 }
//             } else if (parsed_message.at("request") == "set_pid"){
//                 if (parsed_message.contains("P") && parsed_message.contains("I") && parsed_message.contains("D")){
//                     reply_json = set_pid(parsed_message.at("P"), parsed_message.at("I"), parsed_message.at("D"));
//                 }
//             } else if (parsed_message.at("request") == "get_pid"){
//                 reply_json = get_pid();
//             }

//             std::string out_str = reply_json.dump();
//             std::cout << "Replying with: " << out_str << std::endl;
//             zmq::message_t reply(out_str.c_str(), out_str.size());
//             socket_rep.send(reply);
//         }
//     }
// }

void zmq_sockets_publish_js_state(int &x, int &y){
         auto now = std::chrono::high_resolution_clock::now();
        auto duration = now.time_since_epoch();
        auto timestamp = std::chrono::duration_cast<std::chrono::duration<double>>(duration).count();
        
        json j = {"message", {{"x", x},{"y", y}, {"timestamp", timestamp}}};
        std::string out_str = j.dump();
        zmq::message_t message(out_str.c_str(), out_str.size());
        socket_pub.send(message);
}

int main(int argc, char *argv[])
{
        std::cout << "Starting up.." << std::endl;
        int priority = 99;
        zmq_sockets_initialize();

        Joystick joystick("/dev/input/js0");
        std::thread joystick_event_thread (&Joystick::ReadJoystickLoop, &joystick);

        // fsm_handle::start();
        // fsm_handle::dispatch(Setup());
        JoystickState js_state;
                std::cout << "Hello" << std::endl;
        
        std::chrono::duration<double> loop_time(1.0 / 20.0); // 20 Hz
        auto last_publish = std::chrono::high_resolution_clock::now();
        while(running){
                //zmq_sockets_poll();
                joystick.getState(js_state);
                        //cmd_data new_cmd;
                        // new_cmd.v = (((new_js.stick[5] / 32767.0f) + 1.0f) / 2.0f);
                        // new_cmd.a = (new_js.stick[0] / 32767.0f);
                        //fsm_handle::dispatch(new_cmd);

                if (std::chrono::high_resolution_clock::now() - last_publish > loop_time){
                        zmq_sockets_publish_js_state(js_state.x, js_state.y);
                        last_publish = std::chrono::high_resolution_clock::now();
                        std::cout << "joystick: x=" << js_state.x << " , a=" << js_state.y << std::endl;
                }
                //fsm_handle::dispatch(Update());
                usleep(5000);
        }

        printf("\n");
        fflush(stdout);
        return 0;
}

