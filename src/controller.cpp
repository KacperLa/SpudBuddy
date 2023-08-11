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

bool Controller::calculateOutput(RobotState actual_state, RobotState desired_state, float& outputLeft, float& outputRight)
{
 

    // if (fabs(actual_state.angles.yaw - desired_state.angles.yaw) > 20){
    //     printf("Yaw error too large!\n");
    //     return false;
    // }
    

    float yaw_error;
    double yaw_output = 0;
    calcTheta(desired_state.angles.yaw, actual_state.angles.yaw, yaw_error);
    std::cout << "actual: " << actual_state.angles.yaw << " , desired: " << desired_state.angles.yaw << ", error: " << yaw_error << std::endl;
    if (fabs(yaw_error) > 5.0)
    {
        yaw_output = (yaw_error * yaw_rate_pid.getP()) + (-1*actual_state.rates.gyro_yaw * yaw_rate_pid.getD());
        if (fabs(yaw_output) < 0.01f){
            yaw_output = 0;
        }
    }

    double velocity_output = velocity_pid.getOutput(actual_state.velocity, desired_state.velocity);

    //std::cout << "output L: " <<  velocity_output_left << "output r: " <<  velocity_output_right << "actual l: " << actual_state.leftVelocity << "actual r: " << actual_state.rightVelocity << " desired vel: " <<  desired_state.velocity << std::endl;
    double pitch_error = desired_state.angles.pitch - (actual_state.angles.pitch + velocity_output);
    double pitch_output   =  (pitch_error * pitch_pid.getP()) + (actual_state.rates.gyro_pitch * pitch_pid.getD());
    //pitch_pid.getOutput(actual_state.angles.pitch, (desired_state.angles.pitch));
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

