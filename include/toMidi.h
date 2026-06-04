#ifndef TO_MIDI_H
#define TO_MIDI_H

#include "helperIO.h"
#include "midiStructs.h"
#include "volumeModulation.h"
#include "debounce.h"
#include "encoder.h"
#include "opcode.h"
// #include <poll.h>

enum fdToMidi{
    toMidiInFrequency = 100,
    toMidiInAmplitude,
    toMidiInOpcode,
    toMidiInInstrument,
    toMidiOutInfo,
    toMidiOutCommand,
};

void mainProcess();
void selectorProcess(pid_t child[], int pipefd[][2]);
void autotuneProcess(pid_t child[], int pipefd[][2]);
void liveProcess(pid_t child[], int pipefd[][2]);
void volumeModulationProcess(pid_t child[], int pipefd[][2]);
void presetModulationProcess(pid_t child[], int pipefd[][2]);
void schedulerProcess(pid_t child[], int pipefd[][2]);
void debounceProcess(pid_t child[], int pipefd[][2]);
void encoderProcess(pid_t child[], int pipefd[][2]);

#endif
