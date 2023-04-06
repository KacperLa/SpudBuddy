#include "controller.h"
#include <iostream>


Controller::Controller() {
    pitch_pid.setOutputLimits(-10.0, 10.0);
    yaw_rate_pid.setOutputLimits(-10.0, 10.0);
    velocity_pid.setOutputLimits(-10.0, 10.0);
    velocity_pid.setMaxIOutput(10.0);
}

Controller::~Controller() {
    // Destructor code here
}

bool Controller::calculateOutput(RobotState actual_state, RobotState desired_state, float& outputLeft, float& outputRight)
{
    if (fabs(actual_state.angles.pitch - desired_state.angles.pitch) > 20){
        std::cout << "Pitch error too large! is: " <<  actual_state.angles.pitch << " should be: " << desired_state.angles.pitch << std::endl;
        return false;
    }

    // if (fabs(actual_state.angles.yaw - desired_state.angles.yaw) > 20){
    //     printf("Yaw error too large!\n");
    //     return false;
    // }
    
    actual_state.velocity = ((actual_state.leftVelocity) + actual_state.rightVelocity) / 2.0f;

    if (fabs(actual_state.velocity - desired_state.velocity) > 5){
        printf("velocity error too large!\n");
        return false;
    }

    double velocity_output = velocity_pid.getOutput(actual_state.velocity, desired_state.velocity);
    std::cout << "velocity output is: " <<  velocity_output << " actual val: " <<  actual_state.velocity << " desired vel: " <<  desired_state.velocity << std::endl;

    double pitch_output    = pitch_pid.getOutput(actual_state.angles.pitch, (desired_state.angles.pitch - velocity_output));
    double yaw_rate_output = yaw_rate_pid.getOutput(actual_state.rates.gyro_x, desired_state.rates.gyro_x);

    if (!std::isnan(pitch_output) && !std::isnan(yaw_rate_output) && !std::isnan(velocity_output)) {
        outputLeft  = -1 * pitch_output + (yaw_rate_output *  1);
        outputRight = pitch_output + (yaw_rate_output * -1);
    } else {
        outputLeft  = 0;
        outputRight = 0;
    }
    return true;
}
 
void Controller::get_pitch_coeffs(double& P, double& I, double& D){
    std::unique_lock<std::mutex> lock(mtx);
    P = pitch_pid.getP();
    I = pitch_pid.getI();
    D = pitch_pid.getD();
}

void Controller::get_yaw_rate_coeffs(double& P, double& I, double& D){
    std::unique_lock<std::mutex> lock(mtx);
    P = yaw_rate_pid.getP();
    I = yaw_rate_pid.getI();
    D = yaw_rate_pid.getD();
}

void Controller::get_velocity_coeffs(double& P, double& I, double& D){
    std::unique_lock<std::mutex> lock(mtx);
    P = velocity_pid.getP();
    I = velocity_pid.getI();
    D = velocity_pid.getD();
}

void Controller::set_pitch_coeffs(const double* P, const double* I, const double* D)  {
    std::unique_lock<std::mutex> lock(mtx);
    pitch_pid.setPID(*P, *I, *D);
}

void Controller::set_yaw_rate_coeffs(const double* P, const double* I, const double* D)  {
    std::unique_lock<std::mutex> lock(mtx);
    yaw_rate_pid.setPID(*P, *I, *D);
}

void Controller::set_velocity_coeffs(const double* P, const double* I, const double* D) {
    std::unique_lock<std::mutex> lock(mtx);
    velocity_pid.setPID(*P, *I, *D);
}

