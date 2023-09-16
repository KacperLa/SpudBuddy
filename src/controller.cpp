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
 

    // if (fabs(actual_state.angles.yaw - desired_state.angles.yaw) > 20){
    //     printf("Yaw error too large!\n");
    //     return false;
    // }
    

    float yaw_rate_error = (actual_state.leftVelocity - actual_state.rightVelocity) + desired_state.rates.gyro_yaw;
    //calcTheta(desired_state.angles.yaw, actual_state.angles.yaw, yaw_error);
    double yaw_output = yaw_rate_pid.getOutput(yaw_rate_error); //(yaw_error * yaw_rate_pid.getP()) + (actual_state.rates.gyro_yaw * yaw_rate_pid.getD());
    
    if (fabs(yaw_output) < 0.001f){
        yaw_output = 0;
    }
    

    double velocity_output = velocity_pid.getOutput(actual_state.velocity, (-1*desired_state.velocity));
    // double velocity_l_output = velocity_pid_l.getOutput(actual_state.leftVelocity,  (desired_state.velocity + yaw_output));
    // double velocity_r_output = velocity_pid_r.getOutput(actual_state.rightVelocity, (desired_state.velocity - yaw_output));

    //std::cout << "output L: " <<  velocity_l_output << "output r: " <<  velocity_r_output << "actual l: " << actual_state.leftVelocity << "actual r: " << actual_state.rightVelocity << " desired vel: " <<  desired_state.velocity << std::endl;
    double pitch_error = desired_state.angles.pitch - (actual_state.angles.pitch + velocity_output);
    double pitch_output   =  (pitch_error * pitch_pid.getP()) + (actual_state.rates.gyro_pitch * pitch_pid.getD());

    if (fabs(pitch_output) < 0.001f){
        pitch_output = 0;
    }
    //std::cout << "output P: " <<  pitch_output << "actual P: " << actual_state.angles.pitch << " desired P: " <<  desired_state.angles.pitch << std::endl;

    //double yaw_rate_output = yaw_rate_pid.getOutput(actual_state.rates.gyro_x, desired_state.rates.gyro_x);
    //std::cout << "Gyro output is: " <<  yaw_rate_output << " actual x: " <<  actual_state.rates.gyro_x << " desired x: " <<  desired_state.rates.gyro_x << std::endl;

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

void Controller::get_sync_coeffs(double& P, double& I, double& D){
    std::unique_lock<std::mutex> lock(mtx);
    P = sync_pid.getP();
    I = sync_pid.getI();
    D = sync_pid.getD();
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

void Controller::set_sync_coeffs(const double* P, const double* I, const double* D) {
    std::unique_lock<std::mutex> lock(mtx);
    sync_pid.setPID(*P, *I, *D);
}
