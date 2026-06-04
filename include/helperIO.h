#ifndef HELPER_IO_H
#define HELPER_IO_H

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int read_full(int fd, void *buf, size_t n);
int write_full(int fd, const void *buf, size_t n);
int readIO(int fd, void *buf, size_t n);
int writeIO(int fd, void *buf, size_t n);
void dupIO(int oldFd, int newFd);
int readIOPoll(int fd, void *buf, size_t n, int timeout);
void pipeInit(int pipefd[][2], int pipefdLen);
void leaveOpen(int pipefd[][2], int pipefdLen, int *openList, int openLen);
void closefds(int *closeList, int closeLen);
void createTee(int fdin, int fdout);

#endif