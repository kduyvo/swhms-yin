#ifndef COMMAND_HISTORY_H
#define COMMAND_HISTORY_H

#include "string.h"
#include <stdint.h>

#define ROWS 27
#define COLUMNS 15
#define TOTAL_CAPACITY (ROWS * COLUMNS)

struct commandHistory {
   uint8_t data[TOTAL_CAPACITY];
   unsigned int head;
   unsigned int size;
};

void commandHistoryInit(void);
void commandHistoryAdd(uint8_t data);
uint8_t commandHistoryGet(unsigned int index);
unsigned int commandHistoryGetSize();

#endif