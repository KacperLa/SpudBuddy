
#ifndef PF_CAN 
#define PF_CAN 29 
#endif 
#ifndef AF_CAN 
#define AF_CAN PF_CAN 
#endif 

#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#include <stdio.h>
#include <cstring>
#include <cstdint>

#include <fcntl.h>

#include <net/if.h>
#include <sys/ioctl.h>



#include <iostream>
//#include "tcp.h"
#include <unistd.h>




// HeartbeatMsg_t struct defintion
struct HeartbeatMsg_t {
    uint32_t axisError = 0;
    uint8_t currentState = 0;
    uint8_t motorFlag = 0;
    uint8_t encoderFlag = 0;
    uint8_t trajectoryDone = 0;
    uint8_t controllerFlag = 0;

    // Member function that takes a struct can_frame (or could just do a raw buf[8])
    void parseMessage(const struct can_frame &inMsg) {
        memcpy(&(axisError), &inMsg.data[0], 4);
        currentState = inMsg.data[4];
        motorFlag = inMsg.data[5];
        encoderFlag = inMsg.data[6];

        controllerFlag = inMsg.data[7] & 1UL;
        trajectoryDone = (inMsg.data[7] >> 7) & 1UL;
    }
};

struct EncoderEstimatesMsg_t {
	float posEstimate = 0;
	float velEstimate = 0;
	
	void parseMessage(const struct can_frame &inMsg) {
        memcpy(&(posEstimate), &inMsg.data[0], 4);
        memcpy(&(velEstimate), &inMsg.data[4], 4);
	}
};

struct EncoderCountsMsg_t {
	int32_t shadowCount = 0;
	int32_t countInCPR = 0;
	
	void parseMessage(const struct can_frame &inMsg) {
        memcpy(&(shadowCount), &inMsg.data[0], 4);
        memcpy(&(countInCPR), &inMsg.data[4], 4);
	}
};

struct IqMsg_t {
	float iqSetpoint = 0;
	float iqMeasured = 0;
	
	void parseMessage(const struct can_frame &inMsg) {
        memcpy(&(iqSetpoint), &inMsg.data[0], 4);
        memcpy(&(iqMeasured), &inMsg.data[4], 4);
	}
};

struct SensorlessEstimatesMsg_t {
	float posEstimate = 0;
	float velEstimate = 0;
	
	void parseMessage(const struct can_frame &inMsg) {
        memcpy(&(posEstimate), &inMsg.data[0], 4);
        memcpy(&(velEstimate), &inMsg.data[4], 4);
	}
};

class ODriveCAN {
public:
    enum AxisState_t {
        AXIS_STATE_UNDEFINED = 0,           //<! will fall through to idle
        AXIS_STATE_IDLE = 1,                //<! disable PWM and do nothing
        AXIS_STATE_STARTUP_SEQUENCE = 2, //<! the actual sequence is defined by the config.startup_... flags
        AXIS_STATE_FULL_CALIBRATION_SEQUENCE = 3,   //<! run all calibration procedures, then idle
        AXIS_STATE_MOTOR_CALIBRATION = 4,   //<! run motor calibration
        AXIS_STATE_SENSORLESS_CONTROL = 5,  //<! run sensorless control
        AXIS_STATE_ENCODER_INDEX_SEARCH = 6, //<! run encoder index search
        AXIS_STATE_ENCODER_OFFSET_CALIBRATION = 7, //<! run encoder offset calibration
        AXIS_STATE_CLOSED_LOOP_CONTROL = 8  //<! run closed loop control
    };
	
	enum ControlMode_t {
		VOLTAGE_CONTROL = 0,
		TORQUE_CONTROL = 1,
		VELOCITY_CONTROL = 2,
		POSITION_CONTROL = 3
	};
	
	enum InputMode_t {
		INACTIVE = 0,
		PASSTHROUGH = 1,
		VEL_RAMP = 2,
		POS_FILTER = 3,
		MIX_CHANNELS = 4,
		TRAP_TRAJ = 5,
		TORQUE_RAMP = 6,
		MIRROR = 7,
		TUNING = 8
	};

    enum CommandId_t {
        CMD_ID_CANOPEN_NMT_MESSAGE = 0x000,
        CMD_ID_ODRIVE_HEARTBEAT_MESSAGE = 0x001,
        CMD_ID_ODRIVE_ESTOP_MESSAGE = 0x002,
        CMD_ID_GET_MOTOR_ERROR = 0x003,
        CMD_ID_GET_ENCODER_ERROR = 0x004,
        CMD_ID_GET_SENSORLESS_ERROR = 0x005,
        CMD_ID_SET_AXIS_NODE_ID = 0x006,
        CMD_ID_SET_AXIS_REQUESTED_STATE = 0x007,
        CMD_ID_SET_AXIS_STARTUP_CONFIG = 0x008,
        CMD_ID_GET_ENCODER_ESTIMATES = 0x009,
        CMD_ID_GET_ENCODER_COUNT = 0x00A,
        CMD_ID_SET_CONTROLLER_MODES = 0x00B,
        CMD_ID_SET_INPUT_POS = 0x00C,
        CMD_ID_SET_INPUT_VEL = 0x00D,
        CMD_ID_SET_INPUT_TORQUE = 0x00E,
		CMD_ID_SET_LIMITS = 0x00F,
        CMD_ID_START_ANTICOGGING = 0x010,
        CMD_ID_SET_TRAJ_VEL_LIMIT = 0x011,
        CMD_ID_SET_TRAJ_ACCEL_LIMITS = 0x012,
        CMD_ID_SET_TRAJ_INERTIA = 0x013,
        CMD_ID_GET_IQ = 0x014,
        CMD_ID_GET_SENSORLESS_ESTIMATES = 0x015,
        CMD_ID_REBOOT_ODRIVE = 0x016,
        CMD_ID_GET_VBUS_VOLTAGE = 0x017,
        CMD_ID_CLEAR_ERRORS = 0x018,
		CMD_ID_SET_LINEAR_COUNT = 0x019,
		CMD_ID_SET_POS_GAIN = 0x01A,
		CMD_ID_SET_VEL_GAINS = 0x01B,
		CMD_ID_GET_ADC_VOLTAGE = 0x01C,
		CMD_ID_GET_CONTROLLER_ERROR = 0x01D,
        CMD_ID_CANOPEN_HEARTBEAT_MESSAGE = 0x700
    };
    
    int sock;
    const std::string CAN_INTERFACE_NAME = "can0";

    ODriveCAN();
    
    int open();

    void sendMessage(int axis_id, int cmd_id, bool remote_function, int length, uint8_t *signal_bytes);
	
	int readMsg(struct can_frame& inMsg);
	
	// Heartbeat
	void Heartbeat(HeartbeatMsg_t &returnVals, struct can_frame &inMsg);

    // Setters
	// void SetAxisNodeId(int axis_id, int node_id);
	// void SetControllerModes(int axis_id, int control_mode, int input_mode);
    // void SetPosition(int axis_id, float position);
    // void SetPosition(int axis_id, float position, float velocity_feedforward);
    // void SetPosition(int axis_id, float position, float velocity_feedforward, float current_feedforward);
    void SetVelocity(int axis_id, float velocity);
    void SetVelocity(int axis_id, float velocity, float current_feedforward);
    void SetTorque(int axis_id, float torque);
	//void SetLimits(int axis_id, float velocity_limit, float current_limit);
	// void SetTrajVelLimit(int axis_id, float traj_vel_limit);
	// void SetTrajAccelLimits(int axis_id, float traj_accel_limit, float traj_decel_limit);
	// void SetTrajInertia(int axis_id, float traj_inertia);
	// void SetLinearCount(int axis_id, int linear_count);
	// void SetPositionGain(int axis_id, float position_gain);
	// void SetVelocityGains(int axis_id, float velocity_gain, float velocity_integrator_gain);

    // Getters
    // void GetPositionVelocity(int axis_id);
    void GetPositionVelocityResponse(EncoderEstimatesMsg_t &returnVal, struct can_frame &inMsg);
	void GetEncoderCounts(int axis_id);
	// void GetEncoderCountsResponse(EncoderCountsMsg_t &returnVal, struct can_frame &inMsg);
	// void GetIq(int axis_id);
	// void GetIqResponse(IqMsg_t &returnVal, struct can_frame &inMsg);
	// void GetSensorlessEstimates(int axis_id);
	// void GetSensorlessEstimatesResponse(SensorlessEstimatesMsg_t &returnVal, struct can_frame &inMsg);
    // void GetMotorError(int axis_id);
	// uint64_t GetMotorErrorResponse(struct can_frame &inMsg);
    // void GetControllerError(int axis_id);
    // uint32_t GetControllerErrorResponse(struct can_frame &inMsg);
    // void GetEncoderError(int axis_id);
    // uint32_t GetEncoderErrorResponse(struct can_frame &inMsg);
	void GetVbusVoltage(int axis_id);  //Can be sent to either axis
	float GetVbusVoltageResponse(struct can_frame &inMsg);
	// void GetADCVoltage(int axis_id, uint8_t gpio_num);
	// float GetADCVoltageResponse(struct can_frame &inMsg);
	
	// Other functions
	void Estop(int axis_id);
	void StartAnticogging(int axis_id);
	void RebootOdrive(int axis_id);  //Can be sent to either axis
	void ClearErrors(int axis_id);

    // State helper
    bool RunState(int axis_id, int requested_state);

};
