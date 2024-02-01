#ifndef IMUREADER_H
#define IMUREADER_H

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

#include <shared_structs.h>

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

class IMUReader : public ronThread
{
public:
  IMUReader(const std::string& bus, const std::string name, Log* logger);
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

  std::int64_t time_limit  {1000000 / 10}; // 10 Hz
  std::int64_t forget_time {1000000 / 1};  // 1 Hz

  IMUState imu_state;

};

#endif
