#include "ODriveCAN.h"

static const int kMotorOffsetFloat = 2;
static const int kMotorStrideFloat = 28;
static const int kMotorOffsetInt32 = 0;
static const int kMotorStrideInt32 = 4;
static const int kMotorOffsetBool = 0;
static const int kMotorStrideBool = 4;
static const int kMotorOffsetUint16 = 0;
static const int kMotorStrideUint16 = 2;

static const int NodeIDLength = 6;
static const int CommandIDLength = 5;

static const float feedforwardFactor = 1 / 0.001;

ODriveCAN::ODriveCAN() {}

int ODriveCAN::open() {
    sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);

    if (sock < 0) {
        std::cout << "Could not open CAN socket." << std::endl;
        return 1;
    }

    struct ifreq ifr;
    strcpy(ifr.ifr_name, CAN_INTERFACE_NAME.c_str());
    ioctl(sock, SIOCGIFINDEX, &ifr);

    struct sockaddr_can addr;
    addr.can_family = AF_CAN;
    addr.can_ifindex = if_nametoindex(CAN_INTERFACE_NAME.c_str());

    if (addr.can_ifindex == 0) {
        std::cout << "Interface can0 doesn't exists" << std::endl;
        return 1;
    }

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cout << "Could not bind CAN socket: " << std::endl;
        return 1;
    }

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    return 0;
}

void ODriveCAN::sendMessage(int axis_id, int cmd_id, bool remote_request, int length, uint8_t *signal_bytes) {
    struct can_frame frame;
    
    if (remote_request) {
        frame.can_id = (axis_id << CommandIDLength) + cmd_id + CAN_RTR_FLAG;
    } else {
        frame.can_id = (axis_id << CommandIDLength) + cmd_id;
    }
    frame.can_dlc = length;
    memcpy(frame.data, signal_bytes, length);

    int nbytes = write(sock, &frame, sizeof(struct can_frame));
}

int ODriveCAN::readMsg(struct can_frame& inMsg) {
    int ret = recv(sock, &inMsg, sizeof(struct can_frame), 0);

    if (ret == 0) {
        std::cout << "Connection closed" << std::endl;
    } else if (ret == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;
        } else {
            return -1;
        }
	} else {
        //printf("0x%03X [%d] ",inMsg.can_id, inMsg.can_dlc);
	 	return 1;
	}
}

void ODriveCAN::Heartbeat(HeartbeatMsg_t &returnVals, struct can_frame &inMsg) {
	returnVals.parseMessage(inMsg);
}

// void ODriveCAN::SetAxisNodeId(int axis_id, int node_id) {
// 	byte* node_id_b = (byte*) &node_id;
	
// 	sendMessage(axis_id, CMD_ID_SET_AXIS_NODE_ID, false, 4, node_id_b);
// }

// void ODriveCAN::SetControllerModes(int axis_id, int control_mode, int input_mode) {
// 	byte* control_mode_b = (std::byte*) &control_mode;
// 	byte* input_mode_b = (std::byte*) &input_mode;
// 	byte msg_data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	
// 	msg_data[0] = control_mode_b[0];
// 	msg_data[1] = control_mode_b[1];
// 	msg_data[2] = control_mode_b[2];
// 	msg_data[3] = control_mode_b[3];	
// 	msg_data[4] = input_mode_b[0];
// 	msg_data[5] = input_mode_b[1];
// 	msg_data[6] = input_mode_b[2];
// 	msg_data[7] = input_mode_b[3];
	
// 	sendMessage(axis_id, CMD_ID_SET_CONTROLLER_MODES, false, 8, msg_data);
// }

void ODriveCAN::SetPosition(int axis_id, float position) {
    SetPosition(axis_id, position, 0.0f, 0.0f);
}

void ODriveCAN::SetPosition(int axis_id, float position, float velocity_feedforward) {
    SetPosition(axis_id, position, velocity_feedforward, 0.0f);
}

void ODriveCAN::SetPosition(int axis_id, float position, float velocity_feedforward, float current_feedforward) {
    int16_t vel_ff = (int16_t) (feedforwardFactor * velocity_feedforward);
    int16_t curr_ff = (int16_t) (feedforwardFactor * current_feedforward);

    uint8_t* position_b = (uint8_t*) &position;
    uint8_t* velocity_feedforward_b = (uint8_t*) &vel_ff;
    uint8_t* current_feedforward_b = (uint8_t*) &curr_ff;
    uint8_t msg_data[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    msg_data[0] = position_b[0];
    msg_data[1] = position_b[1];
    msg_data[2] = position_b[2];
    msg_data[3] = position_b[3];
    msg_data[4] = velocity_feedforward_b[0];
    msg_data[5] = velocity_feedforward_b[1];
    msg_data[6] = current_feedforward_b[0];
    msg_data[7] = current_feedforward_b[1];

    sendMessage(axis_id, CMD_ID_SET_INPUT_POS, false, 8, msg_data);
}

void ODriveCAN::SetVelocity(int axis_id, float velocity) {
    SetVelocity(axis_id, velocity, 0.0f);
}

void ODriveCAN::SetVelocity(int axis_id, float velocity, float current_feedforward) {
    uint8_t* velocity_b = (uint8_t*) &velocity;
    uint8_t* current_feedforward_b = (uint8_t*) &current_feedforward;
    uint8_t msg_data[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    msg_data[0] = velocity_b[0];
    msg_data[1] = velocity_b[1];
    msg_data[2] = velocity_b[2];
    msg_data[3] = velocity_b[3];
    msg_data[4] = current_feedforward_b[0];
    msg_data[5] = current_feedforward_b[1];
    msg_data[6] = current_feedforward_b[2];
    msg_data[7] = current_feedforward_b[3];
    
    sendMessage(axis_id, CMD_ID_SET_INPUT_VEL, false, 8, msg_data);
}

void ODriveCAN::SetTorque(int axis_id, float torque) {
    uint8_t* torque_b = (uint8_t*) &torque;

    sendMessage(axis_id, CMD_ID_SET_INPUT_TORQUE, false, 4, torque_b);
}

// void ODriveCAN::SetLimits(int axis_id, float velocity_limit, float current_limit) {
//     byte* velocity_limit_b = (byte*) &velocity_limit;
// 	byte* current_limit_b = (byte*) &current_limit;
//     byte msg_data[8] = {0, 0, 0, 0, 0, 0, 0, 0};

//     msg_data[0] = velocity_limit_b[0];
//     msg_data[1] = velocity_limit_b[1];
//     msg_data[2] = velocity_limit_b[2];
//     msg_data[3] = velocity_limit_b[3];
//     msg_data[4] = current_limit_b[0];
//     msg_data[5] = current_limit_b[1];
//     msg_data[6] = current_limit_b[2];
//     msg_data[7] = current_limit_b[3];

//     sendMessage(axis_id, CMD_ID_SET_LIMITS, false, 8, msg_data);
// }

// void ODriveCAN::SetTrajVelLimit(int axis_id, float traj_vel_limit) {
//     byte* traj_vel_limit_b = (byte*) &traj_vel_limit;

//     sendMessage(axis_id, CMD_ID_SET_TRAJ_VEL_LIMIT, false, 4, traj_vel_limit_b);
// }

// void ODriveCAN::SetTrajAccelLimits(int axis_id, float traj_accel_limit, float traj_decel_limit) {
// 	byte* traj_accel_limit_b = (byte*) &traj_accel_limit;
// 	byte* traj_decel_limit_b = (byte*) &traj_decel_limit;
// 	byte msg_data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	
// 	msg_data[0] = traj_accel_limit_b[0];
// 	msg_data[1] = traj_accel_limit_b[1];
// 	msg_data[2] = traj_accel_limit_b[2];
// 	msg_data[3] = traj_accel_limit_b[3];
// 	msg_data[4] = traj_decel_limit_b[0];
// 	msg_data[5] = traj_decel_limit_b[1];
// 	msg_data[6] = traj_decel_limit_b[2];
// 	msg_data[7] = traj_decel_limit_b[3];
	
// 	sendMessage(axis_id, CMD_ID_SET_TRAJ_ACCEL_LIMITS, false, 8, msg_data);
// }

// void ODriveCAN::SetTrajInertia(int axis_id, float traj_inertia) {
//     byte* traj_inertia_b = (byte*) &traj_inertia;

//     sendMessage(axis_id, CMD_ID_SET_TRAJ_INERTIA, false, 4, traj_inertia_b);
// }

// void ODriveCAN::SetLinearCount(int axis_id, int linear_count) {
//     byte* linear_count_b = (byte*) &linear_count;

//     sendMessage(axis_id, CMD_ID_SET_LINEAR_COUNT, false, 4, linear_count_b);
// }

// void ODriveCAN::SetPositionGain(int axis_id, float position_gain) {
//     byte* position_gain_b = (byte*) &position_gain;

//     sendMessage(axis_id, CMD_ID_SET_POS_GAIN, false, 4, position_gain_b);
// }

// void ODriveCAN::SetVelocityGains(int axis_id, float velocity_gain, float velocity_integrator_gain) {
//     byte* velocity_gain_b = (byte*) &velocity_gain;
// 	byte* velocity_integrator_gain_b = (byte*) &velocity_integrator_gain;
//     byte msg_data[8] = {0, 0, 0, 0, 0, 0, 0, 0};

//     msg_data[0] = velocity_gain_b[0];
//     msg_data[1] = velocity_gain_b[1];
//     msg_data[2] = velocity_gain_b[2];
//     msg_data[3] = velocity_gain_b[3];
//     msg_data[4] = velocity_integrator_gain_b[0];
//     msg_data[5] = velocity_integrator_gain_b[1];
//     msg_data[6] = velocity_integrator_gain_b[2];
//     msg_data[7] = velocity_integrator_gain_b[3];

//     sendMessage(axis_id, CMD_ID_SET_VEL_GAINS, false, 8, msg_data);
// }

//////////// Get functions ///////////

// void ODriveCAN::GetPositionVelocity(int axis_id) {
//     byte msg_data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	
//     sendMessage(axis_id, CMD_ID_GET_ENCODER_ESTIMATES, true, 8, msg_data);
// }

void ODriveCAN::GetPositionVelocityResponse(EncoderEstimatesMsg_t &returnVal, struct can_frame &inMsg) {
	returnVal.parseMessage(inMsg);
}

void ODriveCAN::GetEncoderCounts(int axis_id) {
	uint8_t msg_data[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    sendMessage(axis_id, CMD_ID_GET_ENCODER_COUNT, true, 8, msg_data);
}

// void ODriveCAN::GetEncoderCountsResponse(EncoderCountsMsg_t &returnVal, struct can_frame &inMsg) {
// 	returnVal.parseMessage(inMsg);
// }

// void ODriveCAN::GetIq(int axis_id) {
// 	byte msg_data[8] = {0, 0, 0, 0, 0, 0, 0, 0};

//     sendMessage(axis_id, CMD_ID_GET_IQ, true, 8, msg_data);
// }

// void ODriveCAN::GetIqResponse(IqMsg_t &returnVal, struct can_frame &inMsg) {
// 	returnVal.parseMessage(inMsg);
// }

// void ODriveCAN::GetSensorlessEstimates(int axis_id) {
// 	byte msg_data[8] = {0, 0, 0, 0, 0, 0, 0, 0};

//     sendMessage(axis_id, CMD_ID_GET_SENSORLESS_ESTIMATES, true, 8, msg_data);
// }

// void ODriveCAN::GetSensorlessEstimatesResponse(SensorlessEstimatesMsg_t &returnVal, struct can_frame &inMsg) {
// 	returnVal.parseMessage(inMsg);
// }

// void ODriveCAN::GetMotorError(int axis_id) {
//     byte msg_data[8] = {0, 0, 0, 0, 0, 0, 0, 0};

//     sendMessage(axis_id, CMD_ID_GET_MOTOR_ERROR, true, 8, msg_data);
// }

// uint64_t ODriveCAN::GetMotorErrorResponse(struct can_frame &inMsg) {
//     uint64_t output;
//     *((uint8_t *)(&output) + 0) = inMsg.buf[0];
//     *((uint8_t *)(&output) + 1) = inMsg.buf[1];
//     *((uint8_t *)(&output) + 2) = inMsg.buf[2];
//     *((uint8_t *)(&output) + 3) = inMsg.buf[3];
//     *((uint8_t *)(&output) + 0) = inMsg.buf[4];
//     *((uint8_t *)(&output) + 1) = inMsg.buf[5];
//     *((uint8_t *)(&output) + 2) = inMsg.buf[6];
//     *((uint8_t *)(&output) + 3) = inMsg.buf[7];
//     return output;
// }

// void ODriveCAN::GetControllerError(int axis_id) {
//     byte msg_data[8] = {0, 0, 0, 0, 0, 0, 0, 0};

//     sendMessage(axis_id, CMD_ID_GET_CONTROLLER_ERROR, true, 8, msg_data);
// }

// uint32_t ODriveCAN::GetControllerErrorResponse(struct can_frame &inMsg) {
//     uint32_t output;
//     *((uint8_t *)(&output) + 0) = inMsg.buf[0];
//     *((uint8_t *)(&output) + 1) = inMsg.buf[1];
//     *((uint8_t *)(&output) + 2) = inMsg.buf[2];
//     *((uint8_t *)(&output) + 3) = inMsg.buf[3];
//     return output;
// }

// void ODriveCAN::GetEncoderError(int axis_id) {
//     byte msg_data[8] = {0, 0, 0, 0, 0, 0, 0, 0};

//     sendMessage(axis_id, CMD_ID_GET_ENCODER_ERROR, true, 8, msg_data);
// }

// uint32_t ODriveCAN::GetEncoderErrorResponse(struct can_frame &inMsg) {
//     uint32_t output;
//     *((uint8_t *)(&output) + 0) = inMsg.buf[0];
//     *((uint8_t *)(&output) + 1) = inMsg.buf[1];
//     *((uint8_t *)(&output) + 2) = inMsg.buf[2];
//     *((uint8_t *)(&output) + 3) = inMsg.buf[3];
//     return output;
// }

//message can be sent to either axis
void ODriveCAN::GetVbusVoltage(int axis_id) {
    uint8_t msg_data[4] = {0, 0, 0, 0};

    sendMessage(axis_id, CMD_ID_GET_VBUS_VOLTAGE, true, 4, msg_data);
}

float ODriveCAN::GetVbusVoltageResponse(struct can_frame &inMsg) {
    float output;
    *((uint8_t*)(&output) + 0) = inMsg.data[0];
    *((uint8_t*)(&output) + 1) = inMsg.data[1];
    *((uint8_t*)(&output) + 2) = inMsg.data[2];
    *((uint8_t*)(&output) + 3) = inMsg.data[3];
    return output;
}

// void ODriveCAN::GetADCVoltage(int axis_id, uint8_t gpio_num) {
//     byte msg_data[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// 	msg_data[0] = gpio_num;
	
//     sendMessage(axis_id, CMD_ID_GET_ADC_VOLTAGE, true, 8, msg_data);  //RTR must be false!
// }

// float ODriveCAN::GetADCVoltageResponse(struct can_frame &inMsg) {
//     float_t output;
//     *((uint8_t *)(&output) + 0) = inMsg.buf[0];
//     *((uint8_t *)(&output) + 1) = inMsg.buf[1];
//     *((uint8_t *)(&output) + 2) = inMsg.buf[2];
//     *((uint8_t *)(&output) + 3) = inMsg.buf[3];
//     return output;
// }

//////////// Other functions ///////////

void ODriveCAN::Estop(int axis_id) {
    sendMessage(axis_id, CMD_ID_ODRIVE_ESTOP_MESSAGE, false, 0, 0);  //message requires no data, thus the 0, 0
}

void ODriveCAN::StartAnticogging(int axis_id) {
    sendMessage(axis_id, CMD_ID_START_ANTICOGGING, false,0, 0);  //message requires no data, thus the 0, 0
}

void ODriveCAN::RebootOdrive(int axis_id) {  //message can be sent to either axis
    sendMessage(axis_id, CMD_ID_REBOOT_ODRIVE, false,0, 0);
}

void ODriveCAN::ClearErrors(int axis_id) {
    sendMessage(axis_id, CMD_ID_CLEAR_ERRORS, false, 0, 0);  //message requires no data, thus the 0, 0
}

//////////// State helper ///////////

bool ODriveCAN::RunState(int axis_id, int requested_state) {
    sendMessage(axis_id, CMD_ID_SET_AXIS_REQUESTED_STATE, false, 4, (uint8_t*) &requested_state);
    return true;
}
