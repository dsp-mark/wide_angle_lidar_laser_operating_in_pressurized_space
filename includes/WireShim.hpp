#ifndef WIRE_SHIM_HPP
#define WIRE_SHIM_HPP

#include <lgpio.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstdint>

#include "Adafruit_BNO08x.h"

class WireShim {
private:
    static int i2c_fd;
    static int lgpio_h;
    static bool yield_to_lidar = false;
public:
    static bool init(int gpio_chip = 0);
    static void cleanup();
};

extern "C" {
    int i2chal_open(sh2_Hal_t *self);
    int i2chal_read(sh2_Hal_t *self, uint8_t *pBuffer, unsigned len, uint32_t *t_us);
    int i2chal_write(sh2_Hal_t *self, uint8_t *pBuffer, unsigned len);
    void i2chal_close(sh2_Hal_t *self);
}

#endif