#include "debounce.h"
enum state {
   holdPitch,
   holdMute,
   changePitch,
   changeMute,
};

int main() {
   struct processedPacket input, lastIn, out;
   memset(&input, 0, sizeof input);
   memset(&lastIn, 0, sizeof lastIn);
   memset(&out, 0, sizeof out);
   uint8_t count = 1;
   uint8_t maxVelocity = 0;
   uint8_t maxCC11 = 0;
   int diffMidiPitch;
   enum state currentState = changePitch;
   while (readIO(debounceInPacket, &input, sizeof input) > 0) {
      switch (currentState) {
         case (changePitch): {
            if (input.data2.velocity == 0) {
               if (out.data2.velocity == 0) {
                  currentState = holdMute;
                  count = 1;
                  break;
               } else {
                  currentState = changeMute;
                  count = 1;
                  break;
               }
            }

            if (input.data1.noteNum == lastIn.data1.noteNum) {
               count++;
               if (input.data2.velocity > maxVelocity) {
                  maxVelocity = input.data2.velocity;
                  maxCC11 = input.data2.cc11;
               }
            } else if (input.data1.noteNum == out.data1.noteNum) {
               count = 1;
               currentState = holdPitch;
               maxCC11 = 0;
               maxVelocity = 0;
               break;
            } else {
               count = 1;
               maxVelocity = input.data2.velocity;
               maxCC11 = input.data2.cc11;
               lastIn = input;
            }
            diffMidiPitch =
               abs((int)out.data1.noteNum - (int)input.data1.noteNum);
            if ((count >= THRESHOLD && (diffMidiPitch <= MAX_SEMITONE_CHANGE ||
                                        out.data1.noteNum == 0)) ||
                (count >= THRESHOLD_LARGE &&
                 diffMidiPitch > MAX_SEMITONE_CHANGE)) {
               out = lastIn;
               out.data2.velocity = maxVelocity;
               out.data2.cc11 = maxCC11;
               currentState = holdPitch;
               writeIO(debounceOut1, &out, sizeof out);
               writeIO(debounceOut2, &out, sizeof out);
               break;
            }
            break;
         }
         case (changeMute): {
            if (input.data2.velocity == 0) {
               count++;
            } else if (input.data1.noteNum == out.data1.noteNum) {
               count = 1;
               currentState = holdPitch;
               maxCC11 = 0;
               maxVelocity = 0;
               break;
            } else {
               count = 1;
               maxVelocity = input.data2.velocity;
               maxCC11 = input.data2.cc11;
               lastIn = input;
               currentState = changePitch;
            }
            if (count >= THRESHOLD) {
               // memset(&out, 0, sizeof out);
               // out.opcode = lastIn.opcode;
               out = input;
               currentState = holdMute;
               writeIO(debounceOut1, &out, sizeof out);
               writeIO(debounceOut2, &out, sizeof out);
               break;
            }
            break;
         }
         case (holdPitch): {

            int pitchBendOffset =
               abs((int)out.data1.pitchBend - (int)input.data1.pitchBend);
            int cc11Offset = abs((int)out.data2.cc11 - (int)input.data2.cc11);

            if (input.data1.noteNum != out.data1.noteNum) {
               lastIn = input;
               count = 1;
               maxVelocity = input.data2.velocity;
               maxCC11 = input.data2.cc11;
               currentState = changePitch;
               break;
            } else if ((pitchBendOffset >= 1600) || (cc11Offset >= 5)) {
               out.data1.pitchBend = input.data1.pitchBend;
               out.data2.velocity = input.data2.velocity;
               out.data2.cc11 = input.data2.cc11;
               writeIO(debounceOut1, &out, sizeof out);
               writeIO(debounceOut2, &out, sizeof out);
            }
            break;
         }
         case (holdMute): {
            if (input.data2.velocity != 0) {
               lastIn = input;
               count = 1;
               maxVelocity = input.data2.velocity;
               maxCC11 = input.data2.cc11;
               currentState = changePitch;
               break;
            }
            break;
         }
      }
   }
}