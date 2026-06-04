#ifndef DEBOUNCE_H
#define DEBOUNCE_H

#include "helperIO.h"
#include "midiStructs.h"
#include "string.h"
#include <fcntl.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define THRESHOLD 4
#define THRESHOLD_LARGE 30
#define MAX_SEMITONE_CHANGE 6

enum fdDebounce { debounceInPacket = 100, debounceOut1, debounceOut2 };

#endif
