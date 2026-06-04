#include "keypad.h"

enum buttonState { low, rising, high, falling };

struct button {
   uint8_t currentState;
   uint8_t count;
};

struct keypad {
   uint16_t output;
   struct button buttons[16];
};

struct keypad kp;

int main(void) {
   int fd = i2cOpen();
   i2cWrite16(fd, ADDRESS, IODIRA, 0xFFFF);
   i2cWrite16(fd, ADDRESS, GPPUA, 0xFFFF);
   keyPadInit();
   // usleep(1000000);
   keyPadStateMachine(fd);
}

void keyPadStateMachine(int fd) {
   uint16_t data;
   uint8_t dataBit;
   while (1) {
      i2cRead16(fd, ADDRESS, GPIOA, &data);
      for (uint8_t button = 0; button < BUTTONS; button++) {
         // outputBit = (kp.output & (1 << button)) != 0;
         dataBit = (data & (1 << button)) != 0;
         switch (kp.buttons[button].currentState) {
            case low:
               if (dataBit == 1) {
                  kp.buttons[button].currentState = rising;
                  kp.buttons[button].count++;
               }
               break;
            case rising:
               if (dataBit == 1) {
                  kp.buttons[button].count++;
               } else {
                  kp.buttons[button].count = 0;
                  kp.buttons[button].currentState = low;
               }
               if (kp.buttons[button].count == THRESHOLD) {
                  kp.buttons[button].currentState = high;
                  kp.buttons[button].count = 0;
                  kp.output |= (1 << button);
                  // printf("%d\n", kp.output);
               }
               break;
            case falling:
               if (dataBit == 0) {
                  kp.buttons[button].count++;
               } else {
                  kp.buttons[button].count = 0;
                  kp.buttons[button].currentState = high;
               }
               if (kp.buttons[button].count == THRESHOLD) {
                  kp.buttons[button].currentState = low;
                  kp.buttons[button].count = 0;
                  // kp.output &= ~(1 << button);
                  writeIO(STDOUT_FILENO, &button, sizeof button);
                  //printf("%d\n", button);
               }
               break;
            case high:
               if (dataBit == 0) {
                  kp.buttons[button].currentState = falling;
                  kp.buttons[button].count++;
               }
               break;
            default:
               break;
         }
      }
      usleep(10000);
   }
}

void keyPadInit(void) {
   kp.output = 0xFFFF;
   for (int i = 0; i < BUTTONS; i++) {
      kp.buttons[i].currentState = high;
      kp.buttons[i].count = 0;
   }
}
