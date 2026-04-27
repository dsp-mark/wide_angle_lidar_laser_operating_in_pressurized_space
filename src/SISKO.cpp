#include "SISKO.hpp"

#include <cmath>
#include <iostream>

static float wrap180_local(float a) {
    while (a > 180.0f) {
        a -= 360.0f;
    }
    while (a < -180.0f) {
        a += 360.0f;
    }
    return a;
}

SISKO::SISKO(int gpio_handle, int az_step, int az_dir, uint8_t az_tmc_addr, double az_steps_per_deg, double az_default_deg, int alt_step, int alt_dir, uint8_t alt_tmc_addr, double alt_steps_per_deg, double alt_default_deg) : imu(), 
                   azimuth(gpio_handle, az_step, az_dir, az_tmc_addr, az_steps_per_deg, az_default_deg), 
                   altitude(gpio_handle, alt_step, alt_dir, alt_tmc_addr, alt_steps_per_deg, alt_default_deg) {}

bool SISKO::begin() {
    azimuth.configure();
    altitude.configure();

    std::cout << "SISKO is waking up and ready for his raktajino!" << std::endl;

    logger.log("SISKO is waking up and ready for his raktajino!");

    if (!imu.begin()) {
        std::cout << "However, the IMU failed to start" << std::endl;

        logger.log("However, the IMU failed to start");

        return false;
    }

    std::cout << "IMU started; SISKO is ready for navigation" << std::endl;
    logger.log("IMU started; SISKO is ready for navigation");

    return true;
}

bool SISKO::update_imu() {
    return imu.update();
}

Orientation SISKO::get_orientation() const {
    return imu.get_orientation();
}

void SISKO::correct_drift() {
    Orientation orient = imu.get_orientation();

    double az_expected = azimuth.get_position();
    double alt_expected = altitude.get_position();

    float az_err = wrap180_local(orient.yaw - az_expected);
    float alt_err = wrap180_local(orient.pitch - alt_expected);

    if (std::fabs(az_err) > drift_threshold_deg) {
        double correction = drift_gain * az_err;
        azimuth.move_degrees(correction, 1000);

        std::cout << "AZ drift correction: err_deg=" + std::to_string(az_err) + ", correction_deg=" + std::to_string(correction) << std::endl;

        logger.log("AZ drift correction: err_deg=" + std::to_string(az_err) + ", correction_deg=" + std::to_string(correction));
    }

    if (std::fabs(alt_err) > drift_threshold_deg) {
        double correction = drift_gain * alt_err;
        altitude.move_degrees(correction, 1000);

        std::cout << "ALT drift correction: err_deg=" + std::to_string(alt_err) + ", correction_deg=" + std::to_string(correction) << std::endl;

        logger.log("ALT drift correction: err_deg=" + std::to_string(alt_err) + ", correction_deg=" + std::to_string(correction));
    }
}

void SISKO::scan_once(int pulse_us) {
    auto t0 = logger.start_scan();

    if (update_imu()) {
        correct_drift();
    } else {
        logger.log("IMU was not updated");
    }

    azimuth.move_degrees(-azimuth.default_degrees, pulse_us);
    altitude.raster_scan(pulse_us);

    azimuth.move_degrees(+azimuth.default_degrees, pulse_us);
    altitude.raster_scan(pulse_us);

    logger.log_scan(t0);
}

void SISKO::home(int pulse_us) {
    logger.log("SISKO is returning home");
    azimuth.home(pulse_us);
    altitude.home(pulse_us);
    logger.log("SISKO is home");
}

ScanLogger& SISKO::get_logger() {
    return logger;
}
