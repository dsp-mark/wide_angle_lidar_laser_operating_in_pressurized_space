#ifndef WALLOPS_HPP
#define WALLOPS_HPP

// Wide Angle LiDAR Laser Operating in Pressurized Space
class WALLOPS {
private:
    SISKO sisko;
    SensorManager sensors;
public:
    void scan_once() {
        sisko.update_imu();
        sisko.apply_drift_correction();

        sensors.run_one_frame();

        sisko.scan_once();
    }
};

#endif