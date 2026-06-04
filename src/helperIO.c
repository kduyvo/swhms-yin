#define _GNU_SOURCE
#include "helperIO.h"

int read_full(int fd, void *buf, size_t n) {
   uint8_t *p = buf;
   size_t got = 0;
   while (got < n) {
      ssize_t r = read(fd, p + got, n - got);
      if (r == 0)
         return 0; // EOF
      if (r < 0)
         return -1; // error
      got += (size_t)r;
   }
   return 1;
}

void pipeInit(int pipefd[][2], int pipefdLen) {
   for (int i = 0; i < pipefdLen; i++) {
      if (pipe2(pipefd[i], O_CLOEXEC) < 0) {
         exit(-1);
      }
   }
}

void leaveOpen(int pipefd[][2], int pipefdLen, int *openList, int openLen) {
   for (int i = 0; i < pipefdLen; i++) {
      int has1 = 0, has2 = 0;
      if (openLen > 0) {
         for (int j = 0; j < openLen; j++) {
            if (pipefd[i][0] == openList[j])
               has1 = 1;
            if (pipefd[i][1] == openList[j])
               has2 = 1;
         }
      }
      if (!has1)
         close(pipefd[i][0]);
      if (!has2)
         close(pipefd[i][1]);
   }
}

void closefds(int *closeList, int closeLen) {
   if (closeLen > 0) {
      for (int j = 0; j < closeLen; j++) {
         close(closeList[j]);
      }
   }
}

int write_full(int fd, const void *buf, size_t n) {
   const uint8_t *p = buf;
   size_t sent = 0;
   while (sent < n) {
      ssize_t w = write(fd, p + sent, n - sent);
      if (w <= 0)
         return -1;
      sent += (size_t)w;
   }
   return 1;
}

int readIO(int fd, void *buf, size_t n) {
   int bytesRead;
   if ((bytesRead = read_full(fd, buf, n)) < 0) {
      perror("read");
      exit(1);
   }
   return bytesRead;
}

int readIOPoll(int fd, void *buf, size_t n, int timeout) {
   struct pollfd pfd;
   pfd.fd = fd;
   pfd.events = POLLIN;
   int r = poll(&pfd, 1, timeout);
   if (r < 0) {
      perror("poll");
      exit(1);
   } else if (r > 0)
      return readIO(fd, buf, n);
   return 0;
}

int writeIO(int fd, void *buf, size_t n) {
   int bytesWrite;
   if ((bytesWrite = write_full(fd, buf, n)) < 0) {
      perror("write");
      exit(1);
   }
   return bytesWrite;
}

void dupIO(int oldFd, int newFd) {
   // if (dup2(oldFd, newFd) < 0){
   //     perror("dup2");
   //     exit(1);
   // }
   if (dup2(oldFd, newFd) < 0) {
      dprintf(2, "dup2 failed: old=%d new=%d errno=%d\n", oldFd, newFd, errno);
      perror("dup2");
      exit(1);
   }
   return;
}

void createTee(int fdin, int fdout) {
   if (tee(fdin, fdout, UINT16_MAX, 0) < 0) {
      perror("tee");
      exit(1);
   }
}