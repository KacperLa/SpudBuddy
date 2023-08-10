#include "IMUReader.h"
#include <sys/resource.h>

IMUReader::IMUReader(const std::string& new_bus, const std::string name, Log& logger) :
  ronThread(name, logger)
{
  this->bus = new_bus;
  this->running = true;
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
    std::cerr << "Error getting IMU angles." << std::endl;
  }
  return ret < 0;
}

bool IMUReader::readRates(rates_t& rates) {
  // Read a rates from BNO055
  int ret = bno055.get_gyro_data(&rates);
  if (ret < 0){
    std::cerr << "Error getting IMU gyreo rates." << std::endl;
  }
  return ret < 0;
}

bool IMUReader::getState(IMUState& data) {
  std::lock_guard<std::mutex> lock(imu_state_lock);
  data = imu_state;
  return data.error;
}

void IMUReader::updateState(IMUState data) {
  std::lock_guard<std::mutex> lock(imu_state_lock);
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

void IMUReader::loop() {
  setpriority(PRIO_PROCESS, getpid(), 1);

  IMUState data;
  // Open the BNO055 device
  if (open() != 0) {
    std::cerr << "Error opening IMU." << std::endl;
    data.error = IMU_ERROR::SENSOR_INIT_FAILURE;
    return;
  }

  // Read the IMU data
  angles_t angles;
  rates_t rates;
  IMUState last_state;

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
      // rates.gyro_y = rates.gyro_y * -1.0;

      getState(last_state);
      //log("old pitch: " + std::to_string(last_state.angles.pitch) + " new pitch: " + std::to_string(angles.pitch));
     
      if ((std::chrono::high_resolution_clock::now() - last_state.timestamp) > forget_time)
      {
        data = {angles, rates, std::chrono::high_resolution_clock::now(), 1, 0};
        updateState(data);
        std::cout << "[IMU] Updating the IMU to replace old data." << std::endl;    
      } else {
        if (fabs(angles.pitch - last_state.angles.pitch) > 100)
        {
          std::cout << "[IMU] pitch was: "<< angles.pitch << std::endl;    
          angles.pitch = last_state.angles.pitch;
        }
        if (fabs(angles.yaw - last_state.angles.yaw) > 100)
        {
          std::cout << "[IMU] yaw was: "<< angles.yaw << std::endl;    
          angles.yaw = last_state.angles.yaw;
        }
        if (fabs(rates.gyro_yaw) > 1000)
        {
          std::cout << "[IMU] gyro_yaw was: "<< rates.gyro_yaw << std::endl;    
          rates.gyro_yaw = 0;
        }
        if (fabs(rates.gyro_pitch) > 1000)
        {
          std::cout << "[IMU] gyro_pitch was: "<< rates.gyro_pitch << std::endl;    
          rates.gyro_pitch = 0;
        }
           
        data = {angles, rates, std::chrono::high_resolution_clock::now(), 1, 0};
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

