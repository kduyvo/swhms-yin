
#include "encoder.h"
#include <alsa/asoundlib.h>

int main(void)
{
    struct processedPacket input;
    struct processedPacket previous;
    memset(&previous, 0, sizeof previous);

    int prev_active = 0;

    while (readIO(encoderInPacket, &input, sizeof input) > 0)
    {
        int curr_active = (input.data1.noteNum > 0 && input.data2.velocity > 0);

        int note_changed = prev_active && curr_active &&
                           input.data1.noteNum != previous.data1.noteNum;

        int prog_changed = prev_active &&
                           input.data2.midiNum != previous.data2.midiNum;

        int note_started = curr_active && !prev_active;
        int note_stopped = prev_active && !curr_active;

        if (note_stopped || note_changed || prog_changed)
            note_off(encoderOut, 1, previous.data1.noteNum, 0);

        if (input.data2.midiNum != previous.data2.midiNum)
            program_change(encoderOut, 1, input.data2.midiNum);

        if (input.data1.pitchBend != previous.data1.pitchBend)
            pitch_change(encoderOut, 1, input.data1.pitchBend);

        if (input.data2.cc11 != previous.data2.cc11)
            control_change(encoderOut, 1, 11, input.data2.cc11);

        if (note_started || note_changed || prog_changed)
            note_on(encoderOut, 1, input.data1.noteNum, input.data2.velocity);

        previous = input;
        prev_active = curr_active;
    }

    return 0;
}