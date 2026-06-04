#ifndef SWHMS_H
#define SWHMS_H

#include "frameProcessor.h"
#include "frameSettings.h"
#include "helperIO.h"
#include "toMidi.h"
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

void lpfProcess(pid_t child[], int pipefd[][2]);
void hpfProcess(pid_t child[], int pipefd[][2]);
void ringBufferProcess(pid_t child[], int pipefd[][2]);
void frameProcessorProcess(pid_t child[], int pipefd[][2]);
void toMidiProcess(pid_t child[], int pipefd[][2]);

enum fdswhms {
   swhmsInAudio = 200,
   swhmsInOpcode,
   swhmsInInstrument,
   swhmsOutInfo,
   swhmsOutCommand
};

#endif // SWHMS_H