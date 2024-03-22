#include "ZEDReader.h"
#include <sys/resource.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include "common.h"

ZEDReader::ZEDReader(const std::string& areaFile, const std::string name, Log* logger) :
    ronThread(name, logger, false),
    shared_tracking_state(shared_tracking_state_file,  true),
    shared_camera_feed(shared_camera_feed_file, true)
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
    init_parameters.coordinate_system = COORDINATE_SYSTEM::RIGHT_HANDED_Z_UP;
    init_parameters.sdk_verbose = true;
    init_parameters.camera_fps = 15;
    init_parameters.camera_resolution = RESOLUTION::HD720;
    init_parameters.depth_mode = DEPTH_MODE::ULTRA;
    
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

    // SpatialMappingParameters mapping_parameters;
    // mapping_parameters.save_texture = true;  // Scene texture will be recorded
    // returned_state = zed.enableSpatialMapping(mapping_parameters);
    // if (returned_state != ERROR_CODE::SUCCESS)
    // {
    //     log("Enabling spatial mapping failed");
    //     zed.close();
    //     return false;
    // }

    return true;
}

void ZEDReader::stop() {
  running = false;
}

void ZEDReader::getIMUData(imuData_t *data)
{
    static SensorsData sensors_data;
    static sl::float3 angles;

    data->error = zed.getSensorsData(sensors_data, TIME_REFERENCE::CURRENT ) != sl::ERROR_CODE::SUCCESS;
    angles = sensors_data.imu.pose.getEulerAngles(true);
    data->angles.yaw       = angles[0];
    data->angles.pitch     = angles[1];
    data->angles.roll      = angles[2];
    data->rates.gyro_yaw   = sensors_data.imu.angular_velocity[0];
    data->rates.gyro_pitch = sensors_data.imu.angular_velocity[1];
    data->rates.gyro_roll  = sensors_data.imu.angular_velocity[2];
    data->timestamp        = sensors_data.imu.timestamp.getNanoseconds();
}

void ZEDReader::saveMesh()
{
    Mesh mesh; 
    zed.extractWholeSpatialMap(mesh);
    mesh.applyTexture();
    mesh.filter(MeshFilterParameters::MESH_FILTER::LOW); // Filter the mesh (remove unnecessary vertices and faces)
    mesh.save("server/static/mesh.obj");
}

void ZEDReader::loop() {
    SensorsData sensors_data;
    Pose camera_path;
    POSITIONAL_TRACKING_STATE tracking_state;

    // set error to true untill camera is open
    positionSystem_t tracking_data;
    tracking_data.status = false;
    shared_tracking_state.setData(&tracking_data);

    sl::Translation camera_pose;  

	const auto& resolution = zed.getCameraInformation().camera_configuration.resolution;
    // create a sl::Mat with a pointer to the shared memory
        
    sl::Mat point_cloud;
    
    // set resolution in each sdata buffer
    for (int i = 0; i < 3; i++)
    {
        shared_camera_feed.getBuffer()->rows = 720;
        shared_camera_feed.getBuffer()->cols = 1280;
        shared_camera_feed.getBuffer()->channels = 4;
        shared_camera_feed.trigger();
    }

    // Open the ZED device
    if (open() != 1) {
        std::cerr << "Error opening ZED." << std::endl;
        return;
    }

    while (running) {
        auto last_run = get_time_nano();

        if (zed.grab() == ERROR_CODE::SUCCESS)
        {
            sl::Mat image(1280,
                          720,
                          MAT_TYPE::U8_C4, 
                          shared_camera_feed.getBuffer()->frame,
                          5120,
                          sl::MEM::CPU);
            zed.retrieveImage(image, VIEW::LEFT, MEM::CPU);
            shared_camera_feed.trigger();

            // Get the position of the camera in a fixed reference frame (the World Frame)
            tracking_state = zed.getPosition(camera_path, REFERENCE_FRAME::WORLD);

            tracking_data.status = (tracking_state == POSITIONAL_TRACKING_STATE::OK);

            camera_pose = camera_path.getTranslation();
            tracking_data.position = {  camera_pose.tx, 
                                        camera_pose.ty, 
                                        camera_pose.tz, 
                                    };
            tracking_data.timestamp = get_time_nano();
        }  
        else
        {
            // watch dog set error is no new readingin a while
            // std::cerr << "Error getting ZED data." << std::endl;
            tracking_data.status = false;
            std::cout << "Error getting ZED data." << std::endl;
        }
        
        shared_tracking_state.setData(&tracking_data);

        // auto loop_dur = get_time_nano() - last_run;
        // if (loop_dur > loop_time)
        // {
        //     auto loop_dur_in_seconds = loop_dur / 1000000.0;

        //     log("Zed loop over time. actual: " + std::to_string(loop_dur_in_seconds) + " s should be: " + std::to_string(loop_time/1000000.0) + " s ");
        // } 
        // else
        // {
        //     std::this_thread::sleep_for(std::chrono::microseconds(loop_time-loop_dur));
        // }
    }
    
    zed.disablePositionalTracking();
    // zed.disableSpatialMapping();
    zed.close();
}

