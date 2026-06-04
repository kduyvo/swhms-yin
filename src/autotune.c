#include "autotune.h"

int main(void)
{
    uint16_t inputFrequency;
    double midiPrev = -1;
    double midiCur;
    double thres = 0.8;
    struct modeFreqOut output;
    output.pitchBend = PITCH_BEND_CENTER;
    while (readIO(STDIN_FILENO, &inputFrequency, sizeof inputFrequency) > 0)
    {
        midiCur = 69.0 + 12.0 * log2(((double)inputFrequency) / 440.0);
        if (midiPrev == -1)
        {
            midiPrev = round(midiCur);
            output.noteNum = (uint8_t)midiPrev;
            writeIO(STDOUT_FILENO, &output, sizeof(output));
        }
        else if (midiCur >= midiPrev + thres || midiCur <= midiPrev - thres)
        {
            midiPrev = round(midiCur);
            output.noteNum = (uint8_t)midiPrev;
            writeIO(STDOUT_FILENO, &output, sizeof(output));
        }
        else
            writeIO(STDOUT_FILENO, &output, sizeof(output));
    }
}