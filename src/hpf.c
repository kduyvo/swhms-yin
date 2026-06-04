
#include "helperIO.h"
#include "volumeModulation.h"
#include <gpiod.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define CHIP "/dev/gpiochip0"
#define GPIO_PIN 17

#define A 987

int main(void) {
   struct gpiod_chip *chip = NULL;
   struct gpiod_line_settings *settings = NULL;
   struct gpiod_line_config *line_cfg = NULL;
   struct gpiod_request_config *req_cfg = NULL;
   struct gpiod_line_request *request = NULL;

   unsigned int offset = GPIO_PIN;

   chip = gpiod_chip_open(CHIP);
   if (!chip) {
      perror("gpiod_chip_open");
      return 1;
   }

   settings = gpiod_line_settings_new();
   if (!settings) {
      perror("gpiod_line_settings_new");
      return 1;
   }

   gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
   gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE);

   line_cfg = gpiod_line_config_new();
   if (!line_cfg) {
      perror("gpiod_line_config_new");
      return 1;
   }

   if (gpiod_line_config_add_line_settings(line_cfg, &offset, 1, settings) <
       0) {
      perror("gpiod_line_config_add_line_settings");
      return 1;
   }

   req_cfg = gpiod_request_config_new();
   if (!req_cfg) {
      perror("gpiod_request_config_new");
      return 1;
   }

   gpiod_request_config_set_consumer(req_cfg, "latency-test");

   request = gpiod_chip_request_lines(chip, req_cfg, line_cfg);
   if (!request) {
      perror("gpiod_chip_request_lines");
      return 1;
   }
   int16_t y_prev = 0;
   int16_t x_prev = 0;
   int16_t y = 0;
   int16_t x = 0;
   int64_t temp;
   while (readIO(STDIN_FILENO, &x, sizeof x)) {
      // if (abs((int)x) > 3000) {
      //    gpiod_line_request_set_value(request, GPIO_PIN,
      //                                 GPIOD_LINE_VALUE_ACTIVE);
      // } else {
      //    gpiod_line_request_set_value(request, GPIO_PIN,
      //                                 GPIOD_LINE_VALUE_INACTIVE);
      // }
      temp = A * (y_prev + x - x_prev);
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
