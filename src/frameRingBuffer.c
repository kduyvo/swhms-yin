#include "frameRingBuffer.h"

int main(void){
    ringBuffer frameRingBuffer;
    ringBufferInit(&frameRingBuffer);
    while (enqueue(&frameRingBuffer) > 0){
        if (frameRingBuffer.size == BUFFER_SIZE){
            if (printBuffer(&frameRingBuffer) < 0){
                perror("printBuffer()");
                exit(1);
            }
            dequeue(&frameRingBuffer);
        }
    }
}

void ringBufferInit(ringBuffer* buffer){
    buffer->head = 0;
    buffer->tail = 0;
    buffer->size = 0;
}

void dequeue(ringBuffer* r){
    if (r->size < HOP_SIZE) return;

    r->head = (r->head +  HOP_SIZE) % BUFFER_SIZE;
    r->size -= HOP_SIZE;
}

ssize_t enqueue(ringBuffer* r){
    if (r->size > BUFFER_SIZE - HOP_SIZE) return -1;
    uint16_t tailSize1 = (BUFFER_SIZE - r->tail < HOP_SIZE) ? BUFFER_SIZE - r->tail : HOP_SIZE;
    uint16_t tailSize2 = HOP_SIZE - tailSize1;
    int bytesRead1 = 0;
    int bytesRead2 = 0;
    if ((bytesRead1 = read_full(STDIN_FILENO, &r->buffer[r->tail], tailSize1*sizeof(int16_t))) < 0){
        return -1;
    }
    if (tailSize1 != HOP_SIZE && (bytesRead2 = read_full(STDIN_FILENO, r->buffer, tailSize2*sizeof(int16_t))) < 0){
        return -1;
    }
    r->tail = (r->tail + HOP_SIZE) % BUFFER_SIZE;
    r->size += HOP_SIZE;
    return (bytesRead1 + bytesRead2);
}

ssize_t printBuffer(ringBuffer* r){
    uint16_t bufferSize1 = BUFFER_SIZE - r->head;
    uint16_t bufferSize2 = r->head;
    int bytesWrite1 = 0;
    int bytesWrite2 = 0;
    if ((bytesWrite1 = write_full(STDOUT_FILENO, &r->buffer[r->head], bufferSize1*sizeof(int16_t))) < 0){
        return -1;
    }
    if (bufferSize1 != BUFFER_SIZE && (bytesWrite2 += write_full(STDOUT_FILENO, r->buffer, bufferSize2*sizeof(int16_t))) < 0){
        return -1;
    }
    return bytesWrite1 + bytesWrite2;  
}