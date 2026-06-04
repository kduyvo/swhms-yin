#include "live.h"


int main(void) {
    uint16_t inputFrequency;
    double midiCur;

    struct modeFreqOut buffer;
    memset(&buffer, 0, sizeof buffer);
    int centerMidi = -1;
    uint16_t pitchBend;
    double thres = THRES; 

    while (readIO(STDIN_FILENO, &inputFrequency, sizeof inputFrequency) > 0){
        if (inputFrequency == 0) 
            continue;
        midiCur = 69.0 + 12.0*log2(((double) inputFrequency) / 440.0);
        if (centerMidi == -1 || midiCur >= centerMidi + thres || midiCur <= centerMidi - thres){
            centerMidi = (int) lround(midiCur);
            if (centerMidi < 0) centerMidi = 0;
            if (centerMidi > 127) centerMidi = 127;
            buffer.noteNum = (uint8_t) centerMidi;
        }
        if (midiCur - centerMidi >= PITCHBEND_RANGE/2)
            pitchBend = 16383;
        else if (midiCur - centerMidi <= -PITCHBEND_RANGE/2)
            pitchBend = 0;
        else
            pitchBend = (uint16_t) lround(( midiCur - centerMidi + 
                PITCHBEND_RANGE/2) * 16383/PITCHBEND_RANGE);
        // buffer.pbLSB = pitchBend & 0x7F;
        // buffer.pbMSB = (pitchBend >> 7) & 0x7F;
        buffer.pitchBend = pitchBend;
        // buffer.pb = ((pitchBend & 0x7F) << 7) | ((pitchBend >> 7) & 0x7F);
        writeIO(STDOUT_FILENO, &buffer, sizeof buffer);
    }
}