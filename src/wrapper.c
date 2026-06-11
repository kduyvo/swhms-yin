#include "wrapper.h"

enum process {
   swhms,
   battery,
   arecord,
   debugScreen,
   uart,
   alsa,
   keyPad,
   NUM_CHILDREN
};

enum pipefds {
   swhmsInfo,
   batInfo,
   opcodePipe,
   arecordOut,
   instrumentPipe,
   swhmsCommand,
   button,
   forwardedCommand,
   forwardedInfo,
   NUM_PIPES
};

void sigint_handler(int sig) {
   (void)sig;
   // Send SIGTERM to entire process group
   kill(0, SIGTERM);
}

void sigterm_handler(int sig) {
   (void)sig;
   return;
}

void sigterm2_handler(int sig) {
   (void)sig;
   while (wait(NULL) > 0)
      ;
   _exit(0);
}

int main(void) {
   setpgid(0, 0); // make parent the leader of a new process group
   signal(SIGINT, sigint_handler);
   signal(SIGTERM, sigterm_handler);
   pid_t child[NUM_CHILDREN];
   int pipefd[NUM_PIPES][2];
   pipeInit(pipefd, NUM_PIPES);
   arecordProcess(child, pipefd);
   swhmsProcess(child, pipefd);
   uartProcess(child, pipefd);
   debugScreenProcess(child, pipefd);
   batteryProcess(child, pipefd);
   alsaProcess(child, pipefd);
   keypadProcess(child, pipefd);
   int devnull = open("/dev/null", O_WRONLY);
   // dupIO(devnull, STDERR_FILENO);
   // dupIO(devnull, STDIN_FILENO);
   // dupIO(devnull, STDOUT_FILENO);
   close(devnull);
   while (wait(NULL) > 0)
      ;
}

void arecordProcess(pid_t child[], int pipefd[][2]) {
   child[arecord] = fork();
   if (child[arecord] == 0) {
      signal(SIGTERM, sigterm2_handler);
      while (1) {
         pid_t grandChild = fork();
         if (grandChild == 0) {
            int card, device;
            if (getAudioHW(&card, &device) < 0)
               exit(1);
            char plughw[16];
            sprintf(plughw, "plughw:%d,%d", card, device);
            int openList[] = {pipefd[arecordOut][1]};
            int openLen = sizeof(openList) / sizeof(int);
            leaveOpen(pipefd, NUM_PIPES, openList, openLen);
            dupIO(pipefd[arecordOut][1], STDOUT_FILENO);
            closefds(openList, openLen);
            int devnull = open("/dev/null", O_WRONLY);
            dupIO(devnull, STDERR_FILENO);
            dupIO(devnull, STDIN_FILENO);
            close(devnull);
            fprintf(stderr, "launching arecord\n");
            fflush(stderr);
            execlp("arecord", "arecord", "-D", plughw, "-f", "S16_LE", "-c",
                   "1", "-r", "48000", "--period-size=256",
                   "--buffer-size=1024", "-t", "raw", NULL);
            perror("execlp arecord failed");
            exit(1);
         }
         sleep(1);
         while (wait(NULL) > 0)
            ;
      }
   } else if (child[arecord] < 0) {
      perror("fork");
      exit(-1);
   }
   return;
}

void keypadProcess(pid_t child[], int pipefd[][2]) {
   child[keyPad] = fork();
   if (child[keyPad] == 0) {
      // setsid();
      int openList[] = {pipefd[button][1]};
      int openLen = sizeof(openList) / sizeof(int);
      leaveOpen(pipefd, NUM_PIPES, openList, openLen);
      dupIO(pipefd[button][1], STDOUT_FILENO);
      closefds(openList, openLen);
      int devnull = open("/dev/null", O_WRONLY);
      dupIO(devnull, STDERR_FILENO);
      dupIO(devnull, STDIN_FILENO);
      close(devnull);
      execl("keypad", "keypad", NULL);
      perror("execlp(): keypad");
      exit(1);
   } else if (child[keyPad] < 0) {
      perror("fork");
      exit(-1);
   }
   return;
}

void debugScreenProcess(pid_t child[], int pipefd[][2]) {
   child[debugScreen] = fork();
   if (child[debugScreen] == 0) {
      int openList[] = {pipefd[instrumentPipe][1], pipefd[swhmsInfo][0],
                        pipefd[opcodePipe][1],     pipefd[forwardedCommand][1],
                        pipefd[swhmsCommand][0],   pipefd[batInfo][0],
                        pipefd[button][0],         pipefd[forwardedInfo][1]};
      int openLen = sizeof(openList) / sizeof(int);
      leaveOpen(pipefd, NUM_PIPES, openList, openLen);
      dupIO(pipefd[instrumentPipe][1], debugScreenOutInstrument);
      dupIO(pipefd[swhmsInfo][0], debugScreenInInfo);
      dupIO(pipefd[opcodePipe][1], debugScreenOutOpcode);
      dupIO(pipefd[forwardedCommand][1], debugScreenOutMIDI);
      dupIO(pipefd[forwardedInfo][1], debugScreenOutInfo);
      dupIO(pipefd[swhmsCommand][0], debugScreenInMIDI);
      dupIO(pipefd[batInfo][0], debugScreenInBatInfo);
      dupIO(pipefd[button][0], debugScreenInKeypad);
      int devnull = open("/dev/null", O_WRONLY);
      dupIO(devnull, STDERR_FILENO);
      closefds(openList, openLen);
      execl("./debugScreen", "debugScreen", NULL);
      perror("execl(): debugScreen");
      exit(-1);
   } else if (child[debugScreen] < 0) {
      perror("fork: debugScreen");
      exit(-1);
   }
   return;
}

void batteryProcess(pid_t child[], int pipefd[][2]) {
   child[battery] = fork();
   if (child[battery] == 0) {
      // setsid();
      int openList[] = {pipefd[batInfo][1]};
      int openLen = sizeof(openList) / sizeof(int);
      leaveOpen(pipefd, NUM_PIPES, openList, openLen);
      dupIO(pipefd[batInfo][1], STDOUT_FILENO);
      int devnull = open("/dev/null", O_WRONLY);
      dupIO(devnull, STDERR_FILENO);
      dupIO(devnull, STDIN_FILENO);
      closefds(openList, openLen);
      execl("./battery", "battery", NULL);
      perror("execl(): battery");
      exit(-1);
   } else if (child[battery] < 0) {
      perror("fork: battery");
      exit(-1);
   }
   return;
}

void swhmsProcess(pid_t child[], int pipefd[][2]) {
   child[swhms] = fork();
   if (child[swhms] == 0) {
      // setsid();
      int openList[] = {pipefd[swhmsInfo][1], pipefd[opcodePipe][0],
                        pipefd[swhmsCommand][1], pipefd[instrumentPipe][0],
                        pipefd[arecordOut][0]};
      int openLen = sizeof(openList) / sizeof(int);
      leaveOpen(pipefd, NUM_PIPES, openList, openLen);
      dupIO(pipefd[opcodePipe][0], swhmsInOpcode);
      dupIO(pipefd[swhmsInfo][1], swhmsOutInfo);
      dupIO(pipefd[arecordOut][0], swhmsInAudio);
      dupIO(pipefd[swhmsCommand][1], swhmsOutCommand);
      dupIO(pipefd[instrumentPipe][0], swhmsInInstrument);
      int devnull = open("/dev/null", O_WRONLY);
      dupIO(devnull, STDERR_FILENO);
      dupIO(devnull, STDOUT_FILENO);
      dupIO(devnull, STDIN_FILENO);
      closefds(openList, openLen);
      execl("./swhms", "swhms", NULL);
      perror("execl(): swhms");
      exit(-1);
   } else if (child[swhms] < 0) {
      perror("fork: swhms");
      exit(-1);
   }
   return;
}

void uartProcess(pid_t child[], int pipefd[][2]) {
   child[uart] = fork();
   if (child[uart] == 0) {
      // setsid();
      int openList[] = {pipefd[forwardedCommand][0]};
      int openLen = sizeof(openList) / sizeof(int);
      leaveOpen(pipefd, NUM_PIPES, openList, openLen);
      dupIO(pipefd[forwardedCommand][0], STDIN_FILENO);
      int devnull = open("/dev/null", O_WRONLY);
      dupIO(devnull, STDERR_FILENO);
      dupIO(devnull, STDOUT_FILENO);
      closefds(openList, openLen);
      execl("./uart", "uart", NULL);
      perror("execl(): uart");
      exit(-1);
   } else if (child[uart] < 0) {
      perror("fork: swhms");
      exit(-1);
   }
   return;
}

void alsaProcess(pid_t child[], int pipefd[][2]) {
   child[alsa] = fork();
   if (child[alsa] == 0) {
      // setsid();
      int openList[] = {pipefd[forwardedInfo][0]};
      int openLen = sizeof(openList) / sizeof(int);
      leaveOpen(pipefd, NUM_PIPES, openList, openLen);
      dupIO(pipefd[forwardedInfo][0], alsaInPacket);
      int devnull = open("/dev/null", O_WRONLY);
      dupIO(devnull, STDERR_FILENO);
      dupIO(devnull, STDOUT_FILENO);
      closefds(openList, openLen);
      execl("./alsa", "alsa", NULL);
      perror("execl(): alsa");
      exit(-1);
   } else if (child[alsa] < 0) {
      perror("fork: alsa");
      exit(-1);
   }
   return;
}
