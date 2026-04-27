#include "IMU.hpp"
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include <cmath>
#include <cstring>
#include <sys/ioctl.h>
#include <cstring>
#include <iostream>

static constexpr uint8_t BNO_ADDR = 0x4A;
static constexpr const char* I2C_DEV = "/dev/i2c-1";
static constexpr uint8_t REPORT_ROTATION_VECTOR = 0x05;

IMU::IMU() = default;

IMU::~IMU() {
    if (i2cFd >= 0) {
        close(i2cFd);
    }
}

bool IMU::begin() {
    i2cFd = open(I2C_DEV, O_RDWR);

    if (i2cFd < 0) {
        std::cout << "Could not find I2C line" << std::endl;
        return false;
    }

    if (ioctl(i2cFd, I2C_SLAVE, BNO_ADDR) < 0) {
        return false;
    }

    uint8_t resetCmd[] = {0x00};

    (void)resetCmd;

    return send_feature_command(REPORT_ROTATION_VECTOR, 5000);
}

bool IMU::update() {
    // These will become the values read in by the IMU
    float qr, qi, qj, qk;

    if (!read_quaternion(qr, qi, qj, qk)) {
        return false;
    }

    // Takes the IMU's quaternion values and stores position as Yaw, Pitch, and Roll
    current_position_ = quaternion_to_rotational(qr, qi, qj, qk);
    
    return true;
}

Orientation IMU::get_orientation() const {
    return current_position_;
}

bool IMU::write_bytes(const uint8_t* data, size_t len) {
    return ::write(i2cFd, data, len) == (ssize_t)len;
}

bool IMU::read_bytes(uint8_t* data, size_t len) {
    return ::read(i2cFd, data, len) == (ssize_t)len;
}

bool IMU::send_feature_command(uint8_t report_id, uint16_t interval_us) {
    uint8_t cmd[12] = {
        0x00, 0x01, 0x00, 0xFD,
        report_id,
        0x00, 0x00, 0x00,
        (uint8_t)(interval_us & 0xFF),
        (uint8_t)(interval_us >> 8),
        0x00, 0x00
    };
    return write_bytes(cmd, sizeof(cmd));
}

bool IMU::read_quaternion(float &qr, float &qi, float &qj, float &qk) {
    uint8_t buf[21] = {0};

    if (!read_bytes(buf, sizeof(buf))) {
        return false;
    }

    int16_t raw_i = (int16_t)((buf[10] << 8) | buf[9]);
    int16_t raw_j = (int16_t)((buf[12] << 8) | buf[11]);
    int16_t raw_k = (int16_t)((buf[14] << 8) | buf[13]);
    int16_t raw_r = (int16_t)((buf[16] << 8) | buf[15]);

    const float scale = 1.0f / 16384.0f;
    qi = raw_i * scale;
    qj = raw_j * scale;
    qk = raw_k * scale;
    qr = raw_r * scale;

    return true;
}

float IMU::wrap_180(float a) {
    while (a > 180.0f) {
        a -= 360.0f;
    }

    while (a < -180.0f) {
        a += 360.0f;
    }

    return a;
}

Orientation IMU::quaternion_to_rotational(float qr, float qi, float qj, float qk) {
    Orientation orient;

    float sqr = qr * qr;
    float sqi = qi * qi;
    float sqj = qj * qj;
    float sqk = qk * qk;

    orient.yaw = std::atan2(2.0f * (qi * qj + qk * qr), (sqi - sqj - sqk + sqr)) * 57.29578f;
    orient.pitch = std::asin(-2.0f * (qi * qk - qj * qr) / (sqi + sqj + sqk + sqr)) * 57.29578f;
    orient.roll = std::atan2(2.0f * (qj * qk + qi * qr), (-sqi - sqj + sqk + sqr)) * 57.29578f;

    return orient;
}