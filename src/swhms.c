#define _GNU_SOURCE
#include "swhms.h"

static void term_handler(int) { return; }

enum process {
   arecord,
   frameRingBuffer,
   frameProcessor,
   lpf,
   hpf,
   toMidi,
   NUM_CHILDREN
};

enum pipefds {
   nFramesPipe,
   frequencyPipe,
   amplitudePipe,
   opcodePipe,
   lpfPipe,
   hpfPipe,
   instrumentPipe,
   NUM_PIPES
};

int main(void) {
   signal(SIGTERM, term_handler);
   // signal(SIGINT, term_handler);
   int pipefd[NUM_PIPES][2];
   pid_t child[NUM_CHILDREN];
   pipeInit(pipefd, NUM_PIPES);
   ringBufferProcess(child, pipefd);
   frameProcessorProcess(child, pipefd);
   toMidiProcess(child, pipefd);
   lpfProcess(child, pipefd);
   hpfProcess(child, pipefd);
   // writeIO(pipefd[opcodePipe][1], &opcode, sizeof opcode);
   // writeIO(pipefd[instrumentPipe][1], &instrument, sizeof instrument);
   while (wait(NULL) > 0)
      ;
   exit(0);
}

void lpfProcess(pid_t child[], int pipefd[][2]) {
   child[lpf] = fork();
   if (child[lpf] == 0) {
      int openList[] = {pipefd[lpfPipe][1], pipefd[hpfPipe][0]};
      int openLen = sizeof(openList) / sizeof(int);
      leaveOpen(pipefd, NUM_PIPES, openList, openLen);
      dupIO(pipefd[lpfPipe][1], STDOUT_FILENO);
      dupIO(pipefd[hpfPipe][0], STDIN_FILENO);
      closefds(openList, openLen);
      execl("./lpf", "lpf", NULL);
      perror("execl(): lpf");
      exit(1);
   } else if (child[lpf] < 0) {
      perror("fork");
      exit(-1);
   }
   return;
}

void hpfProcess(pid_t child[], int pipefd[][2]) {
   child[hpf] = fork();
   if (child[hpf] == 0) {
      int openList[] = {pipefd[hpfPipe][1]};
      int openLen = sizeof(openList) / sizeof(int);
      leaveOpen(pipefd, NUM_PIPES, openList, openLen);
      dupIO(pipefd[hpfPipe][1], STDOUT_FILENO);
      dupIO(swhmsInAudio, STDIN_FILENO);
      closefds(openList, openLen);
      execl("./hpf", "hpf", NULL);
      perror("execl(): hpf");
      exit(1);
   } else if (child[hpf] < 0) {
      perror("fork");
      exit(-1);
   }
   return;
}

/*******************************************************************************
 * Child Process: Ring Buffer
 * Author: Kevin Vo
 * Purpose: Ring Buffer that speeds up throughput
 ******************************************************************************/
void ringBufferProcess(pid_t child[], int pipefd[][2]) {
   child[frameRingBuffer] = fork();
   if (child[frameRingBuffer] == 0) {
      int openList[] = {pipefd[nFramesPipe][1], pipefd[lpfPipe][0]};
      int openLen = sizeof(openList) / sizeof(int);
      leaveOpen(pipefd, NUM_PIPES, openList, openLen);
      dupIO(pipefd[nFramesPipe][1], STDOUT_FILENO);
      dupIO(pipefd[lpfPipe][0], STDIN_FILENO);
      closefds(openList, openLen);
      execl("./frameRingBuffer", "frameRingbuffer", NULL);
      perror("execl(): frameRingBuffer");
      exit(1);
   } else if (child[frameRingBuffer] < 0) {
      perror("fork");
      exit(-1);
   }
   return;
}

/*******************************************************************************
 * Child Process: frameProcessor
 * Author: Kevin Vo
 * Purpose: Processes each frame's Pitch and Envelope
 ******************************************************************************/
void frameProcessorProcess(pid_t child[], int pipefd[][2]) {
   child[frameProcessor] = fork();
   if (child[frameProcessor] == 0) {
      int openList[] = {pipefd[nFramesPipe][0], pipefd[frequencyPipe][1],
                        pipefd[amplitudePipe][1]};
      int openLen = sizeof(openList) / sizeof(int);
      leaveOpen(pipefd, NUM_PIPES, openList, openLen);
      dupIO(pipefd[nFramesPipe][0], frameProcessorInFrame);
      dupIO(pipefd[frequencyPipe][1], frameProcessorOutFrequency);
      dupIO(pipefd[amplitudePipe][1], frameProcessorOutAmplitude);
      closefds(openList, openLen);
      execl("./frameProcessor", "frameProcessor", NULL);
      perror("execl(): frameProcessor");
      exit(1);
   } else if (child[frameProcessor] < 0) {
      perror("fork");
      exit(-1);
   }
   return;
}
/*******************************************************************************
 * Child Process: toMidi
 * Author: Kevin Vo
 * Purpose: Converts output from frameProcessor and converts to midi command.
 ******************************************************************************/
void toMidiProcess(pid_t child[], int pipefd[][2]) {
   child[toMidi] = fork();
   if (child[toMidi] == 0) {
      int openList[] = {pipefd[frequencyPipe][0], pipefd[amplitudePipe][0],
                        pipefd[instrumentPipe][0]};
      int openLen = sizeof(openList) / sizeof(int);
      leaveOpen(pipefd, NUM_PIPES, openList, openLen);
      dupIO(pipefd[frequencyPipe][0], toMidiInFrequency);
      dupIO(pipefd[amplitudePipe][0], toMidiInAmplitude);
      dupIO(swhmsInOpcode, toMidiInOpcode);
      dupIO(swhmsInInstrument, toMidiInInstrument);
      dupIO(swhmsOutCommand, toMidiOutCommand);
      dupIO(swhmsOutInfo, toMidiOutInfo);
      closefds(openList, openLen);
      execl("./toMidi", "toMidi", NULL);
      perror("execl(): toMidi");
      exit(1);
   } else if (child[toMidi] < 0) {
      perror("fork");
      exit(-1);
   }
}
