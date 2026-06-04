#include "i2c.h"

int i2cOpen()
{
   int fd = open("/dev/i2c-1", O_RDWR);
   if (fd < 0)
   {
      perror("open");
      return -1;
   }
   return fd;
}

void i2cWrite16(int fd, uint8_t address, uint8_t reg, uint16_t data)
{
   struct i2c_rdwr_ioctl_data packets;
   struct i2c_msg m;
   uint8_t buf[3];
   data = htons(data);
   buf[0] = reg;
   memcpy(&buf[1], &data, sizeof data);
   m.addr = address;
   m.flags = 0;
   m.len = 3;
   m.buf = buf;

   packets.msgs = &m;
   packets.nmsgs = 1;

   if (ioctl(fd, I2C_RDWR, &packets) < 0)
   {
      perror("I2C_RDWR failed");
   }
}

void i2cRead16(int fd, uint8_t address, uint8_t reg, uint16_t *data)
{
   struct i2c_rdwr_ioctl_data packets;
   struct i2c_msg m[2];

   m[0].addr = address;
   m[0].flags = 0;
   m[0].len = 1;
   m[0].buf = &reg;

   m[1].addr = address;
   m[1].flags = I2C_M_RD;
   m[1].len = 2;
   m[1].buf = (uint8_t *)data;

   packets.msgs = m;
   packets.nmsgs = 2;

   if (ioctl(fd, I2C_RDWR, &packets) < 0)
   {
      perror("I2C_RDWR failed");
   }

   *data = ntohs(*data);
}
