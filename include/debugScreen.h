#ifndef DEBUG_SCREEN
#define DEBUG_SCREEN

#include <fcntl.h>
#include <ncurses.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "alsa.h"
#include "battery.h"
#include "commandHistory.h"
#include "getHW.h"
#include "helperIO.h"
#include "midiStructs.h"
#include "opcode.h"
#include "swhms.h"
#include <signal.h>
enum fdDebugScreen {
   debugScreenInInfo = 300,
   debugScreenInMIDI,
   debugScreenOutOpcode,
   debugScreenOutInstrument,
   debugScreenOutMIDI,
   debugScreenOutInfo,
   debugScreenInBatInfo,
   debugScreenInKeypad
};

#define COMMAND_HISTORY_LABEL_Y_POS 1

#define COMMAND_HISTORY_X_POS 9
#define COMMAND_HISTORY_Y_POS (COMMAND_HISTORY_LABEL_Y_POS + 2)
#define COMMAND_HISTORY_HEIGHT 10
#define COMMAND_HISTORY_WIDTH 81

#define LEGEND_X_POS 73
#define LEGEND_Y_POS (COMMAND_HISTORY_HEIGHT + COMMAND_HISTORY_Y_POS + 1)

#define MENU_X_POS 65
#define MENU_Y_POS (COMMAND_HISTORY_HEIGHT + COMMAND_HISTORY_Y_POS + 1)

#define PROMPT_Y_POS 28
#define PROMPT_BUFFER_SIZE (COMMAND_HISTORY_WIDTH - 3)

#define BATTERY_X_POS 40
#define BATTERY_Y_POS (COMMAND_HISTORY_HEIGHT + COMMAND_HISTORY_Y_POS + 1)

#define INFO_X_POS 9
#define INFO_Y_POS (COMMAND_HISTORY_HEIGHT + COMMAND_HISTORY_Y_POS + 1)

void printCommandHistory(void);

void printPrompt(void);
void printLegend(void);

void printInfo(struct processedPacket info);

void handleInput(void);

void parseCommand(char buffer[PROMPT_BUFFER_SIZE]);

void handleModeFCommand(char *arg);

void printHistoryLabel(void);

void handleModeACommand(char *arg);

void handleInstrumentCommand(char *arg);

void historyPrintByte(uint8_t data);

void printBatteryLabel(void);

void printBatteryInfo(struct batteryInfo batInfo);

void printMenu(void);

void processButton(uint8_t button);

struct buttonCallback {
   void (*function)(char *);
   char *argument;
};

#endif