#include "presetModulation.h"

int main(void) {
    uint8_t instrument;
    struct modeModOut output;
    presetInit();
    while(readIO(STDIN_FILENO, &instrument, sizeof(instrument))){
        output.midiNum = getMidiNum(instrument);
        output.velocity = getVelocity(instrument);
        output.cc11 = getCC11(instrument);
        writeIO(STDOUT_FILENO, &output, sizeof output);
    }
}