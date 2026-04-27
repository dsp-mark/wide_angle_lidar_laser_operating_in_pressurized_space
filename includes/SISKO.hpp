#ifndef SISKO_HPP
#define SISKO_HPP

#include "IMU.hpp"
#include "Stepper.hpp"

// Stepper IMU Start Keeping and Optimisation
class SISKO {
private:
    IMU imu;
    Stepper azimuth;
    Stepper altitude;

    float drift_gain = 0.2f;
    float drift_threshold_deg = 0.5f;

public:
    SISKO(int gpio_handle, int az_step, int az_dir, uint8_t az_tmc_addr, double az_steps_per_deg, double az_default_deg, int alt_step, int alt_dir, uint8_t alt_tmc_addr, double alt_steps_per_deg, double alt_degault_deg);

    bool begin();
    bool update_imu();
    void scan_once();
    void home();
    void correct_drift();
    Orientation get_orientation() const;
};

#endif