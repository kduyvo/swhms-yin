#ifndef BATTERY_H
#define BATTERY_H

#include "i2c.h"

#include "helperIO.h"

#define DEVICE_ADDRESS 0x41

#define REG_CALIBRATION 0x05
#define REG_CONFIG 0x00
#define REG_SHUNT_VOLTAGE 0x01
#define REG_BUS_VOLTAGE 0x02
#define REG_POWER 0x03
#define REG_CURRENT 0x04

#define BUS_VOLTAGE_RANGE_16V 0x00
#define BUS_VOLTAGE_RANGE_32V 0x01
#define GAIN_DIV_8_320MV 0x03
#define GAIN_DIV_2_80MV 0x01
#define ADC_RES_12BIT_32S 0x0D
#define SANDBVOLT_CONTINUOUS 0x07

void setCalibration(void);

double getBusVoltage(void);

double getCurrent_mA(void);

double getShuntVoltage_mV(void);

double getPower_W(void);

struct batteryInfo {
   double busVoltage_V;
   double shuntVoltage_V;
   double current_A;
   double power_W;
   double percent;
};

#endif