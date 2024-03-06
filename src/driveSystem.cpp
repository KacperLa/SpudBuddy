#include "driveSystem.h"
#define PI 3.1415926535897932384626433832795

driveSystem::driveSystem(const int id[], const bool dir[], const int size, const std::string name, Log* logger) :
    ronThread(name, logger, false),
    shared_imu_state(shared_imu_file, false),
    numberOfNodes(size),
    shared_state(shared_drive_system_file, true),
    nodeIDs(id),
    nodeReversed(dir),
    odriveCAN() {
    }

driveSystem::~driveSystem() {}

void driveSystem::calcDeadRec()
{
    shared_imu_state.getData(&imu_state);

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

        double global_x = ((sin(deadRecAngle) * local_y) + (cos(deadRecAngle) * local_x));
        double global_y = ((cos(deadRecAngle) * local_y) + (sin(deadRecAngle) * local_x));

        deadRecPos.x -= global_x;
        deadRecPos.y += global_y;
        
        deadRecPosSinceStart.x -= global_x;
        deadRecPosSinceStart.y += global_y;

        // update heading angle
        deadRecAngle = (imu_state.angles.yaw/180)*PI;

    } else {
        double global_x = (sin(deadRecAngle) * (arcR+arcL)/2.0);
        double global_y = (cos(deadRecAngle) * (arcR+arcL)/2.0);
        
        deadRecPos.x -= global_x;
        deadRecPos.y += global_y;
        deadRecPosSinceStart.x -= global_x;    
        deadRecPosSinceStart.y += global_y;
    }

    std::memcpy(lastWheelPos, currentWheelPos, sizeof(currentWheelPos));
}

void driveSystem::getDRReletive(position_t& pos)
{
    pos = deadRecPos;
}

void driveSystem::getDRAbsolute(position_t& pos)
{
    pos = deadRecPosSinceStart;
}

bool driveSystem::open() {
    // Open the can interface
    log("opening can");
    return odriveCAN.open();
}

bool driveSystem::readEvent(struct can_frame& inMsg) {
    // Read a can event
    return odriveCAN.readMsg(inMsg) > 0;
}

void driveSystem::close() {
    std::cout << "[driveSystem] close()" << std::endl;
}

void driveSystem::setTorque(float& t, const int axis_id){
    odriveCAN.SetTorque(axis_id, t*(isReversed(axis_id) ? -1 : 1));
}

void driveSystem::setPosition(float& pos, const int axis_id){
    // check if pos is within limits
    if (pos > max_position[axis_id] || pos < min_position[axis_id]){
        log("Position out of range" + std::to_string(pos) + " for axis: " + std::to_string(axis_id));
        return;
    }
    odriveCAN.SetPosition(axis_id, pos*(isReversed(axis_id) ? -1 : 1));
}

void driveSystem::requestVbusVoltage(){
    odriveCAN.GetVbusVoltage(0);
}

void driveSystem::requestDRReset()
{
    deadRecPos = {0.0, 0.0, 0.0};
}

void driveSystem::getVelocity(float& vel, const int axis_id){
    DriveState cur_state;
    getState(cur_state, axis_id);
    vel = cur_state.velocity;
}

void driveSystem::getPosition(float& pos, const int axis_id){
    DriveState cur_state;
    getState(cur_state, axis_id);
    pos = cur_state.position;
}

bool driveSystem::getState(DriveState& data, int axis_id) {
    std::lock_guard<std::mutex> lock(thread_lock);
    // check if index if out of range
    int axis_index = findIndex(nodeIDs, axis_id, numberOfNodes);
    data = state[axis_index];
    data.vBusVoltage = vBusVoltage;
    return data.error;
}

bool driveSystem::getStatus(){
    bool is_running = running.load(std::memory_order_relaxed);
    bool axis_error = false;
    DriveState cur_state;
    for (int index = 0; index < numberOfNodes; index++)
    {
        axis_error =+ getState(cur_state, nodeIDs[index]);
    }
    return (!is_running || axis_error > 0);
}

bool driveSystem::isReversed(int axis_id)
{
    int axis_index = findIndex(nodeIDs, axis_id, numberOfNodes);
    return nodeReversed[axis_index];
}

void driveSystem::updateState(const DriveState& data, int axis_id) {
    int axis_index = findIndex(nodeIDs, axis_id, numberOfNodes);
    driveSystemState_t s_state;
    shared_state.getData(&s_state);
    s_state.axis[axis_index] = data;
    s_state.position = deadRecPos;
    shared_state.setData(&s_state);
}

void driveSystem::runState(int axisState){
    for (int index = 0; index < numberOfNodes; index++)
    {
        odriveCAN.RunState(nodeIDs[index], axisState);
    }
}

bool driveSystem::enable() {
    runState(ODriveCAN::AxisState_t::AXIS_STATE_CLOSED_LOOP_CONTROL);
    return 0;
}

bool driveSystem::disable(){
    runState(ODriveCAN::AxisState_t::AXIS_STATE_IDLE);
    return 0;
}

bool driveSystem::ESTOP(){
    for (int index = 0; index < numberOfNodes; index++)
    {
        odriveCAN.Estop(nodeIDs[index]);
    }
    return 0;
}

bool driveSystem::reset(){
    for (int index = 0; index < numberOfNodes; index++)
    {
        odriveCAN.ClearErrors(nodeIDs[index]);
    }
    return 0;
}

int driveSystem::findIndex(const int arr[], int target, int size) {
    for (int i = 0; i < size; i++) {
        if (arr[i] == target) {
            return i;
        }
    }
    return -1; // target not found
}

void driveSystem::loop() {
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
                    // log("Axis: " + std::to_string(axis_id) + " State: " + std::to_string(cur_state.state) + " Error: " + std::to_string(cur_state.error));
                    break;
                    }
                case (ODriveCAN::CMD_ID_GET_ENCODER_ESTIMATES): {
                    EncoderEstimatesMsg_t returnVals;
                    odriveCAN.GetPositionVelocityResponse(returnVals, frame);
                    cur_state.velocity = returnVals.velEstimate * dir;
                    cur_state.position = returnVals.posEstimate * dir;
                    // cur_state.timestamp = message_time;
                    // log("Axis: " + std::to_string(axis_id) + " Position: " + std::to_string(cur_state.position) + " Velocity: " + std::to_string(cur_state.velocity));
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

