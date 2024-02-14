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

void calcPositionError(position_t actual, position_t desired, float & error_distance)
{
    float d_x = (desired.x - actual.x);
    float d_y = (desired.y - actual.y);
    error_distance = sqrt(pow(d_x, 2) + pow(d_y, 2));
}

void calcDesiredYaw(position_t actual, position_t desired, float & desired_yaw)
{
    float d_x = (desired.x - actual.x);
    float d_y = (desired.y - actual.y);
    desired_yaw = (atan(d_y / d_x)/M_PI) * 180.0f;
    
    if (d_x < 0 && d_y < 0)
    {
        desired_yaw = -1.0f * (180.0f - desired_yaw);
    } 
    else if (d_x < 0 && d_y > 0)
    {
        desired_yaw = (180.0f + desired_yaw);
    } 
    
    desired_yaw = -1.0f * desired_yaw;
}


    
    
bool Controller::calculateOutput(driveSystemState_t drive_state, imuData_t imu, trackingState_t tracking, systemDesired_t command, controllerSettings_t controller_settings, float& outputLeft, float& outputRight)
{
    // Positon error
    float position_error;   
    calcPositionError(tracking.position, command.position, position_error);

    if (fabs(position_error) < controller_settings.dead_zone_pos && 
        tracking.is_tracking == false)
    {
        position_error = 0;
    }

    // Position PID
    float position_output = position_pid.getOutput(position_error);
    float desired_velocity = -1.0f * position_output;

    // ====================================
    // Yaw error
    float yaw_error;
    calcDesiredYaw(tracking.position, command.position, command.angles.yaw);
    calcTheta(command.angles.yaw, imu.angles.yaw, yaw_error);

    if (fabs(yaw_error) < controller_settings.dead_zone_yaw)
    {
        yaw_error = 0;
    }

    // Yaw PID
    float yaw_pos_output = yaw_pid.getOutput(yaw_error);
    float desired__yaw = -1.0f * yaw_pos_output;

    // ====================================
    // Velocity error
    float velocity_error;
    float actual_velocity = (drive_state.axis[leftNode].velocity + drive_state.axis[rightNode].velocity) / 2.0f;
    velocity_error = desired_velocity - actual_velocity;

    // Velocity PID
    double velocity_output;
    velocity_output = velocity_pid.getOutput(velocity_error);


    // ====================================
    // Yaw rate error
    float yaw_rate_error = (drive_state.axis[leftNode].velocity - drive_state.axis[rightNode].velocity) + command.rates.gyro_yaw;
    
    if (fabs(yaw_rate_error) < controller_settings.dead_zone_yaw_rate)
    {
        yaw_rate_error = 0;
    }
    
    // Yaw rate PID
    double yaw_output = yaw_rate_pid.getOutput(yaw_rate_error);
        
   
    // ====================================
    // Pitch error
    double pitch_error = command.angles.pitch - (imu.angles.pitch + velocity_output);

    if (fabs(pitch_error) > 25){
        // logger->pushEvent("[FSM] Robot fell, going into error state." + std::to_string(actual_state.angles.pitch));
        std::cout << "Pitch error too large! is: " <<  imu.angles.pitch << " should be: " << command.angles.pitch << std::endl;
        return false;    
    }

    // Pitch PID
    double pitch_output   =  (pitch_error * controller_settings.pitch_p) - (imu.rates.gyro_pitch * controller_settings.pitch_d);

    if (fabs(pitch_output) < controller_settings.dead_zone_pitch){
        pitch_output = 0;
    }

    // ====================================
    // Set Outputs
    if (!std::isnan(pitch_output) && 
        !std::isnan(yaw_output) && 
        !std::isnan(velocity_output)) 
    {
        outputLeft  = pitch_output + yaw_output;
        outputRight = pitch_output - yaw_output;
    }
    else
    {
        outputLeft  = 0;
        outputRight = 0;
    }
    
    // std::cout << std::fixed << std::setprecision(2) << "yaw_e: " << yaw_error << " d_yaw: " << desired_yaw << " a_yaw: " << actual_state.angles.yaw << " d_x: " << d_x << " d_y" << d_y << std::endl;
    // std::cout << std::fixed << std::setprecision(2) << "distance: " << position_error << std::endl;
    
    return true;
}
 
void Controller::get_settings(controllerSettings_t & settings) {
    std::lock_guard<std::mutex> lock(mtx);
    pitch_pid.getGains(settings.pitch_p, settings.pitch_i, settings.pitch_d);
    velocity_pid.getGains(settings.velocity_p, settings.velocity_i, settings.velocity_d);
    yaw_rate_pid.getGains(settings.yaw_rate_p, settings.yaw_rate_i, settings.yaw_rate_d);
    yaw_pid.getGains(settings.yaw_p, settings.yaw_i, settings.yaw_d);
    position_pid.getGains(settings.positon_p, settings.positon_i, settings.positon_d);
    settings.pitch_zero = m_settings.pitch_zero;
}

void Controller::set_settings(const controllerSettings_t & settings) {
    std::lock_guard<std::mutex> lock(mtx);
    pitch_pid.setPID(settings.pitch_p, settings.pitch_i, settings.pitch_d);
    velocity_pid.setPID(settings.velocity_p, settings.velocity_i, settings.velocity_d);
    yaw_rate_pid.setPID(settings.yaw_rate_p, settings.yaw_rate_i, settings.yaw_rate_d);
    m_settings.pitch_zero = settings.pitch_zero;
    yaw_pid.setPID(settings.yaw_p, settings.yaw_i, settings.yaw_d);
    position_pid.setPID(settings.positon_p, settings.positon_i, settings.positon_d);
}
