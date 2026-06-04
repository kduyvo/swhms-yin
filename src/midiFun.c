#include "midiFun.h"

void note_on(int fd, uint8_t channel, uint8_t note, uint8_t velocity){
    channel--;
    if (
        channel > 127   || 
        note > 127      ||
        velocity > 127 ) 
        {
        fprintf(stderr, "bad note_on()\n");
    }
    uint8_t buffer[3] = {NOTE_ON | channel, note, velocity};
    writeIO(fd, buffer, sizeof buffer);
}
void note_off(int fd, uint8_t channel, uint8_t note, uint8_t velocity){
    channel--;
    if (
        channel > 127   || 
        note > 127      ||
        velocity > 127 ) {
        fprintf(stderr, "bad note_off()\n");
    }
    uint8_t buffer[3] = {NOTE_OFF | channel, note, velocity};
    writeIO(fd, buffer, sizeof buffer);
}
void control_change(int fd, uint8_t channel, uint8_t control_number, uint8_t value){
    channel--;
    if (
        channel > 127       || 
        control_number > 127||
        value > 127 ) {
        fprintf(stderr, "bad control_change()\n");
    }
    uint8_t buffer[3] = {CONTROL_CHANGE | channel, control_number, value};
    writeIO(fd, buffer, sizeof buffer);
}
void program_change(int fd, uint8_t channel, uint8_t program_number){
    channel--;
    if (
        channel > 127       || 
        program_number > 127) {
        fprintf(stderr, "bad program_change()\n");
    }
    uint8_t buffer[2] = {PROGRAM_CHANGE | channel, program_number};
    writeIO(fd, buffer, sizeof buffer);
}
void pitch_change(int fd, uint8_t channel, uint16_t pitch){
    channel--;
    if (
        channel > 127       || 
        pitch > 16383) {
        fprintf(stderr, "bad pitch()\n");
    }
    uint8_t lsb = pitch & 0x7F;
    uint8_t msb = (pitch >> 7) & 0x7F;
    uint8_t buffer[3] = {PITCH_CHANGE | channel, lsb, msb};
    writeIO(fd, buffer, sizeof buffer);
}

