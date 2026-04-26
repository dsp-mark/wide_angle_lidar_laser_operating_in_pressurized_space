#ifndef VL53L8CX_SENSOR_HPP
#define VL53L8CX_SENSOR_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <cstring>
#include <cmath>
#include <mutex>

#include <chrono>
#include <iomanip>
#include <sstream>

extern "C" {
#include "vl53l8cx_api.h"
#include "platform.h"
}

// Used for adding the date and time to the csv file name
static std::string timestamp_string() {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_r(&t, &tm);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
    return oss.str();
}

// Used for recording time since boot (TE-1)
static double seconds_since_boot() {
    std::ifstream f("/proc/uptime");
    double uptime = 0.0;
    if (f >> uptime) {
        return uptime;
    }
    return 0.0;
}


class VL53L8CX_Sensor {
private:
    VL53L8CX_Configuration dev {};
    VL53L8CX_ResultsData results{};
    std::string sensor_name;
    std::ofstream csvFile;
    int reset_pin;
    inline static std::mutex i2cMutex;
    std::mutex csvMutex;

    const float fov = 45.0f;
    const float fov_rad = fov * (M_PI / 180.0f);
    const int res = 8;
    const uint32_t signal_threshold = 100;


    void process_frame(int frame) {
        std::lock_guard<std::mutex> lock(csvMutex);
        
        for (int r = 0; r < res; r++) {
            for (int c = 0; c < res; c++) {
                int zone = r * res + c;
                
                int idx = VL53L8CX_NB_TARGET_PER_ZONE * zone;
                uint8_t status = results.target_status[idx];
                float d = (float)results.distance_mm[idx];
                uint32_t signal = results.signal_per_spad[idx];

                if ((status == 5 || status == 6 || status == 9) && signal >= signal_threshold) {
                    float angle_x = ((float)c - (res - 1) / 2.0f) * (fov_rad / res);
                    float angle_y = ((res - 1) / 2.0f - (float)r) * (fov_rad / res);
                    float x = d * tan(angle_x);
                    float y = d * tan(angle_y);

                    double t_boot = seconds_since_boot();
                    csvFile << frame << "," << t_boot << "," << zone << "," << (int)status << "," 
                            << (int)d << "," << signal << "," << x << "," << y << "," << d << "\n";
                    std::cout << "[" << sensor_name << "] " << frame << "," << t_boot << "," << zone << "," << (int)status << "," 
                            << (int)d << "," << signal << "," << x << "," << y << "," << d << std::endl;
                }
            }
        }
        csvFile.flush();
    }

public:
    VL53L8CX_Sensor(std::string name, int lp_pin) : sensor_name(std::move(name)), reset_pin(lp_pin) {}

    const std::string& get_name() {
        return sensor_name;
    }

    void power_on() {
        system(("sudo pinctrl set " + std::to_string(reset_pin) + " op pn dl").c_str());
        usleep(200000);
        system(("sudo pinctrl set " + std::to_string(reset_pin) + " op pn dh").c_str());
        usleep(600000);
    }

    void power_off() {
        system(("sudo pinctrl set " + std::to_string(reset_pin) + " op pn dl").c_str());
        usleep(100000);
    }


    bool begin(uint8_t target_addr_7bit) {
        std::lock_guard<std::mutex> lock(i2cMutex);

        memset(&dev, 0, sizeof(dev));
        uint8_t is_alive = 0;

        const uint16_t default_addr_8bit = 0x52; // Used to be 0x52
        const uint16_t target_addr_8bit = static_cast<uint16_t>(target_addr_7bit << 1);

        // Checking to see if the sensor is at the default address (it should be)
        // dev.platform.address = 0x29;
        dev.platform.address = default_addr_8bit;
        if (vl53l8cx_comms_init(&dev.platform) != 0) {
            std::cerr << sensor_name << ": comms init failed at default address\n";

            return false;
        }

        std::cout << "Initialized comms" << std::endl;

        if (vl53l8cx_is_alive(&dev, &is_alive) != 0 || !is_alive) {
            std::cerr << sensor_name << ": device not alive at 0x29\n";
            vl53l8cx_comms_close(&dev.platform);
            return false;
        }

        std::cout << "Found " << sensor_name << " at 0x29" << std::endl;

        if (vl53l8cx_set_i2c_address(&dev, target_addr_8bit) != 0) {
            std::cerr << sensor_name << ": address change failed\n";
            vl53l8cx_comms_close(&dev.platform);
            return false;
        }

        // vl53l8cx_comms_close(&dev.platform);
        usleep(100000); 

        // Setting sensor to new address
        dev.platform.address = target_addr_8bit; 
        
        if (vl53l8cx_init(&dev) != 0) {
            std::cerr << sensor_name << ": sensor init failed\n";
            vl53l8cx_comms_close(&dev.platform);
            return false;
        }

        if (vl53l8cx_set_resolution(&dev, VL53L8CX_RESOLUTION_8X8) != 0) {
            std::cerr << sensor_name << ": set resolution failed\n";
            vl53l8cx_comms_close(&dev.platform);
            return false;
        }
        
        std::string dated_csv_name = sensor_name + "_" + timestamp_string() + ".csv";
        csvFile.open(dated_csv_name, std::ios::out | std::ios::trunc);

        if (!csvFile.is_open()) {
            std::cerr << sensor_name << ": CSV open failed\n";
            vl53l8cx_comms_close(&dev.platform);
            return false;
        }
        
        csvFile << "Frame,BootTime_s,Zone,Status,Distance_mm,Signal_kcps,X,Y,Z\n";
        std::cout << sensor_name << " brought up at 0x" << std::hex << static_cast<int>(target_addr_7bit) << std::dec << "\n";

        return true;
    }


    bool start() {
        std::lock_guard<std::mutex> lock(i2cMutex);
        int status = vl53l8cx_start_ranging(&dev);
        if (status != 0) {
            std::cerr << sensor_name << ": start ranging failed. Error: " << status << std::endl;
            return false;
        }
        return true;
    }

    bool poll_once(int frame) {
        std::lock_guard<std::mutex> lock(i2cMutex);

        uint8_t is_ready = 0;
        if (vl53l8cx_check_data_ready(&dev, &is_ready) != 0) {
            return false;
        }

        if (!is_ready) {
            return false;
        }

        if (vl53l8cx_get_ranging_data(&dev, &results) != 0) {
            return false;
        }

        process_frame(frame);
        return true;
    }

    void stop () {
        std::lock_guard<std::mutex> lock(i2cMutex);
        vl53l8cx_stop_ranging(&dev);
    }

    void close() {
        std::lock_guard<std::mutex> lock(i2cMutex);
        vl53l8cx_comms_close(&dev.platform);
        if (csvFile.is_open()) {
            csvFile.close();
        }
    }
};

#endif