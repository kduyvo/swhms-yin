#ifndef I2C_H
#define I2C_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <arpa/inet.h>
#include <string.h>

int i2cOpen();
void i2cWrite16(int fd, uint8_t address, uint8_t reg, uint16_t data);
void i2cRead16(int fd, uint8_t address, uint8_t reg, uint16_t *data);
uint16_t i2cRead(int fd, uint8_t reg);
#endif