#include "controller.h"
#include <iostream>


Controller::Controller() {
    pitch_pid.setOutputLimits(-10.0, 10.0);
    yaw_rate_pid.setOutputLimits(-10.0, 10.0);
    velocity_pid_l.setOutputLimits(-10.0, 10.0);
    velocity_pid_l.setMaxIOutput(10.0);
    velocity_pid_r.setOutputLimits(-10.0, 10.0);
    velocity_pid_r.setMaxIOutput(10.0);
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
    cmd = 60.0f;
    float b = mod(((actual - cmd) + (2.0 * 180)), (2.0 * 180));
    float f = mod(((cmd - actual) + (2.0 * 180)), (2.0 * 180));

    if (b < 180) {
        error =  (-1 * b);
    }
    else {
        error =  f;
    }
}

bool Controller::calculateOutput(RobotState actual_state, RobotState desired_state, float& outputLeft, float& outputRight)
{
    if (fabs(actual_state.angles.pitch - desired_state.angles.pitch) > 25){
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

   
    
    float yaw_error;
    calcTheta(desired_state.angles.yaw, actual_state.angles.yaw, yaw_error);
    double yaw_output      = yaw_rate_pid.getOutput(yaw_error);

    double velocity_output_left  = velocity_pid_l.getOutput(actual_state.leftVelocity,  (desired_state.velocity + yaw_output));
    double velocity_output_right = velocity_pid_r.getOutput(actual_state.rightVelocity, (desired_state.velocity - yaw_output));

    //std::cout << "output L: " <<  velocity_output_left << "output r: " <<  velocity_output_right << "actual l: " << actual_state.leftVelocity << "actual r: " << actual_state.rightVelocity << " desired vel: " <<  desired_state.velocity << std::endl;

    double pitch_output    = pitch_pid.getOutput(actual_state.angles.pitch, (desired_state.angles.pitch));

    std::cout << "output P: " <<  pitch_output << "actual P: " << actual_state.angles.pitch << " desired P: " <<  desired_state.angles.pitch << std::endl;

    //double yaw_rate_output = yaw_rate_pid.getOutput(actual_state.rates.gyro_x, desired_state.rates.gyro_x);
    //std::cout << "Gyro output is: " <<  yaw_rate_output << " actual x: " <<  actual_state.rates.gyro_x << " desired x: " <<  desired_state.rates.gyro_x << std::endl;

    if (!std::isnan(pitch_output) && !std::isnan(yaw_output) && !std::isnan(velocity_output_left)) {
        outputLeft  = -1 * (pitch_output);
        outputRight =       pitch_output;
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
    P = velocity_pid_l.getP();
    I = velocity_pid_l.getI();
    D = velocity_pid_l.getD();
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
    velocity_pid_l.setPID(*P, *I, *D);
    velocity_pid_r.setPID(*P, *I, *D);
}

