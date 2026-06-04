#ifndef INSTRUMENTS_H
#define INSTRUMENTS_H

#include "stdint.h"
#include <string.h>
#include "helperIO.h"
#include <stdio.h>

struct instrument {
    char name[32];
    uint8_t midiNum;
    uint8_t velocity;
    uint8_t cc11;
};

void presetInit(void);
char* getName(uint8_t midiNum);
void addPreset(char name[32], uint8_t midiNum, uint8_t velocity, uint8_t cc11);
uint8_t getMidiNum(uint8_t index);
uint8_t parseStrToU8(char* s);
uint8_t getVelocity(uint8_t index);
uint8_t getCC11(uint8_t index);
#endif