#ifndef WRAPPER_H
#define WRAPPER_H

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
// #include <ncurses.h>
#include "alsa.h"
#include "commandHistory.h"
#include "debugScreen.h"
#include "getHW.h"
#include "helperIO.h"
#include "midiStructs.h"
#include "opcode.h"
#include "swhms.h"
#include <signal.h>

void arecordProcess(pid_t child[], int pipefd[][2]);
void swhmsProcess(pid_t child[], int pipefd[][2]);
void debugScreenProcess(pid_t child[], int pipefd[][2]);
void uartProcess(pid_t child[], int pipefd[][2]);
void batteryProcess(pid_t child[], int pipefd[][2]);
void keypadProcess(pid_t child[], int pipefd[][2]);
void alsaProcess(pid_t child[], int pipefd[][2]);

#endif
