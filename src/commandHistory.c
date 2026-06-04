#include "commandHistory.h"

static struct commandHistory history;

void commandHistoryInit(void) {
    memset(&history, 0, sizeof history);
}

void commandHistoryAdd(uint8_t data) {
    if (history.size == TOTAL_CAPACITY) {
        // memset(&history.data[history.head], 0, COLUMNS);
        history.head = (history.head + COLUMNS) % TOTAL_CAPACITY;
        history.size = history.size - COLUMNS;
    }
    unsigned int index = (history.head + history.size) % TOTAL_CAPACITY;
    history.data[index] = data;
    history.size++;
}

uint8_t commandHistoryGet(unsigned int index){
    return history.data[(history.head + index) % TOTAL_CAPACITY];
}

unsigned int commandHistoryGetSize(){
    return history.size;
}