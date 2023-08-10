/* ------------------------------------------------------------ *
 * file:        bno055.cpp                                      *
 * purpose:     Extract sensor data from Bosch BNO055 modules.  *
 *              Functions for I2C bus communication, get and    *
 *              set sensor register data. Ths file belongs to   *
 *              the pi-bno055 package. Functions are called     *
 *              from getbno055.c, globals are in getbno055.h.   *
 *                                                              *
 * Requires:    I2C development packages i2c-tools libi2c-dev     *
 *                                                              *
 * author:      07/14/2018 Frank4DD                             *
 * ------------------------------------------------------------ */

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "BNO055.h"

BNO055::BNO055()
{
}

BNO055::~BNO055()
{
}

int BNO055::imu_init(const char* i2c_bus, uint i2c_address, operation_mode_t newmode)
{
  int ret = 0;
  for (size_t i = 0; i < 10; i++) {
    ret = get_i2c_bus(i2c_bus, i2c_address);
    usleep(100000);
    if (ret == 0) {
      break;
    }
  }
  if (ret != 0) {
    return (-1);
  }

  if (get_mode() != newmode) {
    while (set_mode(newmode) != 0) {
      usleep(100000);
    }

    usleep(100000);

    while (get_sstat() < 0x05) {
      usleep(100000);
    }
  }

  while (get_sstat() < 0x05) {
    usleep(100000);
  }

  return (0);
}

int BNO055::get_i2c_bus(const char* i2c_bus, uint i2c_address)
{

    if((i2cfd = open(i2c_bus, O_RDWR)) < 0) {
        printf("Error failed to open I2C bus [%s].\n", i2c_bus);
        return(-1);
    }
    // Set I2C device (BNO055 I2C address is  0x28 or 0x29).
    int addr = i2c_address;
    if(verbose == 1) printf("Debug: Sensor address: [0x%02X]\n", addr);

    if(ioctl(i2cfd, I2C_SLAVE, addr) != 0) {
        printf("Error can't find sensor at address [0x%02X].\n", addr);
        return(-1);
    }
    
    // I2C communication test is the only way to confirm success.
    char reg = BNO055_CHIP_ID_ADDR;
    if(write(i2cfd, &reg, 1) != 1) {
        printf("Error: I2C write failure register [0x%02X], sensor addr [0x%02X]?\n", reg, addr);
        return(-1);
    }

    char data;
    if(read(i2cfd, &data, 1) != 1) {
        printf("Error: Failed to read Chip ID at register [0x%02X], sensor addr [0x%02X]?\n", reg, addr);
        return(-1);
    }

    if(data != BNO055_ID) {
        printf("Error: Wrong Chip ID 0x%02X\n", data);
        return(-1);
    }

    return(0);
}

int BNO055::register_map_dump()
{
    int count = 0;

    printf("------------------------------------------------------\n");
    printf("BNO055 page-0:\n");
    printf("------------------------------------------------------\n");
    printf(" reg    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
    printf("------------------------------------------------------\n");

    while(count < 8) {
        char reg = count;
        if(write(i2cfd, &reg, 1) != 1) {
            printf("Error: I2C write failure for register 0x%02X\n", reg);
            exit(-1);
        }

        char data[16] = {0};
        if(read(i2cfd, &data, 16) != 16) {
            printf("Error: I2C read failure for register 0x%02X\n", reg);
            exit(-1);
         
        }
        printf("[0x%02X] %02X %02X %02X %02X %02X %02X %02X %02X",
                 (reg*16), data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
        printf(" %02X %02X %02X %02X %02X %02X %02X %02X\n",
                 data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
        count++;
    }

    set_page1();
    usleep(50 * 1000);
    count = 0;
    printf("------------------------------------------------------\n");
    printf("BNO055 page-1:\n");
    printf("------------------------------------------------------\n");
    printf(" reg    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
    printf("------------------------------------------------------\n");

    while(count < 8) {
        char reg = count;
        if(write(i2cfd, &reg, 1) != 1) {
            printf("Error: I2C write failure for register 0x%02X\n", reg);
            exit(-1);
        }

        char data[16] = {0};
        if(read(i2cfd, &data, 16) != 16) {
            printf("Error: I2C read failure for register 0x%02X\n", reg);
            exit(-1);
         
        }
        printf("[0x%02X] %02X %02X %02X %02X %02X %02X %02X %02X",
                 (reg*16), data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
        printf(" %02X %02X %02X %02X %02X %02X %02X %02X\n",
                 data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
        count++;
    }

    set_page0();
    usleep(50 * 1000);
    return(0);
}

int BNO055::sensor_reset()
{
    char data[2];
    data[0] = BNO055_SYS_TRIGGER_ADDR;
    data[1] = 0x20;

    if(write(i2cfd, data, 2) != 2) {
        printf("Error: I2C write failure for register 0x%02X\n", data[0]);
        return(-1);
    }

    if(verbose == 1) {
        printf("Debug: BNO055 Sensor Reset complete\n");
    }
    
    // After a reset, the sensor needs at leat 650ms to boot up.
    usleep(650 * 1000);
    return(0);
}

int BNO055::get_cal_status(bno_cal* bno_ptr)
{
    char reg = BNO055_CALIB_STAT_ADDR;
    if(write(i2cfd, &reg, 1) != 1) {
        printf("Error: I2C write failure for register 0x%02X\n", reg);
        return(-1);
    }

    char data = 0;
    if(read(i2cfd, &data, 1) != 1) {
        printf("Error: I2C read failure for register 0x%02X\n", reg);
        return(-1);
    }

    bno_ptr->scal_st = (data & 0b11000000) >> 6; // system calibration status
    
    if(verbose == 1) {
        printf("Debug: sensor system calibration: [%d]\n", bno_ptr->scal_st);
    }
    
    bno_ptr->gcal_st = (data & 0b00110000) >> 4; // gyro calibration
    
    if(verbose == 1) {
        printf("Debug:     gyroscope calibration: [%d]\n", bno_ptr->gcal_st);
    }
    
    bno_ptr->acal_st = (data & 0b00001100) >> 2; // accel calibration status
    
    if(verbose == 1) {
        printf("Debug: accelerometer calibration: [%d]\n", bno_ptr->acal_st);
    }
    
    bno_ptr->mcal_st = (data & 0b00000011);      // magneto calibration status
    
    if(verbose == 1) {
        printf("Debug:  magnetometer calibration: [%d]\n", bno_ptr->mcal_st);
    }

    return(0);
}

int BNO055::get_cal_offset(bno_cal *bno_ptr)
{
    // Registers may not update in fusion mode, switch to CONFIG.
    operation_mode_t old_mode = get_mode();
    set_mode(config);

    char reg = ACC_OFFSET_X_LSB_ADDR;
    if(write(i2cfd, &reg, 1) != 1) {
        printf("Error: I2C write failure for register 0x%02X\n", reg);
        return(-1);
    }

    if(verbose == 1) printf("Debug: I2C read %d bytes starting at register 0x%02X\n", CALIB_BYTECOUNT, reg);

    char data[CALIB_BYTECOUNT] = {0};
    if(read(i2cfd, data, CALIB_BYTECOUNT) != CALIB_BYTECOUNT) {
        printf("Error: I2C calibration data read from 0x%02X\n", reg);
        return(-1);
    }

    if(verbose == 1) {
        int i = 0;
        printf("Debug: Calibrationset:");

        while(i<CALIB_BYTECOUNT) {
            printf(" %02X", data[i]);
            i++;
        }

        printf("\n");
    }

    /* ------------------------------------------------------------ *
     * assigning accelerometer X-Y-Z offset, range per G-range      *
     * 16G = +/-16000, 8G = +/-8000, 4G = +/-4000, 2G = +/-2000     *
     * ------------------------------------------------------------ */
    if(verbose == 1) {
        printf("Debug: accelerometer offset: [%d] [%d] [%d] (X-Y-Z)\n",
                ((int16_t)data[1] << 8) | data[0],
                ((int16_t)data[3] << 8) | data[2],
                ((int16_t)data[5] << 8) | data[4]);
    }

    bno_ptr->aoff_x = ((int16_t)data[1] << 8) | data[0];
    bno_ptr->aoff_y = ((int16_t)data[3] << 8) | data[2];
    bno_ptr->aoff_z = ((int16_t)data[5] << 8) | data[4];

    /* ------------------------------------------------------------ *
     * assigning magnetometer X-Y-Z offset, offset range is +/-6400 *
     * ------------------------------------------------------------ */
    if(verbose == 1) {
        printf("Debug:  magnetometer offset: [%d] [%d] [%d] (X-Y-Z)\n",
                ((int16_t)data[7] << 8) | data[6],
                ((int16_t)data[9] << 8) | data[8],
                ((int16_t)data[11] << 8) | data[10]);
    }

    bno_ptr->moff_x = ((int16_t)data[7] << 8) | data[6];
    bno_ptr->moff_y = ((int16_t)data[9] << 8) | data[8];
    bno_ptr->moff_z = ((int16_t)data[11] << 8) | data[10];

    /* ------------------------------------------------------------ *
     * assigning gyroscope X-Y-Z offset, range depends on dps value *
     * 2000 = +/-32000, 1000 = +/-16000, 500 = +/-8000, etc         *
     * ------------------------------------------------------------ */
    if(verbose == 1) {
        printf("Debug:     gyroscope offset: [%d] [%d] [%d] (X-Y-Z)\n",
                ((int16_t)data[13] << 8) | data[12],
                ((int16_t)data[15] << 8) | data[14],
                ((int16_t)data[17] << 8) | data[16]);
    }

    bno_ptr->goff_x = ((int16_t)data[13] << 8) | data[12];
    bno_ptr->goff_y = ((int16_t)data[15] << 8) | data[14];
    bno_ptr->goff_z = ((int16_t)data[17] << 8) | data[16];

    /* ------------------------------------------------------------ *
     * assigning accelerometer radius, range is +/-1000             *
     * ------------------------------------------------------------ */
    if(verbose == 1) {
        printf("Debug: accelerometer radius: [%d] (+/-1000)\n",
                ((int16_t)data[19] << 8) | data[18]);
    }

    bno_ptr->acc_rad = ((int16_t)data[19] << 8) | data[18];

    /* ------------------------------------------------------------ *
     * assigning magnetometer radius, range is +/-960               *
     * ------------------------------------------------------------ */
    if(verbose == 1) {
        printf("Debug:  magnetometer radius: [%d] (+/- 960)\n",
                ((int16_t)data[21] << 8) | data[20]);
    }

    bno_ptr->mag_rad = ((int16_t)data[21] << 8) | data[20];
    set_mode(old_mode);
    return(0);
}

int BNO055::save_cal(char *file)
{
    /* --------------------------------------------------------- *
     * Read 34 bytes calibration data from registers 0x43~66,    *
     * plus 4 reg 0x67~6A with accelerometer/magnetometer radius *
     * switch to CONFIG, data is only visible in non-fusion mode *
     * --------------------------------------------------------- */
    operation_mode_t old_mode = get_mode();
    set_mode(config);
    int i = 0;
    //char reg = ACC_OFFSET_X_LSB_ADDR;
    char reg = BNO055_SIC_MATRIX_0_LSB_ADDR;
    if(write(i2cfd, &reg, 1) != 1) {
        printf("Error: I2C write failure for register 0x%02X\n", reg);
        return(-1);
    }

    if(verbose == 1) {
        printf("Debug: I2C read %d bytes starting at register 0x%02X\n",
                CALIB_BYTECOUNT, reg);
    }

    char data[CALIB_BYTECOUNT] = {0};
    if(read(i2cfd, data, CALIB_BYTECOUNT) != CALIB_BYTECOUNT) {
        printf("Error: I2C calibration data read from 0x%02X\n", reg);
        return(-1);
    }

    if(verbose == 1) {
        printf("Debug: Calibrationset:");
        while(i<CALIB_BYTECOUNT) {
            printf(" %02X", data[i]);
            i++;
        }
        printf("\n");
    }

    // Open the calibration data file for writing.
    FILE *calib;
    if(! (calib=fopen(file, "w"))) {
        printf("Error: Can't open %s for writing.\n", file);
        exit(-1);
    }
    if(verbose == 1) {
        printf("Debug:  Write to file: [%s]\n", file);
    }

    // write the bytes in data[] out.
    int outbytes = fwrite(data, 1, CALIB_BYTECOUNT, calib);
    fclose(calib);
    
    if(verbose == 1) {
        printf("Debug:  Bytes to file: [%d]\n", outbytes);
    }
    
    if(outbytes != CALIB_BYTECOUNT) {
        printf("Error: %d/%d bytes written to file.\n", outbytes, CALIB_BYTECOUNT);
        return(-1);
    }

    set_mode(old_mode);
    return(0);
}

int BNO055::load_cal(char *file)
{
    //  Open the calibration data file for reading.
    FILE *calib;
    if(! (calib=fopen(file, "r"))) {
        printf("Error: Can't open %s for reading.\n", file);
        exit(-1);
    }

    if(verbose == 1) printf("Debug: Load from file: [%s]\n", file);

    // Read 34 bytes from file into data[], starting at data[1].
    char data[CALIB_BYTECOUNT+1] = {0};
    //data[0] = ACC_OFFSET_X_LSB_ADDR;
    data[0] = BNO055_SIC_MATRIX_0_LSB_ADDR;
    int inbytes = fread(&data[1], 1, CALIB_BYTECOUNT, calib);
    fclose(calib);

    if(inbytes != CALIB_BYTECOUNT) {
        printf("Error: %d/%d bytes read to file.\n", inbytes, CALIB_BYTECOUNT);
        return(-1);
    }

    if(verbose == 1) {
        printf("Debug: Calibrationset:");
        int i = 1;
        while(i<CALIB_BYTECOUNT+1) {
            printf(" %02X", data[i]);
            i++;
        }
        printf("\n");
    }

    /* -------------------------------------------------------- *
     * Write 34 bytes from file into sensor registers from 0x43 *
     * We need to switch in and out of CONFIG mode if needed... *
     * -------------------------------------------------------- */
    operation_mode_t old_mode = get_mode();
    set_mode(config);
    usleep(50 * 1000);

    if(write(i2cfd, data, (CALIB_BYTECOUNT+1)) != (CALIB_BYTECOUNT+1)) {
        printf("Error: I2C write failure for register 0x%02X\n", data[0]);
        return(-1);
    }

    // To verify, we read 34 bytes from 0x43 & compare to input.
    //char reg = ACC_OFFSET_X_LSB_ADDR;
    char reg = BNO055_SIC_MATRIX_0_LSB_ADDR;
    if(write(i2cfd, &reg, 1) != 1) {
        printf("Error: I2C write failure for register 0x%02X\n", reg);
        return(-1);
    }

    char newdata[CALIB_BYTECOUNT] = {0};
    if(read(i2cfd, newdata, CALIB_BYTECOUNT) != CALIB_BYTECOUNT) {
        printf("Error: I2C calibration data read from 0x%02X\n", reg);
        return(-1);
    }

    if(verbose == 1) {
        printf("Debug: Registerupdate:");
    }

    int i = 0;
    while(i<CALIB_BYTECOUNT) {
        if(data[i+1] != newdata[i]) {
            printf("\nError: Calibration load failure %02X register 0x%02X\n", newdata[i], reg+i);
            //exit(-1);
        }
        if(verbose == 1) printf(" %02X", newdata[i]);
        i++;
    }

    if(verbose == 1) {
        printf("\n");
    }

    set_mode(old_mode);

    // 650 ms delay are only needed if -l and -t are both used
    // to let the fusion code process the new calibration data.
    usleep(650 * 1000);
    return(0);
}

void BNO055::print_unit(int unit_sel)
{
    // bit-0
    printf("Acceleration Unit  = ");
    if((unit_sel >> 0) & 0x01) {
        printf("mg\n");
    } else {
        printf("m/s2\n");
    }

    // bit-1
    printf("    Gyroscope Unit = ");
    if((unit_sel >> 1) & 0x01) {
        printf("rps\n");
    } else {
        printf("dps\n");
    }

    // bit-2
    printf("        Euler Unit = ");
    if((unit_sel >> 2) & 0x01) {
        printf("Radians\n");
    } else {
        printf("Degrees\n");
    }

    // bit-3: unused
    // bit-4
    printf("  Temperature Unit = ");
    if((unit_sel >> 4) & 0x01) {
        printf("Fahrenheit\n");
    } else {
        printf("Celsius\n");
    }

    // bit-5: unused
    // bit-6: unused
    // bit-7
    printf("  Orientation Mode = ");
    if((unit_sel >> 3) & 0x01) {
        printf("Android\n");
    } else {
        printf("Windows\n");
    }
}

int BNO055::get_info(bno_info *bno_ptr)
{
    char reg = 0x00;
    if(write(i2cfd, &reg, 1) != 1) {
        printf("Error: I2C write failure for register 0x%02X\n", reg);
        return(-1);
    }

    char data[7] = {0};
    if(read(i2cfd, data, 7) != 7) {
        printf("Error: I2C read failure for register data 0x00-0x06\n");
        return(-1);
    }

    // 1-byte chip ID in register 0x00, default: 0xA0.
    if(verbose == 1) {
        printf("Debug: Sensor CHIP ID: [0x%02X]\n", data[0]);
    }

    bno_ptr->chip_id = data[0];

    // 1-byte Accelerometer ID in register 0x01, default: 0xFB.
    if(verbose == 1) {
        printf("Debug: Sensor  ACC ID: [0x%02X]\n", data[1]);
    }

    bno_ptr->acc_id = data[1];

    // 1-byte Magnetometer ID in register 0x02, default 0x32.
    if(verbose == 1) {
        printf("Debug: Sensor  MAG ID: [0x%02X]\n", data[2]);
    }

    bno_ptr->mag_id = data[2];

    // 1-byte Gyroscope ID in register 0x03, default: 0x0F.
    if(verbose == 1) {
        printf("Debug: Sensor  GYR ID: [0x%02X]\n", data[3]);
    }

    bno_ptr->gyr_id = data[3];

    // 1-byte SW Revsion ID LSB in register 0x04, default: 0x08.
    if(verbose == 1) {
        printf("Debug: SW  Rev-ID LSB: [0x%02X]\n", data[4]);
    }

    bno_ptr->sw_lsb = data[4];

    // 1-byte SW Revision ID MSB in register 0x05, default: 0x03.
    if(verbose == 1) {
        printf("Debug: SW  Rev-ID MSB: [0x%02X]\n", data[5]);
    }

    bno_ptr->sw_msb = data[5];

    // 1-byte BootLoader Revision ID register 0x06, no default.
    if(verbose == 1) {
        printf("Debug: Bootloader Ver: [0x%02X]\n", data[6]);
    }

    bno_ptr->bl_rev = data[6];

    // Read the operations mode with get_mode(), default: 0x0.
    bno_ptr->opr_mode = get_mode();

    // Read the power mode with get_power(), default: 0x0.
    bno_ptr->pwr_mode = get_power();

    // Read the axis remap config get_remap('c'), default: 0x24.
    bno_ptr->axr_conf = get_remap('c');

    // Read the axis remap sign get_remap('s'), default: 0x00.
    bno_ptr->axr_sign = get_remap('s');

    // Read 1-byte system status from register 0x39, no default.
    reg = BNO055_SYS_STAT_ADDR;
    if(write(i2cfd, &reg, 1) != 1) {
        printf("Error: I2C write failure for register 0x%02X\n", reg);
        return(-1);
    }

    data[0] = 0;
    if(read(i2cfd, data, 1) != 1) {
        printf("Error: I2C read failure for register data 0x%02X\n", reg);
        return(-1);
    }
    if(verbose == 1) printf("Debug:  System Status: [0x%02X]\n", data[0]);
    bno_ptr->sys_stat = data[0];

    // Read 1-byte Self Test Result register 0x36, 0x0F=pass.
    reg = BNO055_SELFTSTRES_ADDR;
    if(write(i2cfd, &reg, 1) != 1) {
        printf("Error: I2C write failure for register 0x%02X\n", reg);
        return(-1);
    }

    data[0] = 0;
    if(read(i2cfd, data, 1) != 1) {
        printf("Error: I2C read failure for register data 0x%02X\n", reg);
        return(-1);
    }
    if(verbose == 1) {
        printf("Debug: Self-Test Mode: [0x%02X] 4bit [0x%02X]\n", data[0], data[0] & 0x0F);
    }

    bno_ptr->selftest = data[0] & 0x0F; // only get the lowest 4 bits

    // Read 1-byte System Error from register 0x3A, 0=OK.
    reg = BNO055_SYS_ERR_ADDR;
    if(write(i2cfd, &reg, 1) != 1) {
        printf("Error: I2C write failure for register 0x%02X\n", reg);
        return(-1);
    }

    data[0] = 0;
    if(read(i2cfd, data, 1) != 1) {
        printf("Error: I2C read failure for register data 0x%02X\n", reg);
        return(-1);
    }

    if(verbose == 1) {
        printf("Debug: Internal Error: [0x%02X]\n", data[0]);
    }

    bno_ptr->sys_err = data[0];

    // Read 1-byte Unit definition from register 0x3B, 0=OK.
    reg = BNO055_UNIT_SEL_ADDR;
    if(write(i2cfd, &reg, 1) != 1) {
        printf("Error: I2C write failure for register 0x%02X\n", reg);
        return(-1);
    }

    data[0] = 0;
    if(read(i2cfd, data, 1) != 1) {
        printf("Error: I2C read failure for register data 0x%02X\n", reg);
        return(-1);
    }

    if(verbose == 1) {
        printf("Debug: UnitDefinition: [0x%02X]\n", data[0]);
    }

    bno_ptr->unitsel = data[0];

    // Extract the temperature unit from the unit selection data.
    char t_unit;
    if((data[0] >> 4) & 0x01) {
        t_unit = 'F';
    } else {
        t_unit = 'C';
    }

    // Read sensor temperature from register 0x34, no default.
    reg = BNO055_TEMP_ADDR;
    if(write(i2cfd, &reg, 1) != 1) {
        printf("Error: I2C write failure for register 0x%02X\n", reg);
        return(-1);
    }

    data[0] = 0;
    if(read(i2cfd, data, 1) != 1) {
        printf("Error: I2C read failure for register data 0x%02X\n", reg);
        return(-1);
    }
    if(verbose == 1) {
        printf("Debug:    Temperature: [0x%02X] [%d°%c]\n", data[0], data[0], t_unit);
    }

    bno_ptr->temp_val = data[0];
    return(0);
}

int BNO055::get_accel_data(accelerometer *accel)
{
    char reg = BNO055_ACC_DATA_X_LSB_ADDR;
    if(write(i2cfd, &reg, 1) != 1) {
        printf("Error: I2C write failure for register 0x%02X\n", reg);
        return(-1);
    }

    char data[6] = {0};
    if(read(i2cfd, data, 6) != 6) {
        printf("Error: I2C read failure for register data 0x%02X\n", reg);
        return(-1);
    }

    int16_t buf = ((int16_t)data[1] << 8) | data[0];
    if(verbose == 1) {
        printf("Debug: Accelerometer Data X: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[0], data[1],buf);
    }

    accel->accel_x = static_cast<double>(buf);

    buf = ((int16_t)data[3] << 8) | data[2];
    if(verbose == 1) {
        printf("Debug: Accelerometer Data Y: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[2], data[3],buf);
    }

    accel->accel_y = (double) buf;

    buf = ((int16_t)data[5] << 8) | data[4];
    if(verbose == 1) {
        printf("Debug: Accelerometer Data Z: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[4], data[5],buf);
    }

    accel->accel_z = (double) buf;
    return(0);
}

int BNO055::get_mag_data(magnetometer *mag)
{
    char reg = BNO055_MAG_DATA_X_LSB_ADDR;
    if(write(i2cfd, &reg, 1) != 1) {
        printf("Error: I2C write failure for register 0x%02X\n", reg);
        return(-1);
    }

    char data[6] = {0};
    if(read(i2cfd, data, 6) != 6) {
        printf("Error: I2C read failure for register data 0x%02X\n", reg);
        return(-1);
    }

    int16_t buf = ((int16_t)data[1] << 8) | data[0]; 
    if(verbose == 1) {
        printf("Debug: Magnetometer Data X: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[0], data[1],buf);
    }

    mag->mag_x = (double) buf / 1.6;

    buf = ((int16_t)data[3] << 8) | data[2]; 
    if(verbose == 1) {
        printf("Debug: Magnetometer Data Y: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[2], data[3],buf);
    }

    mag->mag_y = (double) buf / 1.6;

    buf = ((int16_t)data[5] << 8) | data[4]; 
    if(verbose == 1) {
        printf("Debug: Magnetometer Data Z: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[4], data[5],buf);
    }

    mag->mag_z = (double) buf / 1.6;
    return(0);
}

int BNO055::get_gyro_data(gyro *gyro)
{
    char reg = BNO055_GYRO_DATA_X_LSB_ADDR;
    if(write(i2cfd, &reg, 1) != 1) {
        printf("Error: I2C write failure for register 0x%02X\n", reg);
        return(-1);
    }

    char data[6] = {0};
    if(read(i2cfd, data, 6) != 6) {
        printf("Error: I2C read failure for register data 0x%02X\n", reg);
        return(-1);
    }

    int16_t buf = ((int16_t)data[1] << 8) | data[0];
    if(verbose == 1) {
        printf("Debug: Gyroscope Data X: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[0], data[1],buf);
    }

    gyro->gyro_roll = static_cast<double>(buf) / 16.0;

    buf = ((int16_t)data[3] << 8) | data[2];
    if(verbose == 1) {
        printf("Debug: Gyrosscope Data Y: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[2], data[3],buf);
    }

    gyro->gyro_pitch = static_cast<double>(buf) / 16.0;

    buf = ((int16_t)data[5] << 8) | data[4];
    if(verbose == 1) {
        printf("Debug: Gyroscope Data Z: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[4], data[5],buf);
    }

    gyro->gyro_yaw = static_cast<double>(buf) / 16.0;
    return(0);
}

int BNO055::get_euler_angles(euler_angles *euler_angles)
{
    char reg = BNO055_EULER_H_LSB_ADDR;
    if(write(i2cfd, &reg, 1) != 1) {
        printf("Error: I2C write failure for register 0x%02X\n", reg);
        return(-1);
    }

    if(verbose == 1) printf("Debug: I2C read 6 bytes starting at register 0x%02X\n", reg);

    unsigned char data[6] = {0, 0, 0, 0, 0, 0};
    if(read(i2cfd, data, 6) != 6) {
        printf("Error: I2C read failure for register data 0x%02X\n", reg);
        return(-1);
    }

    int16_t buf = ((int16_t)data[1] << 8) | data[0]; 
    if(verbose == 1) {
        printf("Debug: Euler Orientation H: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[0], data[1],buf);
    }

    euler_angles->yaw = static_cast<double>(buf) / 16.0;

    buf = ((int16_t)data[3] << 8) | data[2]; 
    if(verbose == 1) {
        printf("Debug: Euler Orientation R: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[2], data[3],buf);
    }

    euler_angles->roll = static_cast<double>(buf) / 16.0;

    buf = ((int16_t)data[5] << 8) | data[4]; 
    if(verbose == 1) {
        printf("Debug: Euler Orientation P: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[4], data[5],buf);
    }

    euler_angles->pitch = static_cast<double>(buf) / 16.0;
    return(0);
}

int BNO055::get_quaternion(struct quaternion *bnod_ptr)
{
    char reg = BNO055_QUATERNION_DATA_W_LSB_ADDR;
    if(write(i2cfd, &reg, 1) != 1) {
        printf("Error: I2C write failure for register 0x%02X\n", reg);
        return(-1);
    }

    if(verbose == 1) printf("Debug: I2C read 8 bytes starting at register 0x%02X\n", reg);

    unsigned char data[8] = {0};
    if(read(i2cfd, data, 8) != 8) {
        printf("Error: I2C read failure for register data 0x%02X\n", reg);
        return(-1);
    }

    int16_t buf = ((int16_t)data[1] << 8) | data[0]; 
    if(verbose == 1) {
        printf("Debug: Quaternation W: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[0], data[1],buf);
    }

    bnod_ptr->quater_w = (double) buf / 16384.0;

    buf = ((int16_t)data[3] << 8) | data[2]; 
    if(verbose == 1) {
        printf("Debug: Quaternation X: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[2], data[3],buf);
    }

    bnod_ptr->quater_x = (double) buf / 16384.0;

    buf = ((int16_t)data[5] << 8) | data[4]; 
    if(verbose == 1) {
        printf("Debug: Quaternation Y: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[4], data[5],buf);
    }

    bnod_ptr->quater_y = (double) buf / 16384.0;

    buf = ((int16_t)data[7] << 8) | data[6]; 
    if(verbose == 1) {
        printf("Debug: Quaternation Z: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[6], data[7],buf);
    }

    bnod_ptr->quater_z = (double) buf / 16384.0;
    return(0);
}

int BNO055::get_gravity_vector(struct gravity_vector *gravity)
{
    // Get the unit conversion: 1 m/s2 = 100 LSB, 1 mg = 1 LSB.
    char reg = BNO055_UNIT_SEL_ADDR;
    if(write(i2cfd, &reg, 1) != 1) {
        printf("Error: I2C write failure for register 0x%02X\n", reg);
        return(-1);
    }
    char unit_sel;
    if(read(i2cfd, &unit_sel, 1) != 1) {
        printf("Error: I2C read failure for register data 0x%02X\n", reg);
        return(-1);
    }

    double ufact;
    if((unit_sel >> 0) & 0x01) ufact = 1.0;
    else ufact = 100.0;

    // Get the gravity vector data.
    reg = BNO055_GRAVITY_DATA_X_LSB_ADDR;
    if(write(i2cfd, &reg, 1) != 1) {
        printf("Error: I2C write failure for register 0x%02X\n", reg);
        return(-1);
    }

    if(verbose == 1) printf("Debug: I2C read 6 bytes starting at register 0x%02X\n", reg);

    unsigned char data[6] = {0, 0, 0, 0, 0, 0};
    if(read(i2cfd, data, 6) != 6) {
        printf("Error: I2C read failure for register data 0x%02X\n", reg);
        return(-1);
    }

    int16_t buf = ((int16_t)data[1] << 8) | data[0];
    if(verbose == 1) {
        printf("Debug: Gravity Vector H: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[0], data[1],buf);
    }

    gravity->gravity_x = (double) buf / ufact;

    buf = ((int16_t)data[3] << 8) | data[2];
    if(verbose == 1) {
        printf("Debug: Gravity Vector M: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[2], data[3],buf);
    }

    gravity->gravity_y = (double) buf / ufact;

    buf = ((int16_t)data[5] << 8) | data[4];
    if(verbose == 1) {
        printf("Debug: Gravity Vector P: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[4], data[5],buf);
    }

    gravity->gravity_z = (double) buf / ufact;
    return(0);
}

int BNO055::get_linear_accleration(struct linear_acceleration *linear_accel)
{
    // Get the unit conversion: 1 m/s2 = 100 LSB, 1 mg = 1 LSB.
    char reg = BNO055_UNIT_SEL_ADDR;
    if(write(i2cfd, &reg, 1) != 1) {
        printf("Error: I2C write failure for register 0x%02X\n", reg);
        return(-1);
    }
    char unit_sel;
    if(read(i2cfd, &unit_sel, 1) != 1) {
        printf("Error: I2C read failure for register data 0x%02X\n", reg);
        return(-1);
    }

    double ufact;
    if((unit_sel >> 0) & 0x01) ufact = 1.0;
    else ufact = 100.0;

    // Get the linear acceleration data.
    reg = BNO055_LIN_ACC_DATA_X_LSB_ADDR;
    if(write(i2cfd, &reg, 1) != 1) {
        printf("Error: I2C write failure for register 0x%02X\n", reg);
        return(-1);
    }

    if(verbose == 1) printf("Debug: I2C read 6 bytes starting at register 0x%02X\n", reg);

    unsigned char data[6] = {0, 0, 0, 0, 0, 0};
    if(read(i2cfd, data, 6) != 6) {
        printf("Error: I2C read failure for register data 0x%02X\n", reg);
        return(-1);
    }

    int16_t buf = ((int16_t)data[1] << 8) | data[0];
    if(verbose == 1) {
        printf("Debug: Linear Acceleration H: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[0], data[1],buf);
    }

    linear_accel->linear_accel_x = static_cast<double>(buf) / ufact;

    buf = ((int16_t)data[3] << 8) | data[2];
    if(verbose == 1) {
        printf("Debug: Linear Acceleration M: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[2], data[3],buf);
    }

    linear_accel->linear_accel_y = static_cast<double>(buf) / ufact;

    buf = ((int16_t)data[5] << 8) | data[4];
    if(verbose == 1) {
        printf("Debug: Linear Acceleration P: LSB [0x%02X] MSB [0x%02X] INT16 [%d]\n", data[4], data[5],buf);
    }

    linear_accel->linear_accel_z = static_cast<double>(buf) / ufact;
    return(0);
}

int BNO055::set_mode(operation_mode_t mode)
{
    char data[2] = {0};
    data[0] = BNO055_OPR_MODE_ADDR;
    operation_mode_t old_mode = get_mode();

    if(old_mode == mode) {
        return(0); // if new mode is the same
    } else if(old_mode > 0 && mode > 0) {  // switch to "config" first
        data[1] = 0x0;
        if(verbose == 1) printf("Debug: Write opr_mode: [0x%02X] to register [0x%02X]\n", data[1], data[0]);
        if(write(i2cfd, data, 2) != 2) {
            printf("Error: I2C write failure for register 0x%02X\n", data[0]);
            return(-1);
        }

        // Switch time: any->config needs 7ms + small buffer = 10ms.
        usleep(10 * 1000);
    }

    data[1] = mode;

    if(verbose == 1) {
        printf("Debug: Write opr_mode: [0x%02X] to register [0x%02X]\n", data[1], data[0]);
    }

    if(write(i2cfd, data, 2) != 2) {
        printf("Error: I2C write failure for register 0x%02X\n", data[0]);
        return(-1);
    }

    // Switch time: config->any needs 19ms + small buffer = 25ms.
    usleep(25 * 1000);

    if(get_mode() == mode) {
        return(0);
    }

    return(-1);
}

operation_mode_t BNO055::get_mode()
{
    int reg = BNO055_OPR_MODE_ADDR;

    if(write(i2cfd, &reg, 1) != 1) {
        printf("Error: I2C write failure for register 0x%02X\n", reg);
        return(error);
    }

    operation_mode_t data = config;
    if(read(i2cfd, &data, 1) != 1) {
          printf("Error: I2C read failure for register data 0x%02X\n", reg);
          return(error);
    }

    if(verbose == 1) {
        printf("Debug: Operation Mode: [0x%02X]\n", data & mask);
    }

    // Only return the lowest 4 bits.
    return(data);
}

int BNO055::print_mode(int mode)
{
    if(mode < 0 || mode > 12) return(-1);
    
    switch(mode) {
        case 0x00:
            printf("CONFIG\n");
            break;
        case 0x01:
            printf("ACCONLY\n");
            break;
        case 0x02:
            printf("MAGONLY\n");
            break;
        case 0x03:
            printf("GYRONLY\n");
            break;
        case 0x04:
            printf("ACCMAG\n");
            break;
        case 0x05:
            printf("ACCGYRO\n");
            break;
        case 0x06:
            printf("MAGGYRO\n");
            break;
        case 0x07:
            printf("AMG\n");
            break;
        case 0x08:
            printf("IMU\n");
            break;
        case 0x09:
            printf("COMPASS\n");
            break;
        case 0x0A:
            printf("M4G\n");
            break;
        case 0x0B:
            printf("NDOF_FMC_OFF\n");
            break;
        case 0x0C:
            printf("NDOF_FMC\n");
            break;
    }

    return(0);
}

int BNO055::set_power(power_t pwr_mode)
{
    char data[2] = {0};

    // Check what operational mode we are in.
    operation_mode_t old_mode = get_mode();

    // If ops mode wasn't config, switch to "CONFIG" mode first.
    if(old_mode > 0) {
        data[0] = BNO055_OPR_MODE_ADDR;
        data[1] = 0x0;
        if(verbose == 1) printf("Debug: Write opr_mode: [0x%02X] to register [0x%02X]\n", data[1], data[0]);
        if(write(i2cfd, data, 2) != 2) {
                printf("Error: I2C write failure for register 0x%02X\n", data[0]);
                return(-1);
        }
          
          usleep(30 * 1000);
    } // now we are in config mode

    // Set the new power mode.
    data[0] = BNO055_PWR_MODE_ADDR;
    data[1] = pwr_mode;
    if(verbose == 1) printf("Debug: Write opr_mode: [0x%02X] to register [0x%02X]\n", data[1], data[0]);
    if(write(i2cfd, data, 2) != 2) {
            printf("Error: I2C write failure for register 0x%02X\n", data[0]);
            return(-1);
    }

     usleep(30 * 1000);

    // If ops mode wasn't config, switch back to original ops mode.
    if(old_mode > 0) {
        data[0] = BNO055_OPR_MODE_ADDR;
        data[1] = old_mode;

        if(verbose == 1) {
                printf("Debug: Write opr_mode: [0x%02X] to register [0x%02X]\n", data[1], data[0]);
        }

        if(write(i2cfd, data, 2) != 2) {
            printf("Error: I2C write failure for register 0x%02X\n", data[0]);
            return(-1);
        }

        usleep(30 * 1000);
    }  // now the previous mode is back

    if(get_power() == pwr_mode) return(0);
    else return(-1);
}

int BNO055::get_power()
{
     int reg = BNO055_PWR_MODE_ADDR;
     if(write(i2cfd, &reg, 1) != 1) {
          printf("Error: I2C write failure for register 0x%02X\n", reg);
          return(-1);
    }

    unsigned int data = 0;
    if(read(i2cfd, &data, 1) != 1) {
        printf("Error: I2C read failure for register data 0x%02X\n", reg);
        return(-1);
    }

    if(verbose == 1) {
        printf("Debug:     Power Mode: [0x%02X] 2bit [0x%02X]\n", data, data & 0x03);
    }

    return(data & 0x03);  // only return the lowest 2 bits
}

int BNO055::print_power(int mode)
{
    if(mode < 0 || mode > 2) {
        return(-1);
    }

    switch(mode) {
        case 0x00:
            printf("NORMAL\n");
            break;
        case 0x01:
            printf("LOW\n");
            break;
        case 0x02:
            printf("SUSPEND\n");
            break;
    }
    return(0);
}

int BNO055::get_sstat()
{
    int reg = BNO055_SYS_STAT_ADDR;
    if(write(i2cfd, &reg, 1) != 1) {
        printf("Error: I2C write failure for register 0x%02X\n", reg);
        return(-1);
    }

    unsigned int data = 0;
    if(read(i2cfd, &data, 1) != 1) {
        printf("Error: I2C read failure for register data 0x%02X\n", reg);
        return(-1);
    }

    if(verbose == 1) {
        printf("Debug:  System Status: [0x%02X]\n", data);
    }

    return(data);
}

int BNO055::print_sstat(int stat_code)
{
    if(stat_code < 0 || stat_code > 6) {
        return(-1);
    }
    
    switch(stat_code) {
        case 0x00:
            printf("Idle\n");
            break;
        case 0x01:
            printf("System Error\n");
            break;
        case 0x02:
            printf("Initializing Peripherals\n");
            break;
        case 0x03:
            printf("System Initalization\n");
            break;
        case 0x04:
            printf("Executing Self-Test\n");
            break;
        case 0x05:
            printf("Sensor running with fusion algorithm\n");
            break;
        case 0x06:
            printf("Sensor running without fusion algorithm\n");
            break;
    }
    return(0);
}

int BNO055::get_serror()
{
    int reg = BNO055_SYS_ERR_ADDR;
    if(write(i2cfd, &reg, 1) != 1) {
        printf("Error: I2C write failure for register 0x%02X\n", reg);
        return(-1);
    }

    unsigned int data = 0;
    if(read(i2cfd, &data, 1) != 1) {
        printf("Error: I2C read failure for register data 0x%02X\n", reg);
        return(-1);
    }

    if(verbose == 1) {
        printf("Debug:  System Eroor: [0x%02X]\n", data);
    }

    return(data);
}

int BNO055::print_serror(int error_code)
{
    if(error_code < 0 || error_code > 6) {
        return(-1);
    }

    switch(error_code) {
        case sys_error_t::PERIPHERAL_INIT_ERROR:
            printf("PERIPHERAL_INIT_ERROR\n");
            break;
        case sys_error_t::SYSTEM_INIT_ERROR:
            printf("SYSTEM_INIT_ERROR\n");
            break;
        case sys_error_t::SELF_TEST_FAILED:
            printf("SELF_TEST_FAILED\n");
            break;
        case sys_error_t::REG_VALUE_OUT_OF_RANGE:
            printf("REG_VALUE_OUT_OF_RANGE\n");
            break;
        case sys_error_t::REG_ADDR_OUT_OF_RANGE:
            printf("REG_ADDR_OUT_OF_RANGE\n");
            break;
        case sys_error_t::REG_WRITE_ERROR :
            printf("REG_WRITE_ERROR \n");
            break;
        case sys_error_t::LOW_POWER_MODE_NOT_AVALIBLE:
            printf("LOW_POWER_MODE_NOT_AVALIBLE\n");
            break;
        case sys_error_t::ACC_POWER_MODE_NOT_AVALIBLE:
            printf("ACC_POWER_MODE_NOT_AVALIBLE\n");
            break;
        case sys_error_t::FUSION_CONFIG_EORROR:
            printf("FUSION_CONFIG_EORROR\n");
            break;
        case sys_error_t::SYS_CONFIG_ERROR:
            printf("SYS_CONFIG_ERROR\n");
            break;
    }
    return(0);
}


int BNO055::get_remap(char mode)
{
    int reg;

    if(mode == 'c') {
        reg = BNO055_AXIS_MAP_CONFIG_ADDR;
    } else if(mode == 's') {
        reg = BNO055_AXIS_MAP_SIGN_ADDR;
    } else {
        printf("Error: Unknown remap function mode %c.\n", mode);
        exit(-1);
    }

    if(write(i2cfd, &reg, 1) != 1) {
        printf("Error: I2C write failure for register 0x%02X\n", reg);
        return(-1);
    }

    unsigned int data = 0;
    if(read(i2cfd, &data, 1) != 1) {
        printf("Error: I2C read failure for register data 0x%02X\n", reg);
        return(-1);
    }

    if(verbose == 1) {
        printf("Debug: Axis Remap '%c': [0x%02X]\n", mode, data);
    }

    return(data);
}

int BNO055::print_remap_conf(int mode)
{

    if(mode != 0x09 && mode != 0x18 &&
       mode != 0x24 && mode != 0x36) {
        return(-1);
    }

    switch(mode) {
        case 0x09:     // 0 0 | 1 0 | 0 1
            printf("X<>Z Y==Y Z<>X (UNE)\n");
            break;
        case 0x18:     // 0 1 | 0 0 | 1 0 
            printf("X<>Y Y<>X Z==Z (NEU)\n");
            break;
        case 0x24:     // 0 1 | 1 0 | 0 0
            printf("X==X Y==Y Z==Z (ENU)\n");
            break;
        case 0x36:     // 1 0 | 0 1 | 0 0
            printf("X==X Y<>Z Z<>Y (EUN)\n");
            break;
    }
    return(0);
}

int BNO055::print_remap_sign(int mode)
{
    if(mode < 0 || mode > 7) {
        return(-1);
    }

    switch(mode) {
        case 0x00:
            printf("X+ Y+ Z+\n");
            break;
        case 0x01:
            printf("X+ Y+ Z-\n");
            break;
        case 0x02:
            printf("X+ Y- Z+\n");
            break;
        case 0x03:
            printf("X+ Y- Z-\n");
            break;
        case 0x04:
            printf("X- Y+ Z+\n");
            break;
        case 0x05:
            printf("X- Y+ Z-\n");
            break;
        case 0x06:
            printf("X- Y- Z+\n");
            break;
        case 0x07:
            printf("X- Y- Z-\n");
            break;
    }
    return(0);
}

int BNO055::set_page0()
{
    char data[2] = {0};
    data[0] = BNO055_PAGE_ID_ADDR;
    data[1] = 0x0;
    if(verbose == 1) {
        printf("Debug: write page-ID: [0x%02X] to register [0x%02X]\n", data[1], data[0]);
    }

    if(write(i2cfd, data, 2) != 2) {
        printf("Error: I2C write failure for register 0x%02X\n", data[0]);
        return(-1);
    }
    return(0);
}

int BNO055::set_page1()
{
    char data[2] = {0};
    data[0] = BNO055_PAGE_ID_ADDR;
    data[1] = 0x1;
    if(verbose == 1) printf("Debug: write page-ID: [0x%02X] to register [0x%02X]\n", data[1], data[0]);
    if(write(i2cfd, data, 2) != 2) {
        printf("Error: I2C write failure for register 0x%02X\n", data[0]);
        return(-1);
    }
    return(0);
}

int BNO055::get_clk_src()
{
    char reg = BNO055_SYS_TRIGGER_ADDR;
    if(write(i2cfd, &reg, 1) != 1) {
        printf("Error: I2C write failure for register 0x%02X\n", reg);
        set_page0();
        return(-1);
    }

    char data;
    if(read(i2cfd, &data, 1) != 1) {
        printf("Error: I2C read failure for register data 0x%02X\n", reg);
        set_page0();
        return(-1);
    }

    if(verbose == 1) printf("Debug: CLK_SEL bit-7 in register %d: [%d]\n", reg, (data & 0b10000000) >> 7);
    return (data & 0b10000000) >> 7; // system calibration status
}

void BNO055::print_clk_src()
{
    int src = get_clk_src();
    if(src == 0) {
        printf("Internal Clock (default)\n");
    } else if(src == 1) {
        printf("External Clock\n");
    }else if(src == -1) {
        printf("Clock Reading error\n");
    }
}

int BNO055::get_accel_config(accel_config *bnoc_ptr)
{
    set_page1();
    char reg = BNO055_ACC_CONFIG_ADDR;
    if(write(i2cfd, &reg, 1) != 1) {
        printf("Error: I2C write failure for register 0x%02X\n", reg);
        set_page0();
        return(-1);
    }

    char data;
    if(read(i2cfd, &data, 1) != 1) {
        printf("Error: I2C read failure for register data 0x%02X\n", reg);
        set_page0();
        return(-1);
    }

    bnoc_ptr->range   = (data & 0b00000011) >> 2; // accel range
    if(verbose == 1) {
        printf("Debug:       accelerometer range: [%d]\n", bnoc_ptr->pwr_mode);
    }

    bnoc_ptr->bandwidth = (data & 0b00011100) >> 4; // accel bandwidth
    if(verbose == 1) {
        printf("Debug:   accelerometer bandwidth: [%d]\n", bnoc_ptr->bandwidth);
    }

    bnoc_ptr->pwr_mode = (data & 0b11100000) >> 6; // accel power mode
    if(verbose == 1) {
        printf("Debug:  accelerometer power mode: [%d]\n", bnoc_ptr->pwr_mode);
    }


    reg = BNO055_ACC_SLEEP_CONFIG_ADDR;
    if(write(i2cfd, &reg, 1) != 1) {
        printf("Error: I2C write failure for register 0x%02X\n", reg);
        set_page0();
        return(-1);
    }

    data = 0;
    if(read(i2cfd, &data, 1) != 1) {
        printf("Error: I2C read failure for register data 0x%02X\n", reg);
        set_page0();
        return(-1);
    }

    bnoc_ptr->sleep_mode = (data & 0b00000011) >> 2; // accel sleep mode
    if(verbose == 1) {
        printf("Debug:  accelerometer sleep mode: [%d]\n", bnoc_ptr->sleep_mode);
    }

    bnoc_ptr->sleep_duration = (data & 0b00011100) >> 4; // accel sleep duration
    if(verbose == 1) {
        printf("Debug:   accelerometer sleep dur: [%d]\n", bnoc_ptr->sleep_duration);
    }

    set_page0();
    return(0);
}

void BNO055::print_accel_config(accel_config *bnoc_ptr)
{
    printf("Accelerometer  Power = ");
    switch(bnoc_ptr->pwr_mode) {
        case 0:
            printf("NORMAL\n");
            break;
        case 1:
            printf("SUSPEND\n");
            break;
        case 2:
            printf("LOW POWER1\n");
            break;
        case 3:
            printf("STANDBY\n");
            break;
        case 4:
            printf("LOW POWER2\n");
            break;
        case 5:
            printf("DEEP SUSPEND\n");
            break;
    }
    printf("Accelerometer Bandwidth = ");
    switch(bnoc_ptr->bandwidth) {
      case 0:
            printf("7.81Hz\n");
            break;
        case 1:
            printf("15.63Hz\n");
            break;
        case 2:
            printf("31.25Hz\n");
            break;
        case 3:
            printf("62.5Hz\n");
            break;
        case 4:
            printf("125Hz\n");
            break;
        case 5:
            printf("250Hz\n");
            break;
        case 6:
            printf("500Hz\n");
            break;
        case 7:
            printf("1KHz\n");
            break;
    }
    printf("Accelerometer GRange = ");
    switch(bnoc_ptr->range) {
        case 0:
            printf("2G\n");
            break;
        case 1:
            printf("4G\n");
            break;
        case 2:
            printf("8G\n");
            break;
        case 3:
            printf("16G\n");
            break;
    }
    printf("Accelerometer Sleep = ");
    switch(bnoc_ptr->sleep_mode) {
        case 0:
            printf("event-driven, ");
            break;
        case 1:
            printf("equidistant sampling, ");
            break;
    }
    if(bnoc_ptr->sleep_duration < 6) printf("0.5ms\n");
    else switch(bnoc_ptr->sleep_duration) {
        case 6:
            printf("1ms\n");
            break;
        case 7:
            printf("2ms\n");
            break;
        case 8:
            printf("4ms\n");
            break;
        case 9:
            printf("6ms\n");
            break;
        case 10:
            printf("10ms\n");
            break;
        case 11:
            printf("25ms\n");
            break;
        case 12:
            printf("50ms\n");
            break;
        case 13:
            printf("100ms\n");
            break;
        case 14:
            printf("500ms\n");
            break;
        case 15:
            printf("1s\n");
            break;
    }
}
