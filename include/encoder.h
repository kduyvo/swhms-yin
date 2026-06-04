#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include "midiStructs.h"
#include "helperIO.h"
#include "midiFun.h"
#include <string.h>

enum fdEncoder {
    encoderInPacket = 100,
    encoderOut
};

#endif
