#include "DriveSystem.h"
#define PI 3.1415926535897932384626433832795

DriveSystem::DriveSystem(const int id[], const bool dir[], const int size, const std::string name, Log& logger) :
    ronThread(name, logger),
    numberOfNodes(size),
    nodeIDs(id),
    nodeReversed(dir),
    odriveCAN() {
    }

DriveSystem::~DriveSystem() {}

void DriveSystem::calcDeadRec(double imu_angle)
{
    static double inter_tire_distance = 0.240; // 240 mm
    static double tire_radius = 0.189/2.0; //80mm
    static double tire_cir = 2.0*PI*tire_radius;

    float currentWheelPos[2];
    getPosition(currentWheelPos[0], 0);
    getPosition(currentWheelPos[1], 1);

    double arcR = (tire_cir)*(currentWheelPos[0] - lastWheelPos[0]);
    double arcL = (tire_cir)*(currentWheelPos[1] - lastWheelPos[1]);

    double radius = 0.0;
    double angle = (arcR-arcL)/inter_tire_distance; 
    if (angle != 0.0f){
        radius = (arcL/angle) + (inter_tire_distance/2.0);
        
        double local_y = (radius - (cos(angle) * radius));
        double local_x = radius * sin(angle);

        double global_x = ((sin(deadRecPos[2]) * local_y) + (cos(deadRecPos[2]) * local_x));
        double global_y = ((cos(deadRecPos[2]) * local_y) + (sin(deadRecPos[2]) * local_x));

        deadRecPos[0] -= global_x;
        deadRecPos[1] += global_y;
        
        deadRecPosSinceStart[0] -= global_x;
        deadRecPosSinceStart[1] += global_y;

        // update heading angle
        deadRecPos[2]           = (imu_angle/180)*PI;
        deadRecPosSinceStart[2] = deadRecPos[2];

    } else {
        double global_x = (sin(deadRecPos[2]) * (arcR+arcL)/2.0);
        double global_y = (cos(deadRecPos[2]) * (arcR+arcL)/2.0);
        
        deadRecPos[0] -= global_x;
        deadRecPos[1] += global_y;
        deadRecPosSinceStart[0] -= global_x;    
        deadRecPosSinceStart[1] += global_y;
    }

    std::memcpy(lastWheelPos, currentWheelPos, sizeof(currentWheelPos));
}

void DriveSystem::getDRReletive(float& x, float& y){
    x = deadRecPos[0];
    y = deadRecPos[1];
}

void DriveSystem::getDRAbsolute(float& x, float& y){
    x = deadRecPosSinceStart[0];
    y = deadRecPosSinceStart[1];
}

bool DriveSystem::open() {
    // Open the can interface
    log("opening can");
    return odriveCAN.open();
}

bool DriveSystem::readEvent(struct can_frame& inMsg) {
    // Read a can event
    return odriveCAN.readMsg(inMsg) > 0;
}

void DriveSystem::close() {
    std::cout << "[DriveSystem] close()" << std::endl;
}

void DriveSystem::setTorque(float& t, const int axis_id){
    odriveCAN.SetTorque(axis_id, t*(isReversed(axis_id) ? -1 : 1));
}

void DriveSystem::requestVbusVoltage(){
    odriveCAN.GetVbusVoltage(0);
}

void DriveSystem::requestDRReset(){
    deadRecPos[0] = 0.0;
    deadRecPos[1] = 0.0;
}

void DriveSystem::getVelocity(float& vel, const int axis_id){
    DriveState cur_state;
    getState(cur_state, axis_id);
    vel = cur_state.velocity;
}

void DriveSystem::getPosition(float& pos, const int axis_id){
    DriveState cur_state;
    getState(cur_state, axis_id);
    pos = cur_state.position;
}

bool DriveSystem::getState(DriveState& data, int axis_id) {
    std::lock_guard<std::mutex> lock(thread_lock);
    // check if index if out of range
    int axis_index = findIndex(nodeIDs, axis_id, numberOfNodes);
    data = state[axis_index];
    data.vBusVoltage = vBusVoltage;
    return data.error;
}

bool DriveSystem::getStatus(){
    bool is_running = running.load(std::memory_order_relaxed);
    bool axis_error = false;
    DriveState cur_state;
    for (int index = 0; index < numberOfNodes; index++)
    {
        axis_error =+ getState(cur_state, nodeIDs[index]);
    }
    return (!is_running || axis_error > 0);
}

bool DriveSystem::isReversed(int axis_id)
{
    int axis_index = findIndex(nodeIDs, axis_id, numberOfNodes);
    return nodeReversed[axis_index];
}

void DriveSystem::updateState(const DriveState& data, int axis_id) {
    std::lock_guard<std::mutex> lock(thread_lock);
    int axis_index = findIndex(nodeIDs, axis_id, numberOfNodes);
    state[axis_index] = data;
}

void DriveSystem::runState(int axisState){
    for (int index = 0; index < numberOfNodes; index++)
    {
        odriveCAN.RunState(nodeIDs[index], axisState);
    }
}

bool DriveSystem::enable() {
    runState(ODriveCAN::AxisState_t::AXIS_STATE_CLOSED_LOOP_CONTROL);
    return 0;
}

bool DriveSystem::disable(){
    runState(ODriveCAN::AxisState_t::AXIS_STATE_IDLE);
    return 0;
}

bool DriveSystem::ESTOP(){
    for (int index = 0; index < numberOfNodes; index++)
    {
        odriveCAN.Estop(nodeIDs[index]);
    }
    return 0;
}

bool DriveSystem::reset(){
    for (int index = 0; index < numberOfNodes; index++)
    {
        odriveCAN.ClearErrors(nodeIDs[index]);
    }
    return 0;
}

int DriveSystem::findIndex(const int arr[], int target, int size) {
    for (int i = 0; i < size; i++) {
        if (arr[i] == target) {
            return i;
        }
    }
    return -1; // target not found
}

void DriveSystem::loop() {
    // Open the odrive device
    if (open()) {
        log("Error opening odrive device");
        running.store(false, std::memory_order_relaxed);
    }

    // Read the odrive events
    can_frame frame;
    DriveState cur_state;
    int dir;

    while (running.load(std::memory_order_relaxed)) { 
        if (readEvent(frame)) {
            uint32_t axis_id = frame.can_id >> 5;
            uint8_t cmd_id = frame.can_id & 0x01F;
            getState(cur_state, axis_id);
            dir = isReversed(axis_id) ? -1 : 1;
            switch(cmd_id) {
                case (ODriveCAN::CMD_ID_ODRIVE_HEARTBEAT_MESSAGE): {
                    HeartbeatMsg_t returnVals;
                    odriveCAN.Heartbeat(returnVals, frame);
                    cur_state.state = returnVals.currentState;
                    cur_state.error = returnVals.axisError > 0;
                    break;
                    }
                case (ODriveCAN::CMD_ID_GET_ENCODER_ESTIMATES): {
                    EncoderEstimatesMsg_t returnVals;
                    odriveCAN.GetPositionVelocityResponse(returnVals, frame);
                    cur_state.velocity = returnVals.velEstimate * dir;
                    cur_state.position = returnVals.posEstimate * dir;
                    // cur_state.timestamp = message_time;
                    break;
                    }
                case (ODriveCAN::CMD_ID_GET_IQ): {
                    //odriveCAN.GetIqResponse(iqVals, inMsg);
                    break;
                    }
                case (ODriveCAN::CMD_ID_GET_ADC_VOLTAGE): {
                    //float adcVoltage = odriveCAN.GetADCVoltageResponse(inMsg);
                    break;
                    }
                case (ODriveCAN::CMD_ID_GET_VBUS_VOLTAGE): {
                    vBusVoltage = odriveCAN.GetVbusVoltageResponse(frame);
                    break;
                    }
                default: {
                    break;
                    }
            }
            updateState(cur_state, axis_id);
        }
        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // Close the odrive device
    close();
}

