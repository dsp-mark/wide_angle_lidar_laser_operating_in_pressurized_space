#include "IMU.hpp"

#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include <cmath>
#include <cstring>
#include <sys/ioctl.h>
#include <iostream>

static constexpr const char* I2C_DEV = "/dev/i2c-1";

IMU::IMU() = default;

IMU::~IMU() {
    if (i2cFd >= 0) {
        close(i2cFd);
    }
}

bool IMU::begin() {
    i2cFd = open(I2C_DEV, O_RDWR);

    if (i2cFd < 0) {
        std::cout << "Could not find I2C device" << I2C_DEV <<  std::endl;
        return false;
    }

    if (ioctl(i2cFd, I2C_SLAVE, BNO_ADDR) < 0) {
        std::cout << "Could not set BNO085 I2C address" << std::endl;

        return false;
    }

    return send_feature_command(REPORT_ROTATION_VECTOR, 5000);
}

bool IMU::update() {
    uint8_t report[64] = {0};
    size_t len = sizeof(report);

    if (!read_report(report, len)) {
        return false;
    }

    // These will become the values read in by the IMU
    float qr, qi, qj, qk;

    if (!parse_rotation_vector(report, len, qr, qi, qj, qk)) {
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

bool IMU::read_report(uint8_t* report, size_t& len) {
    len = 21;

    if (!read_bytes(report, len)) {
        return false;
    }

    return true;
}

bool IMU::parse_rotation_vector(const uint8_t* report, size_t len, float &qr, float &qi, float &qj, float &qk) {
    if (len < 17) {
        return false;
    }

    int16_t raw_i = (int16_t)((report[10] << 8) | report[9]);
    int16_t raw_j = (int16_t)((report[12] << 8) | report[11]);
    int16_t raw_k = (int16_t)((report[14] << 8) | report[13]);
    int16_t raw_r = (int16_t)((report[16] << 8) | report[15]);

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