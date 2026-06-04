#ifndef _GET_HW_H
#define _GET_HW_H

#include "alsa.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct alsa_list {
   struct alsa_device devices[32];
   int size;
};

int getAudioHW(int *destCard, int *destDevice);

struct alsa_list getMidiDevices(void);

void printList(struct alsa_list *list);

#endif
