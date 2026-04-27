#include "WireShim.hpp"
#include <iostream>
#include <cstring>

int WireShim::i2c_fd = -1;
int WireShim::lgpio_h = -1;

bool WireShim::init(int gpio_chip) {
    lgpio_h = lgGpiochipOpen(gpio_chip);

    if (lgpio_h < 0) {
        std::cerr << "Failed to open gpiochip" << std::endl;
        return false;
    }

    i2c_fd = open("/dev/i2c-1", O_RDWR);
    if (i2c_fd < 0) {
        std::cerr << "Failed to open /dev/i2c-1" << std::endl;
        lgGpiochipClose(lgpio_h);
        return false;
    }

    if (ioctl(i2c_fd, I2C_SLAVE, 0x4A) < 0) {
        std::cerr << "Failed to set BNO085 I2C address " << std::endl;
        close(i2c_fc);
        lgGpiochipClose(lgpio_h);
        return false;
    }

    std::cout << "WireShim initialized" << std::endl;
    return true;
}

void WireShim::cleanup() {
    if (i2c_fd >= 0) {
        close(i2c_fd);
        i2c_fd = -1;
    }

    if (lgpio_h >= 0) {
        lgGpiochipClose(lgpio_h);
        lgpio_h = -1;
    }
}

extern "C" {

int i2chal_open(sh2_Hal_t *self) {
    uint8_t softreset[] = {0x05, 0x00, 0x01, 0x00, 0x01};
    if (write(WireShim::i2c_fd, softreset, 5) != 5) {
        return -1;
    }
    usleep(300000);
    return 0;
}

void i2chal_close(sh2_Hal_t *self) {}

int i2chal_write(sh2_Hal_t *self, uint8_t *pBuffer, unsigned len) {
    return write(WireShim::i2c_fd, pBuffer, len);
}

int i2chal_read(sh2_Hal_t *self, uint8_t *pBuffer, unsigned len, uint32_t *t_us) {
    uint8_t header[4];
    if (read(WireShim::i2c_fd, header, 4) != 4) {
        return 0;
    }

    uint16_t packet_size = ((uint16_t)header[1] << 8) | header[0];
    packet_size &= ~0x8000;

    if (packet_size > len) {
        return 0;
    }

    if (read(WireShim::i2c_fd, pBuffer, packet_size) != (ssize_t)packet_size) {
        return 0;
    }

    *t_us = micros();
    return packet_size;
}

}