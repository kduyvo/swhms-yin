#include "debugScreen.h"

static volatile sig_atomic_t stop = 0;

static void cleanup_and_exit(int) { stop = 1; }

uint8_t opcode = 0;
const struct buttonCallback callback[16] = {
   {handleModeFCommand, "toggle"}, {handleInstrumentCommand, "0"},
   {handleInstrumentCommand, "1"}, {handleInstrumentCommand, "2"},
   {handleInstrumentCommand, "3"}, {handleInstrumentCommand, "0"},
   {handleInstrumentCommand, "0"}, {handleInstrumentCommand, "0"}, // Not used
   {handleInstrumentCommand, "0"}, {handleInstrumentCommand, "4"},
   {handleInstrumentCommand, "5"}, {handleInstrumentCommand, "6"},
   {handleInstrumentCommand, "7"}, {handleModeACommand, "toggle"},
   {handleInstrumentCommand, "0"}, // Not used
   {handleInstrumentCommand, "0"}, // Not used
};

WINDOW *histwin;
WINDOW *histlabel;
WINDOW *batterywin;
WINDOW *infowin;
WINDOW *legendwin;
WINDOW *promptwin;
WINDOW *selectmidi;

int main(void) {
   signal(SIGINT, cleanup_and_exit);
   signal(SIGTERM, cleanup_and_exit);
   // commandHistoryInit();
   initscr(); // start ncurses
   // newterm();
   start_color();
   cbreak();    // disable line buffering
   noecho();    // don't echo typed keys
   curs_set(0); // hide cursor (optional)
   histwin = newwin(COMMAND_HISTORY_HEIGHT, COMMAND_HISTORY_WIDTH,
                    COMMAND_HISTORY_Y_POS, COMMAND_HISTORY_X_POS);
   histlabel = newwin(1, COMMAND_HISTORY_WIDTH, COMMAND_HISTORY_LABEL_Y_POS,
                      COMMAND_HISTORY_X_POS);
   infowin = newwin(8, 30, INFO_Y_POS, INFO_X_POS);
   legendwin = newwin(6, 17, LEGEND_Y_POS, LEGEND_X_POS);
   batterywin = newwin(8, 21, BATTERY_Y_POS, BATTERY_X_POS);
   selectmidi = newwin(15, 25, MENU_Y_POS, MENU_X_POS);
   promptwin =
      newwin(1, COMMAND_HISTORY_WIDTH, PROMPT_Y_POS, COMMAND_HISTORY_X_POS);

   nodelay(promptwin, TRUE);
   keypad(promptwin, TRUE); // enable arrow keys, etc.

   scrollok(histwin, TRUE);
   idlok(histwin, FALSE);
   leaveok(infowin, TRUE);
   leaveok(batterywin, TRUE);
   leaveok(histwin, TRUE);
   leaveok(legendwin, TRUE);

   init_pair(1, COLOR_GREEN, COLOR_RED);
   init_pair(2, COLOR_BLUE, COLOR_YELLOW);
   init_pair(3, COLOR_RED, COLOR_GREEN);
   init_pair(4, COLOR_WHITE, COLOR_CYAN);
   init_pair(5, COLOR_WHITE, COLOR_MAGENTA);

   printLegend();
   struct processedPacket info = {0};
   struct batteryInfo batInfo = {0};
   uint8_t commandByte;
   uint8_t button;
   printBatteryLabel();
   printInfo(info);
   printBatteryInfo(batInfo);
   printPrompt();
   printHistoryLabel();
   wnoutrefresh(infowin);
   // int dirty = 0;
   while (stop != 1) {

      while (readIOPoll(debugScreenInKeypad, &button, sizeof button, 0)) {
         processButton(button);
      }

      while (readIOPoll(debugScreenInInfo, &info, sizeof info, 0)) {
         writeIO(debugScreenOutInfo, &info, sizeof info);
         printInfo(info);
         // dirty = 1;
      }

      while (
         readIOPoll(debugScreenInMIDI, &commandByte, sizeof commandByte, 0)) {
         historyPrintByte(commandByte);
         writeIO(debugScreenOutMIDI, &commandByte, sizeof commandByte);
         // dirty = 1;
      }

      while (readIOPoll(debugScreenInBatInfo, &batInfo, sizeof batInfo, 0)) {
         printBatteryInfo(batInfo);
         // dirty = 1;
      }
      printMenu();
      handleInput();
      doupdate();
      // dirty = 0;
      napms(16);
   }
   endwin(); // restore terminal
   exit(0);
}

void printHistoryLabel(void) {
   mvwprintw(histlabel, 0, 0, "UART MIDI Output");
   wnoutrefresh(histlabel);
}

void printPrompt(void) {
   mvwprintw(promptwin, 0, 0, "%%: ");
   wnoutrefresh(promptwin);
   return;
}

void printBatteryLabel(void) {
   mvwprintw(batterywin, 0, 0, "Battery Information");
   mvwprintw(batterywin, 2, 0, "  PSU V: ");
   mvwprintw(batterywin, 3, 0, "Shunt V: ");
   mvwprintw(batterywin, 4, 0, " Load V: ");
   mvwprintw(batterywin, 5, 0, "  PSU I: ");
   mvwprintw(batterywin, 6, 0, "  Power: ");
   mvwprintw(batterywin, 7, 0, "Percent: ");
   wnoutrefresh(batterywin);
}

void printBatteryInfo(struct batteryInfo batInfo) {
   mvwprintw(batterywin, 2, 10, "%9.3f V",
             batInfo.busVoltage_V + batInfo.shuntVoltage_V);
   mvwprintw(batterywin, 3, 10, "%9.6f V", batInfo.shuntVoltage_V);
   mvwprintw(batterywin, 4, 10, "%9.3f V", batInfo.busVoltage_V);
   mvwprintw(batterywin, 5, 10, "%9.6f A", batInfo.current_A);
   mvwprintw(batterywin, 6, 10, "%9.3f W", batInfo.power_W);
   mvwprintw(batterywin, 7, 10, "%9.1f %%", batInfo.percent);

   wnoutrefresh(batterywin);
}

void historyPrintByte(uint8_t data) {
   attr_t color = A_NORMAL;

   switch (data) {
      case NOTE_ON:
         color = COLOR_PAIR(1);
         break;
      case NOTE_OFF:
         color = COLOR_PAIR(2);
         break;
      case CONTROL_CHANGE:
         color = COLOR_PAIR(3);
         break;
      case PITCH_CHANGE:
         color = COLOR_PAIR(4);
         break;
      case PROGRAM_CHANGE:
         color = COLOR_PAIR(5);
         break;
   }

   wattrset(histwin, color);
   wprintw(histwin, "%02X ", data);
   wattrset(histwin, A_NORMAL);

   wnoutrefresh(histwin);
}

void printCommandHistory(void) {
   unsigned int count = 0;
   for (int r = 0; r < ROWS; r++) {
      for (int c = 0; c < COLUMNS; c++) {
         unsigned int size = commandHistoryGetSize();
         if (count >= size)
            mvprintw(r, COMMAND_HISTORY_X_POS + 3 * c, "   ");
         else {
            attr_t color = 0;
            uint8_t data = commandHistoryGet(count);

            switch (data) {
               case NOTE_ON:
                  color = COLOR_PAIR(1);
                  break;
               case NOTE_OFF:
                  color = COLOR_PAIR(2);
                  break;
               case CONTROL_CHANGE:
                  color = COLOR_PAIR(3);
                  break;
               case PITCH_CHANGE:
                  color = COLOR_PAIR(4);
                  break;
               case PROGRAM_CHANGE:
                  color = COLOR_PAIR(5);
                  break;
            }
            attron(color);
            mvprintw(r, COMMAND_HISTORY_X_POS + 3 * c, "%02X", data);
            attroff(color);
         }
         count++;
      }
   }
}

void printLegend(void) {
   mvwprintw(legendwin, 0, 0, "LEGEND");
   wattron(legendwin, COLOR_PAIR(1));
   mvwprintw(legendwin, 2, 0, "%X", NOTE_ON);
   wattroff(legendwin, COLOR_PAIR(1));
   mvwprintw(legendwin, 2, 3, "NOTE_ON");

   wattron(legendwin, COLOR_PAIR(2));
   mvwprintw(legendwin, 3, 0, "%X", NOTE_OFF);
   wattroff(legendwin, COLOR_PAIR(2));
   mvwprintw(legendwin, 3, 3, "NOTE_OFF");

   wattron(legendwin, COLOR_PAIR(3));
   mvwprintw(legendwin, 4, 0, "%X", CONTROL_CHANGE);
   wattroff(legendwin, COLOR_PAIR(3));
   mvwprintw(legendwin, 4, 3, "CONTROL_CHANGE");

   wattron(legendwin, COLOR_PAIR(4));
   mvwprintw(legendwin, 5, 0, "%X", PITCH_CHANGE);
   wattroff(legendwin, COLOR_PAIR(4));
   mvwprintw(legendwin, 5, 3, "PITCH_CHANGE");

   wattron(legendwin, COLOR_PAIR(5));
   mvwprintw(legendwin, 6, 0, "%X", PROGRAM_CHANGE);
   wattroff(legendwin, COLOR_PAIR(5));
   mvwprintw(legendwin, 6, 3, "PROGRAM_CHANGE");

   wnoutrefresh(legendwin);
}

void printMenu(void) {
   mvwprintw(selectmidi, 0, 0, "Select MIDI device: ");
   struct alsa_list list = getMidiDevices();
   int current_client = 0, current_port = 0;
   for (int row = 3; row < 13; row++) {
      if ((current_client == list.size - 1) &&
          (current_port == list.devices[current_client].ports_size))
         break;
      if (current_port == list.devices[current_client].ports_size) {
         current_client++;
         current_port = 0;
      }
      mvwprintw(selectmidi, row, 0, "%s %d:%d",
                list.devices[current_client].name,
                list.devices[current_client].client,
                list.devices[current_client].ports[current_port]);
      current_port++;
   }
   wnoutrefresh(selectmidi);
}

void printInfo(struct processedPacket info) {
   char *modeF = (info.opcode & FREQ_MODE_MASK) ? "Live" : "Autotune";
   char *modeA = (info.opcode & AMP_MODE_MASK) ? "Preset" : "Volume";
   mvwprintw(infowin, 0, 0, "FREQUENCY MODE: %-8s", modeF);
   mvwprintw(infowin, 1, 0, "AMPLITUDE MODE: %-16s", modeA);
   mvwprintw(infowin, 2, 0, "Pitch Num: %-5d", info.data1.noteNum);
   mvwprintw(infowin, 3, 0, "Pitch Bend: %-5d", info.data1.pitchBend);
   mvwprintw(infowin, 4, 0, "Program: %-5d", info.data2.midiNum);
   mvwprintw(infowin, 5, 0, "Velocity: %-5d", info.data2.velocity);
   mvwprintw(infowin, 6, 0, "CC11: %-5d", info.data2.cc11);
   wnoutrefresh(infowin);
}

void handleInput(void) {
   static char buffer[PROMPT_BUFFER_SIZE] = {'\0'};
   static uint8_t index = 0;
   int ch = wgetch(promptwin);
   switch (ch) {
      case KEY_BACKSPACE: {
         if (index > 0) {
            index--;
            buffer[index] = '\0';
         }
         mvwprintw(promptwin, 0, 3, "%-78s", buffer);
         break;
      }
      case '\n':
      case KEY_ENTER:
      case '\r': {
         parseCommand(buffer);
         memset(buffer, '\0', sizeof buffer);
         index = 0;
         mvwprintw(promptwin, 0, 3, "%-78s", "");
         break;
      }
      case ERR: {
         return;
      }
      default: {
         buffer[index] = (uint8_t)ch;
         index++;
         mvwprintw(promptwin, 0, 3, "%-78s", buffer);

         break;
      }
   }
   wnoutrefresh(promptwin);
}

void parseCommand(char buffer[PROMPT_BUFFER_SIZE]) {
   char *command = strtok(buffer, " \t\n");
   char *arg1 = strtok(NULL, " \t\n");
   // char* arg2 = strtok(NULL, " \t\n");
   if (command == NULL)
      return;
   if (strcmp(command, "modeF") == 0)
      handleModeFCommand(arg1);
   if (strcmp(command, "modeA") == 0)
      handleModeACommand(arg1);
   if (strcmp(command, "instrument") == 0)
      handleInstrumentCommand(arg1);
}

void handleModeFCommand(char *arg) {
   if (strcmp(arg, "live") == 0) {
      opcode |= FREQ_MODE_MASK;
   } else if (strcasecmp(arg, "autotune") == 0) {
      opcode &= ~FREQ_MODE_MASK;
   } else if (strcasecmp(arg, "toggle") == 0) {
      opcode ^= FREQ_MODE_MASK;
   }
   writeIO(debugScreenOutOpcode, &opcode, sizeof opcode);
}

void handleModeACommand(char *arg) {
   if (strcmp(arg, "preset") == 0) {
      opcode |= AMP_MODE_MASK;
   } else if (strcasecmp(arg, "volume") == 0) {
      opcode &= ~AMP_MODE_MASK;
   } else if (strcasecmp(arg, "toggle") == 0) {
      opcode ^= AMP_MODE_MASK;
   }
   writeIO(debugScreenOutOpcode, &opcode, sizeof opcode);
}

void handleInstrumentCommand(char *arg) {
   char *endptr = NULL;
   uint8_t instrument = (uint8_t)strtol(arg, &endptr, 10);
   if (*endptr != '\0') {
      printf("bad endptr\n");
      exit(1);
   }
   writeIO(debugScreenOutInstrument, &instrument, sizeof instrument);
}

void processButton(uint8_t button) {
   callback[button].function(callback[button].argument);
}
