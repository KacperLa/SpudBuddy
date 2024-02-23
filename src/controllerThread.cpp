#include "controllerThread.h"

controllerThread::controllerThread(const std::string name, Log* logger, driveSystem& drive_system) :
    ronThread(name, logger, true),
    shared_actual_map(logger, shared_actual_state_file, true)
{
    fsm_handle::set_logger(logger);
    fsm_handle::shared_imu_state = new SData<imuData_t>(logger, shared_imu_file,             false);
    fsm_handle::shared_tracking_state = new SData<trackingState_t>(logger, shared_tracking_state_file,  false);
    fsm_handle::shared_command_state = new SData<systemDesired_t>(logger, shared_command_file,   false);
    fsm_handle::shared_drive_system_state = new SData<driveSystemState_t>(logger, shared_drive_system_file,    false);
    fsm_handle::shared_settings = new SData<controllerSettings_t>(logger, shared_settings_file,    false);
    
    fsm_handle::setDriveSystem(&drive_system);
    fsm_handle::start();
}

controllerThread::~controllerThread()
{
}


void controllerThread::request_transition(int action){
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

void controllerThread::loop() {
    systemDesired_t desired_state;
    systemDesired_t prev_desired_state;
    systemActual_t current_state;
    
    while (running) {
        auto last_run = get_time_nano();
        
        fsm_handle::shared_command_state->getData(desired_state);
        if (desired_state.state != prev_desired_state.state)
        {
            prev_desired_state = desired_state;
            request_transition(desired_state.state);
        }

        // set current state
        current_state.state = fsm_handle::state;
        shared_actual_map.setData(current_state);

        fsm_handle::dispatch(Update());

        auto loop_dur = get_time_nano() - last_run;
        if (loop_dur > loop_time)
        {
            auto loop_dur_in_seconds = loop_dur / 1000000000;
            // log("loop over time. actual: " + std::to_string(loop_dur_in_seconds) + " s should be: " + std::to_string(loop_time/1000000000) + " s ");
        } 
        else
        {
            std::this_thread::sleep_for(std::chrono::nanoseconds(loop_time-loop_dur));
        }
    }
}

