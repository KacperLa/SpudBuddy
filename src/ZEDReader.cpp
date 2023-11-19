#include "ZEDReader.h"
#include <sys/resource.h>
#include <cmath>
#include <iostream>
#include <sstream>

ZEDReader::ZEDReader(const std::string& areaFile, const std::string name, Log& logger) :
  ronThread(name, logger)
{
    m_areaFile = areaFile;
}

ZEDReader::~ZEDReader()
{
}

bool ZEDReader::open() {
    // Set configuration parameters for the ZED
    InitParameters init_parameters;
    init_parameters.coordinate_units = UNIT::METER;
    init_parameters.coordinate_system = COORDINATE_SYSTEM::RIGHT_HANDED_Y_UP;
    init_parameters.sdk_verbose = true;
    init_parameters.camera_fps = 15;
    init_parameters.camera_resolution = RESOLUTION::VGA;
    
    PositionalTrackingParameters positional_tracking_param;  
    positional_tracking_param.enable_imu_fusion = true;
    // positional_tracking_param.enable_area_memory = true;
    // Open the camera
    auto returned_state = zed.open(init_parameters);
    if (returned_state != ERROR_CODE::SUCCESS)
    {
        // log("Camera Open " + std::to_string(returned_state) + " exit thread.");
        return false;
    }

    returned_state = zed.enablePositionalTracking(positional_tracking_param);
    if (returned_state != ERROR_CODE::SUCCESS)
    {
        log("Enabling positionnal tracking failed");
        zed.close();
        return false;
    }
    return true;
}

bool ZEDReader::getState(IMUState& data) {
  std::lock_guard<std::mutex> lock(thread_lock);
  data = imu_state;
  return data.error;
}

bool ZEDReader::getState(slamState_t& data) {
  std::lock_guard<std::mutex> lock(thread_lock);
  data = slam_state;
  return data.tracking_state;
}

void ZEDReader::updateState(IMUState data) {
  std::lock_guard<std::mutex> lock(thread_lock);
  imu_state = data;
}

void ZEDReader::updateState(slamState_t data) {
  std::lock_guard<std::mutex> lock(thread_lock);
  slam_state = data;
}

void ZEDReader::stop() {
  running = false;
}

void ZEDReader::loop() {
    setpriority(PRIO_PROCESS, getpid(), 1);

    IMUState data;
    slamState_t slam_data;
    SensorsData sensors_data;
    Pose camera_path;
    POSITIONAL_TRACKING_STATE tracking_state;

    // set error to true untill camera is open
    data.error = true;
    updateState(data);

    // Open the ZED device
    if (open() != 1) {
        std::cerr << "Error opening ZED." << std::endl;
        data.error = true;
        return;
    }

    while (running) {
        if (zed.grab() == ERROR_CODE::SUCCESS)
        {
            // Get the position of the camera in a fixed reference frame (the World Frame)
            tracking_state = zed.getPosition(camera_path, REFERENCE_FRAME::WORLD);

            if (zed.getSensorsData(sensors_data, TIME_REFERENCE::CURRENT ) == sl::ERROR_CODE::SUCCESS)
            {   
                auto zedAngles = sensors_data.imu.pose.getEulerAngles(false);
                auto zedRates = sensors_data.imu.angular_velocity;
                angles_t angles = {zedAngles[3], -1*zedAngles[0], zedAngles[1]};
                rates_t rates = {zedRates[3], -1*zedRates[0], zedRates[1]};
                data = {angles,
                        rates,
                        std::chrono::high_resolution_clock::now(), 1, 0};
            }
            updateState(data);

            if (tracking_state == POSITIONAL_TRACKING_STATE::OK)
            {
                slam_data = {camera_path.getTranslation().tz*-1.0f, 
                            camera_path.getTranslation().tx, 
                            camera_path.getTranslation().ty, 
                            true};
            } else {
                slam_data.tracking_state = false;
            }
            // log("ZEDReader: " + std::to_string(slam_data.x) + " " + std::to_string(slam_data.y) + " " + std::to_string(slam_data.z) + " " + std::to_string(slam_data.tracking_state));
            updateState(slam_data);
        }  
        else
        {
            // watch dog set error is no new readingin a while
            // std::cerr << "Error getting ZED data." << std::endl;
            data.error = true;
            std::cout << "Error getting ZED data." << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    zed.disablePositionalTracking();
    zed.close();
}

