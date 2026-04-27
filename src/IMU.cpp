#include "IMU.hpp"
#include <iostream>
#include <cmath>

IMU::IMU() : bno08x() {}

IMU::~IMU() {}

bool IMU::begin() {
    if (!bno08x.begin_I2C()) {
        std::cout << "Failed to initialize IMU" << std::endl;
        
        return false;
    }

    std::cout << "IMU initialized" << std::endl;

    // The IMU needs a vector report enabled; Trying at 50Hz
    if (!bno08x.enableReport(SH2_ROTATION_VECTOR, 20000)) {
        std::cout << "Could not enable rotation vector report" << std::endl;
        return false;
    }

    return true;
}

bool IMU::update() {
    if (!bno08x.getSensorEvent(&sensor_value)) {
        return false;
    }

    if (sensor_value.sensorId == SH2_ROTATION_VECTOR) {
        sh2_RotationVector_t* rv = &sensor_value.un.rotationVector;
        current_position_ = quaternion_to_rotational(rv->real, rv->i, rv->j, rv->k);
        return true;
    }

    return false;
}

Orientation IMU::get_orientation() const {
    return current_position_;
}

Orientation IMU::quaternion_to_rotational(float qr, float qi, float qj, float qk) {
    Orientation orient;
    
    float sqr = qr * qr;
    float sqi = qi * qi;
    float sqj = qj * qj;
    float sqk = qk * qk;

    orient.yaw = std::atan2(2.0f * (qi * qj + qk * qr), (sqi - sqj - sqk + sqr)) * 57.29578f;
    orient.pitch = std::asin(-2.0f * (qi * qk - qj * qr) / (sqi + sqj + sqk + sqr) * 57.29578f);
    orient.roll = std::atan2(2.0f * (qj * qk + qi * qr), (-sqi - sqj + sqk + sqr)) * 57.29578f;

    return orient;
}