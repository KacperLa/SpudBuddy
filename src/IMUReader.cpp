#include "IMUReader.h"
#include <sys/resource.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include "common.h"

IMUReader::IMUReader(const std::string& new_bus, const std::string name, Log* logger) :
  ronThread(name, logger, true)
{
  this->bus = new_bus;
}

IMUReader::~IMUReader()
{
}

bool IMUReader::open() {
  return bno055.imu_init("/dev/i2c-1", BNO055_I2C_ADDRESS, imu);
}

bool IMUReader::readAngles(angles_t& angles) {
  // Read a euler angles from BNO055
  int ret = bno055.get_euler_angles(&angles);
  if (ret < 0){
    log("Error getting IMU angles.");
  }
  return ret < 0;
}

bool IMUReader::readRates(rates_t& rates) {
  // Read a rates from BNO055
  int ret = bno055.get_gyro_data(&rates);
  if (ret < 0){
    log("Error getting IMU gyreo rates.");
  }
  return ret < 0;
}

bool IMUReader::getState(imuData_t& data) {
  std::lock_guard<std::mutex> lock(thread_lock);
  data = imu_state;
  return data.error;
}

void IMUReader::logCalStatus(){
  imu_cal_info_t cal;
  imu_sys_info_t info;
  int ret = bno055.get_cal_status(&cal);
  ret = bno055.get_info(&info);

  bno055.print_mode(info.opr_mode);

  log("IMU op mode: , IMU cal: " + std::to_string(cal.scal_st) + \
      ", gyro cal: " + std::to_string(cal.gcal_st) + \ 
      ", acel cal: " + std::to_string(cal.acal_st) + \ 
      ", mag cal: " + std::to_string(cal.mcal_st));
   
}

void IMUReader::updateState(imuData_t data) {
  std::lock_guard<std::mutex> lock(thread_lock);
  imu_state = data;
}

void IMUReader::stop() {
  running = false;
}

void transformReading(angles_t & reading)
{
  double tmp = reading.pitch;
  reading.pitch = reading.roll * -1.0;
  reading.roll = tmp;
}

double mod(double a, double n)
{
    return (a - floor(a/n) * n);
}

double calcTheta(double cmd, double actual)
{
    double b = mod(((actual - cmd) + (2.0 * 180.0)), (2.0 * 180));
    double f = mod(((cmd - actual) + (2.0 * 180.0)), (2.0 * 180));

    if (b < 180.0) {
        return (-1.0 * b);
    }
    else {
        return f;
    }
}

void IMUReader::loop() {
  setpriority(PRIO_PROCESS, getpid(), 1);

  imuData_t data;
  // Open the BNO055 device
  if (open() != 0) {
    std::cerr << "Error opening IMU." << std::endl;
    data.error = IMU_ERROR::SENSOR_INIT_FAILURE;
    return;
  }

  // set bno 055 to config
  bno055.set_mode(operation_mode_t::config);
  // set fmu on 

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  bno055.set_mode(operation_mode_t::ndof);


  // Read the IMU data
  angles_t angles;
  rates_t rates;
  imuData_t last_state;

  while (running) {
    int imu_status = bno055.get_sstat();
    if (imu_status > 0){
      //printf("IMU STATUS: 0x%02X\n", imu_status);
      //bno055.print_sstat(imu_status);
      if (imu_status == 0x01){
        bno055.print_serror(bno055.get_serror());
      } else if (imu_status == 0x06){
        std::cout << "[IMU] No fusion alogithm ." << std::endl;        
      }
    }
    
    if (!readAngles(angles) && !readRates(rates))
    {
      transformReading(angles);
      rates.gyro_pitch = rates.gyro_pitch * -1;

      getState(last_state);
     
      if ((get_time_nano() - last_state.timestamp) > forget_time)
      {
        data = {angles, rates, get_time_nano(), 1, 0};
        updateState(data);
        std::cout << "[IMU] Updating the IMU to replace old data." << std::endl;    
      } else {
        if (fabs(angles.pitch - last_state.angles.pitch) > 100)
        {
          log("[IMU] pitch was: " + std::to_string(angles.pitch));
          angles.pitch = last_state.angles.pitch;
        }

        // if (calcTheta(angles.yaw, last_state.angles.yaw) > 50)
        // {
        //   log("[IMU] yaw was: " + std::to_string(angles.yaw));
        //   angles.yaw = last_state.angles.yaw;
        // }
        if (fabs(rates.gyro_yaw) > 1000)
        {
          log("[IMU] gyro_yaw was: " + std::to_string(rates.gyro_yaw));
          rates.gyro_yaw = 0;
        }
        if (fabs(rates.gyro_pitch) > 1000)
        {
          log("[IMU] gyro_pitch was: " + std::to_string(rates.gyro_pitch));
          rates.gyro_pitch = 0;
        }
           
        data = {angles, rates, get_time_micro(), 1, 0};
        updateState(data);
        // } else {
        //   //std::cout << "[IMU] Difference between consecative pitch reading was greater than 10 degrees, ignorring reading." << std::endl;    
        // }
      }
    } 
    else
    {
      std::cerr << "Error getting IMU data." << std::endl;
      data.error = true;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  // Close the BNO055 device
  // This is already done by the destructor of the BNO055 object
}

