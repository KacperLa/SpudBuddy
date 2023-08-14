#ifndef IMUREADER_H
#define IMUREADER_H

#include <chrono>
#include <string>
#include <mutex>
#include <iostream>
#include <atomic>
#include <thread>

#include <fcntl.h>
#include <unistd.h>
#include <cmath>

#include "drivers/bno055/BNO055.h"
#include<ronThread.h>

typedef BNO055::euler_angles angles_t;
typedef BNO055::gyro rates_t;
typedef BNO055::bno_cal imu_cal_info_t;
typedef BNO055::bno_info imu_sys_info_t;


// FAILURE MODES
typedef enum {
  NO_ERROR = 0x00,
  QUALITY_LOW,
  NO_RESPONSE,
  READ_TIME_OUT,
  SENSOR_INIT_FAILURE,
  SENSOR_OVER_TEMP,
  SENSOR_SELF_TEST_FAIL,
  SENSOR_SYSTEM_ERROR
} IMU_ERROR;

struct IMUState {
  angles_t angles {0.0, 0.0, 0.0};
  rates_t rates   {0.0, 0.0, 0.0};
  std::chrono::high_resolution_clock::time_point timestamp;
  bool quality {false};
  bool error {false};
};

class IMUReader : public ronThread
{
public:
  IMUReader(const std::string& bus, const std::string name, Log& logger);
  virtual ~IMUReader();

  bool getState(IMUState& data);
  void logCalStatus();

private:
  virtual void loop() override;

protected:
  void stop();
  void updateState(IMUState data);

  bool open();
  bool readAngles(BNO055::euler_angles& angles);
  bool readRates(rates_t& rates);

  std::string device;

  BNO055 bno055{};
  std::string bus{"0"};

  std::chrono::duration<double> time_limit{1.0 / 10.0}; // 20 Hz
  std::chrono::duration<double> forget_time{1.0 / 1.0}; // 1 Hz

  IMUState imu_state;

};

#endif
