#ifndef FRAME_PROCESSOR_H
#define FRAME_PROCESSOR_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "helperIO.h"
#include "frameSettings.h"
#include <poll.h>

enum fdFrameProcessor{
    frameProcessorInFrame = 100,
    frameProcessorOutFrequency,
    frameProcessorOutAmplitude,
};

void mainProcess(int pipefd [][2]);
void yinProcess(pid_t child[], int pipefd[][2]);
void envelopeProcess(pid_t child[], int pipefd[][2]);

#endif //FRAME_PROCESSOR_H