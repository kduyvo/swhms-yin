#ifndef VOLUME_MODULATION
#define VOLUME_MODULATION

#include "helperIO.h"
#include "instruments.h"
#include "midiStructs.h"
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define FLOOR_AMPLITUDE 1000
#define CEIL_AMPLITUDE 16000
#define RANGE (CEIL_AMPLITUDE - FLOOR_AMPLITUDE)

#endif
