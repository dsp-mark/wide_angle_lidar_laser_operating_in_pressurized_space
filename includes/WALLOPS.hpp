#ifndef WALLOPS_HPP
#define WALLOPS_HPP

#include "SISKO.hpp"
#include "SensorManager.hpp"
#include "ScanLogger.hpp"

// Wide Angle LiDAR Laser Operating in Pressurized Space
class WALLOPS {
private:
    SISKO sisko;
    SensorManager sensors;
    ScanLogger logger;

    std::vector<double> az_scan_positions = {-30, -15, 0, 15, 30};
    std::vector<double> alt_scan_positions = {-20, 0, 20};

public:

    WALLOPS(int gpio_handle, const std::vector<SensorConfig>& lidar_configs, int az_step, int_az_dir, uint8_t az_tmc_addr, double az_steps_per_deg, double az_default_deg, int alt_step, int alt_dir, uint8_t alt_tmc_addr, double alt_steps_per_deg, double alt_default_deg);

    bool begin();

    void scan_once();

    void home_all();
    
    void emergency_stop();

    ScanLogger& get_logger() {
        return logger;
    }
};

#endif