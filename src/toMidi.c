#define _GNU_SOURCE
#include "toMidi.h"
static void term_handler(int) {
   // kill(0, SIGTERM);
   return;
}
enum process {
   autotune,
   live,
   volumeModulation,
   presetModulation,
   selector,
   scheduler,
   debounce,
   encoder,
   NUM_CHILDREN
};

enum pipefds {
   selectorAutotune,
   selectorLive,
   selectorVolumeModulation,
   selectorPresetModulation,
   selectorScheduler,
   autotuneScheduler,
   volumeModulationScheduler,
   presetModulationScheduler,
   liveScheduler,
   inSelector,
   schedulerDebounce,
   // debounceOut,
   debounceEncoder,
   // encoderOut,
   NUM_PIPES
};

int main(void) {
   signal(SIGTERM, term_handler);
   signal(SIGINT, term_handler);
   int pipefd[NUM_PIPES][2];
   pid_t child[NUM_CHILDREN];
   pipeInit(pipefd, NUM_PIPES);
   autotuneProcess(child, pipefd);
   liveProcess(child, pipefd);
   selectorProcess(child, pipefd);
   volumeModulationProcess(child, pipefd);
   presetModulationProcess(child, pipefd);
   schedulerProcess(child, pipefd);
   debounceProcess(child, pipefd);
   encoderProcess(child, pipefd);
   mainProcess(pipefd);
   exit(0);
}

/*******************************************************************************
 * Main Process: main
 * Author: Kevin Vo
 * Purpose: Waits for all Processes to exit
 *
 * INPUT: stream of pitches (uint16_t)
 * OUTPUT: stream of midi frequency codes (uint8_t)
 ******************************************************************************/
void mainProcess(int pipefd[][2]) {
   // struct processedPacket buffer;
   // int openList[] = {
   //     pipefd[debounceOut][0],
   // };
   // int openLen = sizeof(openList)/sizeof(int);
   leaveOpen(pipefd, NUM_PIPES, NULL, 0);
   // printf("test67\n");
   // while (readIO(pipefd[debounceOut][0], &buffer, sizeof buffer) > 0) {
   //     // printf("test68\n");
   //     printf("ModeF: %s, ModeM: %s, Note Num: %d, Pitch Bend: %d, Program:
   //     %d, Velocity: %d, CC11: %d\n",
   //             (buffer.opcode & 1) == 0 ? "Autotune" : "Live",
   //             (buffer.opcode & 2) == 0 ? "VolumeMod" : "PresetMod",
   //             buffer.data1.noteNum, buffer.data1.pitchBend,
   //             buffer.data2.midiNum, buffer.data2.velocity,
   //             buffer.data2.cc11);
   // }
   while (wait(NULL) > 0)
      ;
   // leaveOpen(pipefd, NUM_PIPES, NULL, 0);
}
/*******************************************************************************
 * Child Process: Selector
 * Author: Kevin Vo
 * Purpose: Separate opcode, frequency and amplitude components,
 *          redirect streams to programs, and flush out disabled programs
 *
 * INPUT: stream of opcode, pitch, amplitude tuple (uint8_t, uint16_t, uint16_t)
 * OUTPUT: Two streams of pitches and two streams of amplitude
 ******************************************************************************/
void selectorProcess(pid_t child[], int pipefd[][2]) {
   child[selector] = fork();
   if (child[selector] < 0) {
      perror("fork");
      exit(1);
   } else if (child[selector] == 0) {
      static uint8_t opcode = 0, sendOpcode = 0, instrument = 0;
      uint16_t amplitude, frequency;
      int openList[] = {pipefd[selectorScheduler][1],
                        pipefd[selectorAutotune][1], pipefd[selectorLive][1],
                        pipefd[selectorVolumeModulation][1],
                        pipefd[selectorPresetModulation][1]};
      int openLen = sizeof(openList) / sizeof(int);
      leaveOpen(pipefd, NUM_PIPES, openList, openLen);
      while (1) {
         if (readIO(toMidiInAmplitude, &amplitude, sizeof amplitude) == 0)
            break;
         if (readIO(toMidiInFrequency, &frequency, sizeof frequency) == 0)
            break;
         readIOPoll(toMidiInOpcode, &opcode, sizeof opcode, 0);
         readIOPoll(toMidiInInstrument, &instrument, sizeof instrument, 0);
         sendOpcode = opcode;
         // if ((opcode & MUTE_MASK) == 0 && amplitude >= FLOOR_AMPLITUDE &&
         // amplitude <= CEIL_AMPLITUDE) {
         if ((opcode & MUTE_MASK) == 0 && amplitude >= FLOOR_AMPLITUDE) {

            if ((opcode & 1) == 0)
               writeIO(pipefd[selectorAutotune][1], &frequency,
                       sizeof frequency);
            else
               writeIO(pipefd[selectorLive][1], &frequency, sizeof frequency);
            if ((opcode & 2) == 0) {
               writeIO(pipefd[selectorVolumeModulation][1], &amplitude,
                       sizeof amplitude);
               writeIO(pipefd[selectorVolumeModulation][1], &instrument,
                       sizeof instrument);
            } else
               writeIO(pipefd[selectorPresetModulation][1], &instrument,
                       sizeof instrument);
         } else {
            sendOpcode |= MUTE_MASK;
         }
         writeIO(pipefd[selectorScheduler][1], &sendOpcode, sizeof sendOpcode);
      }
      closefds(openList, openLen);
      exit(0);
   }
   return;
}
/*******************************************************************************
 * Child Process: Autotune
 * Author: Kevin Vo
 * Purpose: Child Process that will process pitch into midi number.
 *
 * INPUT: stream of pitches (uint16_t)
 * OUTPUT: stream of midi frequency codes (uint8_t)
 ******************************************************************************/
void autotuneProcess(pid_t child[], int pipefd[][2]) {
   child[autotune] = fork();
   if (child[autotune] == 0) {
      int openList[] = {pipefd[selectorAutotune][0],
                        pipefd[autotuneScheduler][1]};
      int openLen = sizeof(openList) / sizeof(int);
      leaveOpen(pipefd, NUM_PIPES, openList, openLen);
      dupIO(pipefd[selectorAutotune][0], STDIN_FILENO);
      dupIO(pipefd[autotuneScheduler][1], STDOUT_FILENO);
      closefds(openList, openLen);
      execl("./autotune", "autotune", NULL);
      perror("execl(): autotune");
      exit(-1);
   } else if (child[autotune] < 0) {
      perror("fork");
      exit(-1);
   }
   return;
}
/*******************************************************************************
 * Child Process: Live
 * Author: Kevin Vo
 * Purpose: Child Process that will process pitch into midi number using live
 *          mode.
 *
 * INPUT: stream of pitches (uint16_t)
 * OUTPUT: stream of tuple midi frequency codes (uint8_t), LSB (uint8_t), MSB
 * (uint8_t)
 ******************************************************************************/
void liveProcess(pid_t child[], int pipefd[][2]) {
   child[live] = fork();
   if (child[live] == 0) {
      int openList[] = {pipefd[selectorLive][0], pipefd[liveScheduler][1]};
      int openLen = sizeof(openList) / sizeof(int);
      leaveOpen(pipefd, NUM_PIPES, openList, openLen);
      dupIO(pipefd[selectorLive][0], STDIN_FILENO);
      dupIO(pipefd[liveScheduler][1], STDOUT_FILENO);
      closefds(openList, openLen);
      execl("./live", "live", NULL);
      perror("execl(): live");
      exit(-1);
   } else if (child[live] < 0) {
      perror("fork");
      exit(-1);
   }
   return;
}
/*******************************************************************************
 * Child Process: volumeModulation
 * Author: Kevin Vo
 * Purpose: Child Process that will process amplitude into midi number
 *
 * INPUT: stream of pitches (uint16_t)
 * OUTPUT: stream of tuple midi volume code
 ******************************************************************************/
void volumeModulationProcess(pid_t child[], int pipefd[][2]) {
   child[volumeModulation] = fork();
   if (child[volumeModulation] == 0) {
      int openList[] = {pipefd[selectorVolumeModulation][0],
                        pipefd[volumeModulationScheduler][1]};
      int openLen = sizeof(openList) / sizeof(int);
      leaveOpen(pipefd, NUM_PIPES, openList, openLen);
      dupIO(pipefd[selectorVolumeModulation][0], STDIN_FILENO);
      dupIO(pipefd[volumeModulationScheduler][1], STDOUT_FILENO);
      closefds(openList, openLen);
      execl("./volumeModulation", "volumeModulation", NULL);
      perror("execl(): volumeModulation");
      exit(-1);
   } else if (child[volumeModulation] < 0) {
      perror("fork");
      exit(-1);
   }
   return;
}
/*******************************************************************************
 * Child Process: presetModulation
 * Author: Kevin Vo
 * Purpose: Child Process that will process instrument into midi number
 *
 * INPUT: stream of pitches (uint16_t)
 * OUTPUT: stream of tuple midi volume code
 ******************************************************************************/
void presetModulationProcess(pid_t child[], int pipefd[][2]) {
   child[presetModulation] = fork();
   if (child[presetModulation] == 0) {
      int openList[] = {pipefd[selectorPresetModulation][0],
                        pipefd[presetModulationScheduler][1]};
      int openLen = sizeof(openList) / sizeof(int);
      leaveOpen(pipefd, NUM_PIPES, openList, openLen);
      dupIO(pipefd[selectorPresetModulation][0], STDIN_FILENO);
      dupIO(pipefd[presetModulationScheduler][1], STDOUT_FILENO);
      closefds(openList, openLen);
      execl("./presetModulation", "presetModulation", NULL);
      perror("execl: presetModulation");
      exit(-1);
   } else if (child[presetModulation] < 0) {
      perror("fork");
      exit(-1);
   }
   return;
}
/*******************************************************************************
 * Child Process: Scheduler
 * Author: Kevin Vo
 * Purpose: Dependent on incoming opcode, will take in specific queues from the
 *          four modes, and packages them to be sent to the output.
 *
 * INPUT:
 * OUTPUT: Two streams of pitches and two streams of amplitude
 ******************************************************************************/
void schedulerProcess(pid_t child[], int pipefd[][2]) {
   child[scheduler] = fork();
   if (child[scheduler] == 0) {
      static uint8_t opcode = 0;
      struct processedPacket packet;
      // struct processedPacket previousPacket;
      // memset(&previousPacket, 0, sizeof previousPacket);
      int openList[] = {pipefd[selectorScheduler][0],
                        pipefd[autotuneScheduler][0],
                        pipefd[liveScheduler][0],
                        pipefd[volumeModulationScheduler][0],
                        pipefd[presetModulationScheduler][0],
                        pipefd[schedulerDebounce][1]};
      int openLen = sizeof(openList) / sizeof(int);
      leaveOpen(pipefd, NUM_PIPES, openList, openLen);
      while (readIO(pipefd[selectorScheduler][0], &opcode, sizeof opcode) > 0) {
         // printf("opcode: %d\n", opcode);
         if ((opcode & MUTE_MASK) == 0) {
            if ((opcode & 1) == 0)
               readIO(pipefd[autotuneScheduler][0], &packet.data1,
                      sizeof packet.data1);
            else
               readIO(pipefd[liveScheduler][0], &packet.data1,
                      sizeof packet.data1);
            if ((opcode & 2) == 0)
               readIO(pipefd[volumeModulationScheduler][0], &packet.data2,
                      sizeof packet.data2);
            else
               readIO(pipefd[presetModulationScheduler][0], &packet.data2,
                      sizeof packet.data2);
            packet.opcode = opcode;
         } else {
            // memset(&packet, 0, sizeof packet);
            packet.opcode = opcode;
            packet.data1.noteNum = 0;
            // packet.data1.pitchBend = PITCH_BEND_CENTER;
            packet.data2.cc11 = 0;
            packet.data2.velocity = 0;
         }
         writeIO(pipefd[schedulerDebounce][1], &packet, sizeof packet);
      }
      closefds(openList, openLen);
      exit(0);
   } else if (child[scheduler] < 0) {
      perror("fork");
      exit(1);
   }
}
/*******************************************************************************
 * Child Process: Debounce
 * Author: Kevin Vo
 * Purpose: Child Process that will process pitch into midi number.
 *
 * INPUT: stream of pitches (uint16_t)
 * OUTPUT: stream of midi frequency codes (uint8_t)
 ******************************************************************************/
void debounceProcess(pid_t child[], int pipefd[][2]) {
   child[debounce] = fork();
   if (child[debounce] == 0) {
      int openList[] = {pipefd[schedulerDebounce][0],
                        pipefd[debounceEncoder][1]};
      int openLen = sizeof(openList) / sizeof(int);
      leaveOpen(pipefd, NUM_PIPES, openList, openLen);
      dupIO(pipefd[schedulerDebounce][0], debounceInPacket);
      dupIO(toMidiOutInfo, debounceOut1);
      dupIO(pipefd[debounceEncoder][1], debounceOut2);
      closefds(openList, openLen);
      execl("./debounce", "debounce", NULL);
      perror("execl(): debounce");
      exit(-1);
   } else if (child[debounce] < 0) {
      perror("fork");
      exit(-1);
   }
   return;
}
/*******************************************************************************
 * Child Process: Encoder
 * Author: Kevin Vo
 * Purpose: Child Process that will process pitch into midi number.
 *
 * INPUT: stream of pitches (uint16_t)
 * OUTPUT: stream of midi frequency codes (uint8_t)
 ******************************************************************************/
void encoderProcess(pid_t child[], int pipefd[][2]) {
   child[encoder] = fork();
   if (child[encoder] == 0) {
      int openList[] = {pipefd[debounceEncoder][0]};
      int openLen = sizeof(openList) / sizeof(int);
      leaveOpen(pipefd, NUM_PIPES, openList, openLen);
      dupIO(pipefd[debounceEncoder][0], encoderInPacket);
      dupIO(toMidiOutCommand, encoderOut);
      closefds(openList, openLen);
      execl("./encoder", "encoder", NULL);
      perror("execl(): encoder");
      exit(-1);
   } else if (child[encoder] < 0) {
      perror("fork");
      exit(-1);
   }
   return;
}
