#include "battery.h"
/* CALIBRATION */
uint8_t address = 0x41;
uint16_t cal_value = 26868;
double current_lsb = 0.1524;
double power_lsb = 0.003048;

int fd;

int main(void)
{
   fd = i2cOpen();
   struct batteryInfo bat = {0};
   setCalibration();
   double p;
   while (1)
   {
      bat.busVoltage_V = getBusVoltage();
      bat.shuntVoltage_V = getShuntVoltage_mV() / 1000;
      bat.current_A = getCurrent_mA() / 1000;
      bat.power_W = getPower_W();
      p = (bat.busVoltage_V - 9.0) / 3.30 * 100;
      if (p > 100)
         p = 100;
      else if (p < 0)
         p = 0;
      bat.percent = p;
      writeIO(STDOUT_FILENO, &bat, sizeof bat);
      sleep(1);
   }
}

void setCalibration(void)
{
   i2cWrite16(fd, DEVICE_ADDRESS, REG_CALIBRATION, cal_value);
   uint16_t config =
       (BUS_VOLTAGE_RANGE_16V << 13) | (GAIN_DIV_2_80MV << 11) |
       (ADC_RES_12BIT_32S << 7) | (ADC_RES_12BIT_32S << 3) |
       (SANDBVOLT_CONTINUOUS);
   i2cWrite16(fd, DEVICE_ADDRESS, REG_CONFIG, config);
}

double getBusVoltage(void)
{
   i2cWrite16(fd, DEVICE_ADDRESS, REG_CALIBRATION, cal_value);
   uint16_t busRead;
   i2cRead16(fd, DEVICE_ADDRESS, REG_BUS_VOLTAGE, &busRead);
   return ((double)(busRead >> 3)) * 0.004;
}

double getCurrent_mA(void)
{
   int16_t val;
   i2cWrite16(fd, DEVICE_ADDRESS, REG_CALIBRATION, cal_value);
   i2cRead16(fd, DEVICE_ADDRESS, REG_CURRENT, (uint16_t *)&val);
   return ((double)val) * current_lsb;
}

double getShuntVoltage_mV(void)
{
   int16_t val;
   i2cWrite16(fd, DEVICE_ADDRESS, REG_CALIBRATION, cal_value);
   i2cRead16(fd, DEVICE_ADDRESS, REG_CURRENT, (uint16_t *)&val);
   return ((double)val) * 0.01;
}

double getPower_W(void)
{
   int16_t val;
   i2cWrite16(fd, DEVICE_ADDRESS, REG_CALIBRATION, cal_value);
   i2cRead16(fd, DEVICE_ADDRESS, REG_POWER, (uint16_t *)&val);
   return val * power_lsb;
}
