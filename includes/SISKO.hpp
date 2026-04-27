#ifndef SISKO_HPP
#define SISKO_HPP

#include "IMU.hpp"
#include "Stepper.hpp"
#include "ScanLogger.hpp"

// Stepper IMU Start Keeping and Optimisation
class SISKO {
private:
    IMU imu;
    Stepper azimuth;
    Stepper altitude;
    ScanLogger logger;
    SensorManager* lidar;

    double lidar_az_positions[] = {-30, -15, 0, 15, 30};
    double lidar_alt_positions[] = {-20, 0, 20};


    float drift_gain = 0.2f;
    float drift_threshold_deg = 0.5f;

    static float wrap180_local(float a);

public:
    SISKO(int gpio_handle, int az_step, int az_dir, uint8_t az_tmc_addr, double az_steps_per_deg, double az_default_deg, int alt_step, int alt_dir, uint8_t alt_tmc_addr, double alt_steps_per_deg, double alt_default_deg);

    bool begin();
    bool update_imu();
    
    Orientation get_orientation() const;

    void correct_drift();
    void scan_once();
    void home();
    
    ScanLogger& get_logger();
};

#endif