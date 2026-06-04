#define _GNU_SOURCE

#include "helperIO.h"
#include "midiFun.h"
#include <asm/termbits.h>
#include <fcntl.h>
#include <gpiod.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define CHIP "/dev/gpiochip0"
#define GPIO_PIN 27

int main() {
   int fd = open("/dev/serial0", O_RDWR | O_NOCTTY);
   if (fd < 0) {
      perror("open");
      return 1;
   }

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

   struct termios2 options;

   ioctl(fd, TCGETS2, &options);

   options.c_cflag &= ~CBAUD;
   options.c_cflag |= BOTHER;

   options.c_ispeed = 31250;
   options.c_ospeed = 31250;

   options.c_cflag &= ~PARENB;
   options.c_cflag &= ~CSTOPB;
   options.c_cflag &= ~CSIZE;
   options.c_cflag |= CS8;
   options.c_cflag |= (CLOCAL | CREAD);

   ioctl(fd, TCSETS2, &options);

   uint8_t byteRead;
   while (readIO(STDIN_FILENO, &byteRead, sizeof byteRead) > 0) {
      //      gpiod_line_request_set_value(request, GPIO_PIN,
      //      GPIOD_LINE_VALUE_ACTIVE);
      writeIO(fd, &byteRead, sizeof byteRead);
      //      usleep(1000);
      //      gpiod_line_request_set_value(request, GPIO_PIN,
      //                                   GPIOD_LINE_VALUE_INACTIVE);
   }

   close(fd);
   return 0;
}
