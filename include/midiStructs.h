#ifndef MIDI_STRUCTS_H
#define MIDI_STRUCTS_H

#include "stdint.h"

#define PITCH_BEND_CENTER 8192

struct modeFreqOut {
    uint8_t noteNum;
    uint16_t pitchBend;
};

struct modeModOut{
    uint8_t midiNum;
    uint8_t velocity;
    uint8_t cc11;
};

struct processedPacket {
    uint8_t opcode;
    struct modeFreqOut data1;
    struct modeModOut data2;
};

#endif