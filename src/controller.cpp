#include "controller.h"
#include <iostream>
#include <iomanip>

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
    // // Calculate positon error
    // float position_error = sqrt(pow(desired_state.position.x - actual_state.position.x, 2) + pow(desired_state.position.y - actual_state.position.y, 2));
        
    float d_x = (desired_state.position.x - actual_state.position.x);
    float d_y = (desired_state.position.y - actual_state.position.y);

    // Calculate yaw error
    float yaw_error = 0.0f;
    float desired_yaw = (atan(d_y / d_x)/M_PI) * 180.0f;

    if (d_x < 0 && d_y < 0)
    {
        desired_yaw = -1.0f * (180.0f - desired_yaw);
    } 
    else if (d_x < 0 && d_y > 0)
    {
        desired_yaw = (180.0f + desired_yaw);
    } 
    
    desired_yaw = -1.0f * desired_yaw;

    calcTheta(desired_yaw, actual_state.angles.yaw, yaw_error);
    if (fabs(yaw_error) < 2){
        yaw_error = 0;
    }
    std::cout << std::fixed << std::setprecision(2) << "yaw_e: " << yaw_error << " d_yaw: " << desired_yaw << " a_yaw: " << actual_state.angles.yaw << " d_x: " << d_x << " d_y" << d_y << std::endl;

    // Calculate yaw pid output
    float yaw_pos_output = yaw_pid.getOutput(yaw_error);
    desired_state.rates.gyro_yaw = -1.0f * yaw_pos_output;

    // // if position error is with iin the dead zome dont move
    // if (fabs(position_error) > m_settings.dead_zone) {
    //     // Calculate position pid output
    //     float position_output = position_pid.getOutput(position_error);
    //     desired_state.velocity += position_output;
    // }
        
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
    settings.yaw_p = yaw_pid.getP();
    settings.yaw_i = yaw_pid.getI();
    settings.yaw_d = yaw_pid.getD();
    settings.positon_p = position_pid.getP();
    settings.positon_i = position_pid.getI();
    settings.positon_d = position_pid.getD();
    settings.dead_zone = m_settings.dead_zone;
}

void Controller::set_settings(const controllerSettings_t & settings) {
    std::unique_lock<std::mutex> lock(mtx);
    pitch_pid.setPID(settings.pitch_p, settings.pitch_i, settings.pitch_d);
    velocity_pid.setPID(settings.velocity_p, settings.velocity_i, settings.velocity_d);
    yaw_rate_pid.setPID(settings.yaw_rate_p, settings.yaw_rate_i, settings.yaw_rate_d);
    m_settings.pitch_zero = settings.pitch_zero;
    yaw_pid.setPID(settings.yaw_p, settings.yaw_i, settings.yaw_d);
    position_pid.setPID(settings.positon_p, settings.positon_i, settings.positon_d);
    m_settings.dead_zone = settings.dead_zone;
}
