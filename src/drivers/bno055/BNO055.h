#include <stdint.h>
#include <string>
#include <unistd.h>

#pragma once

static const uint16_t BNO055_I2C_ADDRESS                {0x28};

static const uint16_t BNO055_ID                         {0xA0};
static const uint16_t POWER_MODE_NORMAL                 {0x00};
static const uint16_t CALIB_BYTECOUNT                   {34};
static const uint16_t REGISTERMAP_END                   {0x7F};

/* ------------------------------------------------------------ *
 * Page-0 registers with general confguration and data output   *
 * ------------------------------------------------------------ */
static const uint16_t BNO055_CHIP_ID_ADDR               {0x00};

/* Page ID register, for page switching */
static const uint16_t BNO055_PAGE_ID_ADDR               {0x07};

/* Accel data register */
static const uint16_t BNO055_ACC_DATA_X_LSB_ADDR        {0x08};
static const uint16_t BNO055_ACC_DATA_X_MSB_ADDR        {0x09};
static const uint16_t BNO055_ACC_DATA_Y_LSB_ADDR        {0x0A};
static const uint16_t BNO055_ACC_DATA_Y_MSB_ADDR        {0x0B};
static const uint16_t BNO055_ACC_DATA_Z_LSB_ADDR        {0x0C};
static const uint16_t BNO055_ACC_DATA_Z_MSB_ADDR        {0x0D};

/* Mag data register */
static const uint16_t BNO055_MAG_DATA_X_LSB_ADDR        {0x0E};
static const uint16_t BNO055_MAG_DATA_X_MSB_ADDR        {0x0F};
static const uint16_t BNO055_MAG_DATA_Y_LSB_ADDR        {0x10};
static const uint16_t BNO055_MAG_DATA_Y_MSB_ADDR        {0x11};
static const uint16_t BNO055_MAG_DATA_Z_LSB_ADDR        {0x12};
static const uint16_t BNO055_MAG_DATA_Z_MSB_ADDR        {0x13};

/* Gyro data registers */
static const uint16_t BNO055_GYRO_DATA_X_LSB_ADDR       {0x14};
static const uint16_t BNO055_GYRO_DATA_X_MSB_ADDR       {0x15};
static const uint16_t BNO055_GYRO_DATA_Y_LSB_ADDR       {0x16};
static const uint16_t BNO055_GYRO_DATA_Y_MSB_ADDR       {0x17};
static const uint16_t BNO055_GYRO_DATA_Z_LSB_ADDR       {0x18};
static const uint16_t BNO055_GYRO_DATA_Z_MSB_ADDR       {0x19};

/* Euler data registers */
static const uint16_t BNO055_EULER_H_LSB_ADDR           {0x1A};
static const uint16_t BNO055_EULER_H_MSB_ADDR           {0x1B};
static const uint16_t BNO055_EULER_R_LSB_ADDR           {0x1C};
static const uint16_t BNO055_EULER_R_MSB_ADDR           {0x1D};
static const uint16_t BNO055_EULER_P_LSB_ADDR           {0x1E};
static const uint16_t BNO055_EULER_P_MSB_ADDR           {0x1F};

/* Quaternion data registers */
static const uint16_t BNO055_QUATERNION_DATA_W_LSB_ADDR {0x20};
static const uint16_t BNO055_QUATERNION_DATA_W_MSB_ADDR {0x21};
static const uint16_t BNO055_QUATERNION_DATA_X_LSB_ADDR {0x22};
static const uint16_t BNO055_QUATERNION_DATA_X_MSB_ADDR {0x23};
static const uint16_t BNO055_QUATERNION_DATA_Y_LSB_ADDR {0x24};
static const uint16_t BNO055_QUATERNION_DATA_Y_MSB_ADDR {0x25};
static const uint16_t BNO055_QUATERNION_DATA_Z_LSB_ADDR {0x26};
static const uint16_t BNO055_QUATERNION_DATA_Z_MSB_ADDR {0x27};

/* Linear acceleration data registers */
static const uint16_t BNO055_LIN_ACC_DATA_X_LSB_ADDR    {0x28};
static const uint16_t BNO055_LIN_ACC_DATA_X_MSB_ADDR    {0x29};
static const uint16_t BNO055_LIN_ACC_DATA_Y_LSB_ADDR    {0x2A};
static const uint16_t BNO055_LIN_ACC_DATA_Y_MSB_ADDR    {0x2B};
static const uint16_t BNO055_LIN_ACC_DATA_Z_LSB_ADDR    {0x2C};
static const uint16_t BNO055_LIN_ACC_DATA_Z_MSB_ADDR    {0x2D};

/* Gravity data registers */
static const uint16_t BNO055_GRAVITY_DATA_X_LSB_ADDR    {0x2E};
static const uint16_t BNO055_GRAVITY_DATA_X_MSB_ADDR    {0x2F};
static const uint16_t BNO055_GRAVITY_DATA_Y_LSB_ADDR    {0x30};
static const uint16_t BNO055_GRAVITY_DATA_Y_MSB_ADDR    {0x31};
static const uint16_t BNO055_GRAVITY_DATA_Z_LSB_ADDR    {0x32};
static const uint16_t BNO055_GRAVITY_DATA_Z_MSB_ADDR    {0x33};

/* Temperature data register */
static const uint16_t BNO055_TEMP_ADDR                  {0x34};

/* Status registers */
static const uint16_t BNO055_CALIB_STAT_ADDR            {0x35};
static const uint16_t BNO055_SELFTSTRES_ADDR            {0x36};
static const uint16_t BNO055_INTR_STAT_ADDR             {0x37};

static const uint16_t BNO055_SYS_CLK_STAT_ADDR          {0x38};
static const uint16_t BNO055_SYS_STAT_ADDR              {0x39};
static const uint16_t BNO055_SYS_ERR_ADDR               {0x3A};

/* Unit selection register */
static const uint16_t BNO055_UNIT_SEL_ADDR              {0x3B};
static const uint16_t BNO055_DATA_SELECT_ADDR           {0x3C};

/* Mode registers */
static const uint16_t BNO055_OPR_MODE_ADDR              {0x3D};
static const uint16_t BNO055_PWR_MODE_ADDR              {0x3E};

static const uint16_t BNO055_SYS_TRIGGER_ADDR           {0x3F};
static const uint16_t BNO055_TEMP_SOURCE_ADDR           {0x40};

/* Axis remap registers */
static const uint16_t BNO055_AXIS_MAP_CONFIG_ADDR       {0x41};
static const uint16_t BNO055_AXIS_MAP_SIGN_ADDR         {0x42};

/* Soft Iron Calibration registers */
static const uint16_t BNO055_SIC_MATRIX_0_LSB_ADDR      {0x43};
static const uint16_t BNO055_SIC_MATRIX_0_MSB_ADDR      {0x44};
static const uint16_t BNO055_SIC_MATRIX_1_LSB_ADDR      {0x45};
static const uint16_t BNO055_SIC_MATRIX_1_MSB_ADDR      {0x46};
static const uint16_t BNO055_SIC_MATRIX_2_LSB_ADDR      {0x47};
static const uint16_t BNO055_SIC_MATRIX_2_MSB_ADDR      {0x48};
static const uint16_t BNO055_SIC_MATRIX_3_LSB_ADDR      {0x49};
static const uint16_t BNO055_SIC_MATRIX_3_MSB_ADDR      {0x4A};
static const uint16_t BNO055_SIC_MATRIX_4_LSB_ADDR      {0x4B};
static const uint16_t BNO055_SIC_MATRIX_4_MSB_ADDR      {0x4C};
static const uint16_t BNO055_SIC_MATRIX_5_LSB_ADDR      {0x4D};
static const uint16_t BNO055_SIC_MATRIX_5_MSB_ADDR      {0x4E};
static const uint16_t BNO055_SIC_MATRIX_6_LSB_ADDR      {0x4F};
static const uint16_t BNO055_SIC_MATRIX_6_MSB_ADDR      {0x50};
static const uint16_t BNO055_SIC_MATRIX_7_LSB_ADDR      {0x51};
static const uint16_t BNO055_SIC_MATRIX_7_MSB_ADDR      {0x52};
static const uint16_t BNO055_SIC_MATRIX_8_LSB_ADDR      {0x53};
static const uint16_t BNO055_SIC_MATRIX_8_MSB_ADDR      {0x54};

/* Accelerometer Offset registers */
static const uint16_t ACC_OFFSET_X_LSB_ADDR             {0x55};
static const uint16_t ACC_OFFSET_X_MSB_ADDR             {0x56};
static const uint16_t ACC_OFFSET_Y_LSB_ADDR             {0x57};
static const uint16_t ACC_OFFSET_Y_MSB_ADDR             {0x58};
static const uint16_t ACC_OFFSET_Z_LSB_ADDR             {0x59};
static const uint16_t ACC_OFFSET_Z_MSB_ADDR             {0x5A};
 
/* Magnetometer Offset registers */
static const uint16_t MAG_OFFSET_X_LSB_ADDR             {0x5B};
static const uint16_t MAG_OFFSET_X_MSB_ADDR             {0x5C};
static const uint16_t MAG_OFFSET_Y_LSB_ADDR             {0x5D};
static const uint16_t MAG_OFFSET_Y_MSB_ADDR             {0x5E};
static const uint16_t MAG_OFFSET_Z_LSB_ADDR             {0x5F};
static const uint16_t MAG_OFFSET_Z_MSB_ADDR             {0x60};

/* Gyroscope Offset register s*/
static const uint16_t GYRO_OFFSET_X_LSB_ADDR            {0x61};
static const uint16_t GYRO_OFFSET_X_MSB_ADDR            {0x62};
static const uint16_t GYRO_OFFSET_Y_LSB_ADDR            {0x63};
static const uint16_t GYRO_OFFSET_Y_MSB_ADDR            {0x64};
static const uint16_t GYRO_OFFSET_Z_LSB_ADDR            {0x65};
static const uint16_t GYRO_OFFSET_Z_MSB_ADDR            {0x66};

/* Radius registers */
static const uint16_t ACCEL_RADIUS_LSB_ADDR             {0x67};
static const uint16_t ACCEL_RADIUS_MSB_ADDR             {0x68};
static const uint16_t MAG_RADIUS_LSB_ADDR               {0x69};
static const uint16_t MAG_RADIUS_MSB_ADDR               {0x6A};

/* ------------------------------------------------------------ *
 * Page-1 contains sensor component specific confguration data  *
 * ------------------------------------------------------------ */
static const uint16_t BNO055_ACC_CONFIG_ADDR            {0x08};
static const uint16_t BNO055_MAG_CONFIG_ADDR            {0x09};
static const uint16_t BNO055_GYR_CONFIG0_ADDR           {0x0A};
static const uint16_t BNO055_GYR_CONFIG1_ADDR           {0x0B};
static const uint16_t BNO055_ACC_SLEEP_CONFIG_ADDR      {0x0C};
static const uint16_t BNO055_GYR_SLEEP_CONFIG_ADDR      {0x0D};

/* ------------------------------------------------------------ *
 * Operations and power mode, name to value translation         *
 * ------------------------------------------------------------ */
typedef enum {
    error    = -1,
    config   = 0x00,
    acconly  = 0x01,
    magonly  = 0x02,
    gyronly  = 0x03,
    accmag   = 0x04,
    accgyro  = 0x05,
    maggyro  = 0x06,
    amg      = 0x07,
    imu      = 0x08,
    compass  = 0x09,
    m4g      = 0x0A,
    ndof     = 0x0B,
    ndof_fmc = 0x0C,
    mask     = 0x0F
} operation_mode_t;

typedef enum {
    normal  = 0x00,
    low     = 0x01,
    suspend = 0x02
} power_t;

typedef enum {
    PERIPHERAL_INIT_ERROR = 0x01,
    SYSTEM_INIT_ERROR = 0x02,
    SELF_TEST_FAILED = 0x03,
    REG_VALUE_OUT_OF_RANGE = 0x04,
    REG_ADDR_OUT_OF_RANGE = 0x05,
    REG_WRITE_ERROR = 0x06,
    LOW_POWER_MODE_NOT_AVALIBLE = 0x07,
    ACC_POWER_MODE_NOT_AVALIBLE = 0x08,
    FUSION_CONFIG_EORROR = 0x09,
    SYS_CONFIG_ERROR = 0x0A
} sys_error_t;

class BNO055
{
public:
    BNO055();
    virtual ~BNO055();

    /* ------------------------------------------------------------ *
     * BNO055 versions, status data and other infos struct          *
     * ------------------------------------------------------------ */
    struct bno_info{
            char chip_id;  // reg 0x00 default 0xA0
            char acc_id;   // reg 0x01 default 0xFB
            char mag_id;   // reg 0x02 default 0x32
            char gyr_id;   // reg 0x03 default 0x0F
            char sw_lsb;   // reg 0x04 default 0x08
            char sw_msb;   // reg 0x05 default 0x03
            char bl_rev;   // reg 0x06 no default
            char opr_mode; // reg 0x3D default 0x1C
            char pwr_mode; // reg 0x3E default 0x00
            char axr_conf; // reg 0x41 default 0x24
            char axr_sign; // reg 0x42 default 0x00
            char sys_stat; // reg 0x39 system error status, range 0-6
            char selftest; // reg 0x36 self test result
            char sys_err;  // reg 0x3a system error code, 0=OK
            char unitsel;  // reg 0x3b SI units definition
            char temp_val; // reg 0x34 sensor temperature value
    };

    /* ------------------------------------------------------------ *
     * BNO055 calibration data struct. The offset ranges depend on  *
     * the component operation range. For example, the accelerometer*
     * range can be set as 2G, 4G, 8G, and 16G. I.e. the offset for *
     * the accelerometer at 16G has a range of +/- 16000mG. Offset  *
     * is stored on the sensor in two bytes with max value of 32768.*
     * ------------------------------------------------------------ */
    struct bno_cal{
            char scal_st;  // reg 0x35 system calibration state, range 0-3
            char gcal_st;  // gyroscope calibration state, range 0-3
            char acal_st;  // accelerometer calibration state, range 0-3
            char mcal_st;  // magnetometer calibration state, range 0-3
            int  aoff_x;   // accelerometer offset, X-axis
            int  aoff_y;   // accelerometer offset, Y-axis
            int  aoff_z;   // accelerometer offset, Z-axis
            int  moff_x;   // magnetometer offset, X-axis
            int  moff_y;   // magnetometer offset, Y-axis
            int  moff_z;   // magnetometer offset, Z-axis
            int  goff_x;   // gyroscope offset, X-axis
            int  goff_y;   // gyroscope offset, Y-axis
            int  goff_z;   // gyroscope offset, Z-axis
            int acc_rad;   // accelerometer radius
            int mag_rad;   // magnetometer radius
    };

    /* ------------------------------------------------------------ *
     * BNO055 measurement data structs. Data gets filled in based   *
     * on the sensor component type that was requested for reading. *
     * ------------------------------------------------------------ */
    struct accelerometer{
            double accel_x;   // accelerometer value, X-axis
            double accel_y;   // accelerometer value, Y-axis
            double accel_z;   // accelerometer value, Z-axis
    };

    struct magnetometer{
            double mag_x;   // magnetometer value, X-axis
            double mag_y;   // magnetometer value, Y-axis
            double mag_z;   // magnetometer value, Z-axis
    };

    struct gyro{
            double gyro_roll;   // gyroscope value, X-axis
            double gyro_pitch;   // gyroscope value, Y-axis
            double gyro_yaw;   // gyroscope value, Z-axis
    };

    struct euler_angles{
            double roll;    // Euler roll value
            double pitch;   // Euler pitch value
            double yaw;     // Euler yaw value
    };

    struct quaternion{
            double quater_w;  // Quaternation value - W (scalar)
            double quater_x;  // Quaternation value - X
            double quater_y;  // Quaternation value - Y
            double quater_z;  // Quaternation value - Z
    };

    struct gravity_vector{
            double gravity_x;  // Gravity Vector X-axis
            double gravity_y;  // Gravity Vector Y-axis
            double gravity_z;  // Gravity Vector Z-axis
    };

    struct linear_acceleration{
            double linear_accel_x;  // Linear Acceleration X-axis
            double linear_accel_y;  // Linear Acceleration Y-axis
            double linear_accel_z;  // Linear Acceleration Z-axis
    };

    /**
     * BNO055 accelerometer gyroscope magnetometer config structs.
     */
    struct accel_config{
            int pwr_mode;       // p-1 reg 0x08 accelerometer power mode
            int bandwidth;      // p-1 reg 0x08 accelerometer bandwidth
            int range;          // p-1 reg 0x08 accelerometer rate
            int sleep_mode;     // p-1 reg 0x0C accelerometer sleep mode
            int sleep_duration; // p-1 reg 0x0C accelerometer sleep duration
    };

    struct gyro_config{
            int pwr_mode;               // p-1 reg 0x0B gyroscope power mode
            int bandwidth;              // p-1 reg 0x0A gyroscope bandwidth
            int range;                  // p-1 reg 0x0A gyroscope range
            int sleep_duration;         // p-1 reg 0x0D gyroscope sleep duration
            int auto_sleep_duration;    // p-1 reg 0x0D gyroscope auto sleep dur
    };

    struct mag_config{
            int pwr_mode;       // p-1 reg 0x09 magnetometer power mode
            int operation_mode; // p-1 reg 0x09 magnetometer operation
            int output_rate;    // p-1 reg 0x09 magnetometer output rate
    };

    /**
     * Dumps the register map data.
     */
    int register_map_dump();

    /**
     * Resets the sensor. It will come up in CONFIG mode.
     */
    int sensor_reset();

    /**
     * Starts the imu driver.
     */
    int imu_init(const char* i2c_bus, uint i2c_address, operation_mode_t newmode);


    /**
     * Enables the I2C bus communication. Raspberry
     * Pi 2 uses i2c-1, RPI 1 used i2c-0, NanoPi also uses i2c-0.
     */
    int get_i2c_bus(const char* i2c_bus, uint i2c_address);

    /**
     * Gets the calibration state from the sensor.
     * Calibration status has 4 values, encoded as 2bit in reg 0x35.
     */
    int get_cal_status(bno_cal* bno_ptr);

    /** 
     * Calibration offset is stored in 3x6 (18) registers 0x55~0x66
     * plus 4 registers 0x67~0x6A accelerometer/magnetometer radius.
     */   
    int get_cal_offset(bno_cal* bno_ptr);

    /**
     * Queries the BNO055 and write the info data into
     * the global struct bno_info defined in getbno055.h
     */
    int get_info(bno_info* bno_ptr);

    /**
     *  Reads accelerometer data into the global struct
     */
    int get_accel_data(accelerometer* accel);

    /**
     * Reads gyroscope data into the global struct
     */
    int get_gyro_data(gyro* gyro);

    /**
     *  Reads magnetometer data into the global struct
     *  Convert magnetometer data in microTesla. 1 microTesla = 16
     */
    int get_mag_data(magnetometer* mag);

    /**
     * Reads Euler orientation into the global struct.
     */
    int get_euler_angles(euler_angles* euler_angles);

    /**
     * Reads Quaternation data into the global struct.
     */
    int get_quaternion(quaternion* quaternion);

    /**
     * Reads gravity vector into the global struct.
     */
    int get_gravity_vector(gravity_vector* gravity);

    /**
     * Reads linear acceleration into the global struct.
     */
    int get_linear_accleration(linear_acceleration* linear_accel);

    /**
     * Returns setting for internal/external clock.
     */
    int get_clk_src();

    /**
     * Prints setting for internal/external clock.
     */
    void print_clk_src();

    /**
     * Returns sensor operational mode register 0x3D
     * Reads 1 byte from Operations Mode register 0x3d, and uses
     * only the lowest 4 bit. Bits 4-7 are unused, stripped off.
     */
    operation_mode_t get_mode();

    /**
     * Sets the sensor operational mode register 0x3D.
     * The modes cannot be switched over directly, first it needs
     * to be set to "config" mode before switching to the new mode.
     */
    int set_mode(operation_mode_t mode);

    /**
     * Prints sensor operational mode string from
     * sensor operational mode numeric value.
     */
    int print_mode(int mode);

    /**
     * Returns the sensor power mode from register 0x3e.
     * Only the lowest 2 bit are used, ignore the unused bits 2-7.
     */
    int get_power();

    /**
     * Sets the sensor power mode in register 0x3E.
     * The power modes cannot be switched over directly, first the
     * ops mode needs to be "config" to write the new power mode.
     */
    int set_power(power_t pwr_mode);

    /**
     * Prints the sensor power mode string from the
     * sensors power mode numeric value.
     */
     int print_power(int mode);

    /**
     * Returns the sensor sys status from register 0x39.
     */
    int get_sstat();

    /**
     * Prints the sensor system status string from
     * the numeric value located in the sys_stat register 0x39
     */
    int print_sstat(int stat_code);

    int get_serror();

    int print_serror(int error_code);

    /**
     * Returns axis remap data from registers 0x41 0x4.
     */
    int get_remap(char);

    /**
     * Prints the sensor axis configuration.
     * The numeric values are located in register 0x41.
     * Valid modes are: 0x24 (default), 0x21. 
     */
    int print_remap_conf(int mode);

    /**
     * Prints the sensor axis remapping +/-.
     * The numeric values are located in register 0x42.
     */
    int print_remap_sign(int mode);

    /**
     * Extracts the SI unit config from register 0x3B.
     */
    void print_unit(int mode);

    /**
     * Loads previously saved calibration data from file.
     */
    int load_cal(char* file);

    /**
     * Saves calibration data to file for reuse.
     */
    int save_cal(char* file);

    /**
     * Reads accelerometer/gyro/magnetometer config into a struct.
     * Requires switching register page 0->1 and back after reading.
     */
    int get_accel_config(accel_config* config_ptr);
    int get_gyro_config(gyro_config* config_ptr);
    int get_mag_config(mag_config* config_ptr);

    /**
     * Sets accelerometer/gyro/magnetometer configuration.
     */
    int set_accel_config();
    int set_gyro_config();
    int set_mag_config();

    /**
     * Print accel/gyro/mag configuration.
     */
    void print_accel_config(accel_config* config_ptr);
    void print_gyro_config(gyro_config* config_ptr);
    void print_mag_config(mag_config* config_ptr);

    /**
     * Set page ID = 0 to set default register access.
     */
    int set_page0();

    /**
     * Set page ID = 1 to switch the register access.
     */
     int set_page1();


protected:
private:

     int i2cfd;          // I2C file descriptor
     int verbose{0};     // debug flag, 0 = normal, 1 = debug mode
};