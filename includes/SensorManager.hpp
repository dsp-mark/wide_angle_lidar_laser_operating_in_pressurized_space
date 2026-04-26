#ifndef SENSOR_MANAGER_HPP
#define SENSOR_MANAGER_HPP

#include <vector>
#include <memory>
#include <iostream>
#include <unistd.h>
#include "VL53L8CX_Sensor.hpp"

// Used to keep track of an arbitrary number of VL53L8CX sensors in a vector
struct SensorConfig {
    std::string name;
    int reset_pin;
    uint8_t address; // NOTE: THIS IS 7-BIT I2C
};

class SensorManager {
private:
    std::vector<SensorConfig> configs;
    std::vector<std::unique_ptr<VL53L8CX_Sensor>> sensors;
    int total_frames;

    void set_scan_led (bool on) {
        if (on) {
            system("sudo pinctrl set 9 op dh");
        } else {
            system("sudo pinctrl set 9 op dl");
        }
    }

public:
    SensorManager(const std::vector<SensorConfig>& sensor_list, int frames = 100) : configs(sensor_list), total_frames(frames) {
            
        // Taking in the list of sensors and keeping track of their information
            for (const auto& curr_sensor : configs) {
                sensors.push_back(std::make_unique<VL53L8CX_Sensor>(curr_sensor.name, curr_sensor.reset_pin));
            }
        }

    bool initialize() {
        if (sensors.empty()) {
            std::cerr << "No sensors configured" << std::endl;
            return false;
        }

        if (configs.size() != sensors.size()) {
            std::cerr << "Config/sensor count mismatch" << std::endl;
            return false;
        }

        std::cout << "Resetting all sensors" << std::endl;

        for (auto& curr_sensor: sensors) {
            curr_sensor->power_off();
        }

        sleep(2);

        for (size_t i = 0; i < sensors.size(); ++i) {
            std::cout << "Waking up " << configs[i].name << std::endl;
            sensors[i]->power_on();
            sleep(1);
            
            if (!sensors[i]->begin(configs[i].address)) {
                std::cerr << configs[i].name << " failed to initialize" << std::endl;
                return false;
            }
        }

        set_scan_led(true);

        for (auto& curr_sensor : sensors) {
            if (!curr_sensor->start()) {
                std::cerr << curr_sensor->get_name() << " failed to start ranging" << std::endl;
                return false;
            }
        }

        return true;
    }

    void run() {
        std::vector<int> frames_done(sensors.size(), 0);

        std::cout << "Polling round robin" << std::endl;

        while (true) {
            bool any_active = false;

            for (size_t i = 0; i < sensors.size(); ++i) {
                if (frames_done[i] < total_frames) {
                    any_active = true;

                    if (sensors[i]->poll_once(frames_done[i])) {
                        ++frames_done[i];
                    }
                }
            }

            if (!any_active) {
                break;
            }

            usleep(5000);
        }
    }

    void shutdown() {
        for (auto& curr_sensor : sensors) {
            curr_sensor->stop();
        }
        for (auto& curr_sensor : sensors) {
            curr_sensor->close();
        }
        for (auto& curr_sensor : sensors) {
            curr_sensor->power_off();
        }

        set_scan_led(false);
    }
};

#endif