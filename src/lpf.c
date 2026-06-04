
#include "helperIO.h"
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#define A 672
#define B 164

int main(void) {
   int16_t y_prev = 0;
   int16_t x_prev = 0;
   int16_t y = 0;
   int16_t x = 0;
   int64_t temp;
   while (readIO(STDIN_FILENO, &x, sizeof x)) {
      temp = y_prev * A + (x + x_prev) * B;
      temp /= 1000;
      if (temp >= INT16_MAX)
         y = INT16_MAX;
      else if (temp <= INT16_MIN)
         y = INT16_MIN;
      else
         y = (int16_t)temp;
      y_prev = y;
      x_prev = x;
      writeIO(STDOUT_FILENO, &y, sizeof y);
   }
}