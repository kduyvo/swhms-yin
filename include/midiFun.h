#ifndef MIDI_FUN
#define MIDI_FUN
#include <stdint.h>
#include <stdio.h>
#include "helperIO.h"

void note_on(int fd, uint8_t channel, uint8_t note, uint8_t velocity);
void note_off(int fd, uint8_t channel, uint8_t note, uint8_t velocity);
void control_change(int fd, uint8_t channel, uint8_t control_number, uint8_t value);
void program_change(int fd, uint8_t channel, uint8_t program_number);
void pitch_change(int fd, uint8_t channel, uint16_t pitch);

#define NOTE_ON 0x90
#define NOTE_OFF 0x80
#define CONTROL_CHANGE 0xB0
#define PITCH_CHANGE 0xE0
#define PROGRAM_CHANGE 0xC0

#endif 