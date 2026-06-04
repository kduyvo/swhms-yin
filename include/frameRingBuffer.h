#ifndef FRAME_RING_BUFFER_H
#define FRAME_RING_BUFFER_H

#include "frameSettings.h"
#include "helperIO.h"
#define THROUGHPUT 5
#define HOP_SIZE BUFFER_SIZE / THROUGHPUT

typedef struct ringBuffer {
   int16_t buffer[BUFFER_SIZE];
   uint16_t head;
   uint16_t tail;
   uint16_t size;

} ringBuffer;

void ringBufferInit(ringBuffer *buffer);
ssize_t printBuffer(ringBuffer *r);
void dequeue(ringBuffer *r);
ssize_t enqueue(ringBuffer *r);

#endif
