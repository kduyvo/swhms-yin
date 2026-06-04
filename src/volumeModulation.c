#include "volumeModulation.h"

// int main(){
//     uint16_t inputAmplitude;
//     uint8_t instrument = 0;
//     struct modeModOut output;
//     presetInit();
//     while(readIO(STDIN_FILENO, &inputAmplitude, sizeof inputAmplitude) > 0){
//         readIO(STDIN_FILENO, &instrument, sizeof instrument);
//         output.midiNum = getMidiNum(instrument);
//         if (inputAmplitude <= FLOOR_AMPLITUDE){
//             output.velocity = 0;
//             output.cc11 = 0;
//         }
//         else if (inputAmplitude >= CEIL_AMPLITUDE) {
//             output.velocity = 127;
//             output.cc11 = 127;
//         }
//         else {
//             uint32_t temp = (inputAmplitude - FLOOR_AMPLITUDE) * 126;
//             uint32_t velocity = 1 + (temp + RANGE/2) / (RANGE);
//             output.velocity = (uint8_t) velocity;
//             output.cc11 = (uint8_t) velocity + 5;
//             if (output.cc11 > 127)
//                 output.cc11 = 127;
//         }
//         writeIO(STDOUT_FILENO, &output, sizeof output);
//     }

//     // close(volModInAmplitude);
//     // close(volModInInstrument);
//     // close(volModOut);
//     return 0
// }

#include "volumeModulation.h"
#include <math.h>

static uint8_t map_curve(uint16_t inputAmplitude) {
   if (inputAmplitude <= FLOOR_AMPLITUDE)
      return 0;
   if (inputAmplitude >= CEIL_AMPLITUDE)
      return 127;

   double x = (double)(inputAmplitude - FLOOR_AMPLITUDE) / (double)RANGE;

   /* Try 1.5, 2.0, or 0.7 depending on feel */
   const double gamma = 0.5;
   double y = pow(x, gamma);

   int midi = (int)lround(1.0 + y * 126.0);
   if (midi < 1)
      midi = 1;
   if (midi > 127)
      midi = 127;
   return (uint8_t)midi;
}

int main(void) {
   uint16_t inputAmplitude;
   uint8_t instrument = 0;
   struct modeModOut output;

   presetInit();

   while (readIO(STDIN_FILENO, &inputAmplitude, sizeof inputAmplitude) > 0) {
      readIO(STDIN_FILENO, &instrument, sizeof instrument);

      output.midiNum = getMidiNum(instrument);

      uint8_t level = map_curve(inputAmplitude);

      output.velocity = level;
      output.cc11 = level;

      dprintf(2, "volumeMod amp=%u inst=%u level=%u\n", inputAmplitude,
              instrument, level);

      writeIO(STDOUT_FILENO, &output, sizeof output);
   }

   return 0;
}
