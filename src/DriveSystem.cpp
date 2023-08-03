#include "DriveSystem.h"

DriveSystem::DriveSystem(const int id[], const int size) : 
    numberOfNodes(size),
    nodeIDs(id),
    odriveCAN() {
        startReading();
    }

DriveSystem::~DriveSystem() {}

bool DriveSystem::open() {
    // Open the can interface
    std::cout << "opening can" << std::endl;
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
    odriveCAN.SetVelocity(axis_id, t);
}

void DriveSystem::getVelocity(float& vel, const int axis_id){
    DriveState cur_state;
    getState(cur_state, axis_id);
    vel = cur_state.velocity;
}


bool DriveSystem::getState(DriveState& data, int axis_id) {
    std::lock_guard<std::mutex> lock(state_lock_);
    // check if index if out of range
    int axis_index = findIndex(nodeIDs, axis_id, numberOfNodes);
    data = state[axis_index];
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

void DriveSystem::updateState(const DriveState& data, int axis_id) {
    std::lock_guard<std::mutex> lock(state_lock_);
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

int DriveSystem::findIndex(const int arr[], int size, int target) {
    for (int i = 0; i < size; i++) {
        if (arr[i] == target) {
            return i;
        }
    }
    return -1; // target not found
}

void DriveSystem::readEventLoop() {
    // Open the odrive device
    if (open()) {
        std::cerr << "Error opening odrive device" << std::endl;
        running.store(false, std::memory_order_relaxed);
    }

    // Read the odrive events
    can_frame frame;
    DriveState cur_state;
    while (running.load(std::memory_order_relaxed)) { 
        if (readEvent(frame)) {
            uint32_t axis_id = frame.can_id >> 5;
            uint8_t cmd_id = frame.can_id & 0x01F;
            getState(cur_state, axis_id);
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
                    cur_state.velocity = returnVals.velEstimate;
                    cur_state.position = returnVals.posEstimate;
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
                    float vbusVoltage = odriveCAN.GetVbusVoltageResponse(frame);
                    vBusVoltage = vbusVoltage;
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

void DriveSystem::startReading() {
    // Create a new thread to read the odrive events
    read_thread = std::thread([this](){ this->readEventLoop(); });

    // Set the running flag to true
    running.store(true, std::memory_order_relaxed);
}

void DriveSystem::stopReading() {
    // Set the running flag to false
    running.store(false, std::memory_order_relaxed);
    std::cout << "[DriveSystem] joining thread..." << std::endl;

    // Wait for the thread to finish
    if (read_thread.joinable()) {
        std::cout << "[DriveSystem] thread is still running." << std::endl;
        read_thread.join();
    } else {
        std::cout << "[DriveSystem] thread is already dead." << std::endl;
    }

    std::cout << "[DriveSystem] thread has been joined." << std::endl;
}
