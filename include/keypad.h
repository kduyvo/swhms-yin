#ifndef _KEYPAD_H
#define _KEYPAD_H

#include "helperIO.h"
#include "i2c.h"

#define ADDRESS 0x20
#define IODIRA 0x00
#define IODIRB 0x01
#define IPOLA 0x02
#define IPOLB 0x03
#define GPINTENA 0x04
#define GPINTENB 0x05
#define DEFVALA 0x06
#define DEFVALB 0x07
#define INTCONA 0x08
#define INTCONB 0x09
#define IOCON1 0x0A
#define IOCON2 0x0B
#define GPPUA 0x0C
#define GPPUB 0x0D
#define INTFA 0x0E
#define INTFB 0x0F
#define INTCAPA 0x10
#define INTCAPB 0x11
#define GPIOA 0x12
#define GPIOB 0x13
#define OLATA 0x14
#define OLATB 0x15

#define BUTTONS 16
#define THRESHOLD 5

/* function prototypes  */

void keyPadInit(void);
void keyPadStateMachine(int fd);

#endif
