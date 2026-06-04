#define _GNU_SOURCE
#include "frameProcessor.h"

static void term_handler(int) {
   //kill(0, SIGTERM);
   return;
}

enum children {
   yin,
   envelope,
   NUM_CHILDREN
};

enum pipefds {
   inYin,
   inEnvelope,
   NUM_PIPES
};

int main(void){
   signal(SIGTERM, term_handler);
   signal(SIGINT, term_handler);
   int pipefd[NUM_PIPES][2];
   pid_t child[NUM_CHILDREN];
   pipeInit(pipefd, NUM_PIPES);
   yinProcess(child, pipefd);
   envelopeProcess(child, pipefd);
   mainProcess(pipefd);
}

/*******************************************************************************
 * Process: main
 * Author: Kevin Vo
 * Purpose: Polls frames and pipes to Yin and Envelope Processes.
 ******************************************************************************/
void mainProcess(int pipefd [][2]) {
   int16_t frameBuffer[BUFFER_SIZE];
   int openList[] = {
      pipefd[inYin][1],
      pipefd[inEnvelope][1]
   };
   int openLen = sizeof(openList)/sizeof(int);
   leaveOpen(pipefd, NUM_PIPES, openList, openLen);
   while(readIO(frameProcessorInFrame, frameBuffer, sizeof frameBuffer) > 0){
      writeIO(pipefd[inYin][1], frameBuffer, sizeof frameBuffer);
      writeIO(pipefd[inEnvelope][1], frameBuffer, sizeof frameBuffer);
   }
   closefds(openList, openLen);
   while(wait(NULL) > 0);
   close(frameProcessorInFrame);
   close(frameProcessorOutAmplitude);
   close(frameProcessorOutFrequency);
   return;
}
/*******************************************************************************
 * Child Process: Yin
 * Author: Kevin Vo
 * Purpose: Child Process that will process pitch. 
 ******************************************************************************/
void yinProcess(pid_t child[], int pipefd[][2]){
   if ((child[yin] = fork()) < 0) {
      perror("fork()");
      exit(-1);
   }
   else if (child[yin] == 0) {
      int openList[] = {
         pipefd[inYin][0]
      };
      int openLen = sizeof(openList)/sizeof(int);
      leaveOpen(pipefd, NUM_PIPES, openList, openLen);
      dupIO(pipefd[inYin][0], STDIN_FILENO);
      dupIO(frameProcessorOutFrequency, STDOUT_FILENO);
      closefds(openList, openLen);
      execl("./yin", "yin", NULL);
      perror("execl(): yin");
      exit(-1);
   }
   return;
}
/*******************************************************************************
 * Child Process: Envelope
 * Author: Kevin Vo
 * Purpose: Child Process that will process frames' amplitude. 
 ******************************************************************************/
void envelopeProcess(pid_t child[], int pipefd[][2]){
   if ((child[envelope] = fork()) < 0) {
      perror("fork()");
      exit(-1);
   }
   else if (child[envelope] == 0) {
      int openList[] = {
         pipefd[inEnvelope][0]
      };
      int openLen = sizeof(openList)/sizeof(int);
      leaveOpen(pipefd, NUM_PIPES, openList, openLen);
      dupIO(pipefd[inEnvelope][0], STDIN_FILENO);
      dupIO(frameProcessorOutAmplitude, STDOUT_FILENO);
      closefds(openList, openLen);
      execl("./envelope", "envelope", NULL);
      perror("execl(): envelope");
      exit(-1);
   }
   return;
}
