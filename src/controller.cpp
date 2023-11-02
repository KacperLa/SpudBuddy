#include "controller.h"
#include <iostream>


Controller::Controller() {
    pitch_pid.setOutputLimits(-1.0, 1.0);
    yaw_rate_pid.setOutputLimits(-1.0, 1.0);
    velocity_pid.setOutputLimits(-15.0, 15.0);
    velocity_pid.setMaxIOutput(15.0);

}

Controller::~Controller() {
    // Destructor code here
}

float mod(float a, float n)
{
    return (a - floor(a/n) * n);
}

void calcTheta(float cmd, float actual, float & error)
{
    float b = mod(((actual - cmd) + (2.0 * 180)), (2.0 * 180));
    float f = mod(((cmd - actual) + (2.0 * 180)), (2.0 * 180));

    if (b < 180) {
        error =  (-1 * b);
    }
    else {
        error =  f;
    }
}

bool Controller::calculateOutput(robot_state_t actual_state, robot_state_t desired_state, float& outputLeft, float& outputRight)
{
    float yaw_rate_error = (actual_state.leftVelocity - actual_state.rightVelocity) + desired_state.rates.gyro_yaw;
    double yaw_output = yaw_rate_pid.getOutput(yaw_rate_error); //(yaw_error * yaw_rate_pid.getP()) + (actual_state.rates.gyro_yaw * yaw_rate_pid.getD());
    
    if (fabs(yaw_output) < 0.001f){
        yaw_output = 0;
    }
    
    double velocity_output = velocity_pid.getOutput(actual_state.velocity, (-1*desired_state.velocity));

    double pitch_error = desired_state.angles.pitch - (actual_state.angles.pitch + velocity_output);
    double pitch_output   =  (pitch_error * pitch_pid.getP()) - (actual_state.rates.gyro_pitch * pitch_pid.getD());

    if (fabs(pitch_output) < 0.001f){
        pitch_output = 0;
    }

   if (fabs(actual_state.angles.pitch - (desired_state.angles.pitch)) > 25){
        std::cout << "Pitch error too large! is: " <<  actual_state.angles.pitch << " should be: " << desired_state.angles.pitch << std::endl;
        return false;
    }

    if (!std::isnan(pitch_output) && !std::isnan(yaw_output) && !std::isnan(velocity_output)) {
        outputLeft  = pitch_output + yaw_output;
        outputRight = pitch_output - yaw_output;
    } else {
        outputLeft  = 0;
        outputRight = 0;
    }
    return true;
}
 
void Controller::get_settings(controllerSettings_t & settings) {
    std::unique_lock<std::mutex> lock(mtx);
    settings.pitch_p = pitch_pid.getP();
    settings.pitch_i = pitch_pid.getI();
    settings.pitch_d = pitch_pid.getD();
    settings.velocity_p = velocity_pid.getP();
    settings.velocity_i = velocity_pid.getI();
    settings.velocity_d = velocity_pid.getD();
    settings.yaw_rate_p = yaw_rate_pid.getP();
    settings.yaw_rate_i = yaw_rate_pid.getI();
    settings.yaw_rate_d = yaw_rate_pid.getD();
    settings.pitch_zero = m_settings.pitch_zero;
}

void Controller::set_settings(const controllerSettings_t & settings) {
    std::unique_lock<std::mutex> lock(mtx);
    pitch_pid.setPID(settings.pitch_p, settings.pitch_i, settings.pitch_d);
    velocity_pid.setPID(settings.velocity_p, settings.velocity_i, settings.velocity_d);
    yaw_rate_pid.setPID(settings.yaw_rate_p, settings.yaw_rate_i, settings.yaw_rate_d);
    m_settings.pitch_zero = settings.pitch_zero;
}
