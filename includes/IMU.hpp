#ifndef IMU_HPP
#define IMU_HPP

#include <cstdint>
#include <cstddef>

struct Orientation {
    float yaw = 0.0f;
    float pitch = 0.0f;
    float roll = 0.0f;
};

// Wrapper for the Official Adafruit BNO085 found here: https://github.com/adafruit/Adafruit_BNO08x
class IMU {
private:
    Adafruit_BNO08x bno08x;
    sh2_SensorValue_t sensor_value;
    Orientation current_position_;
public:
    IMU();
    ~IMU();

    bool begin();
    bool update();

    Orientation get_orientation() const;
    static Orientation quaternion_to_rotational(float qr, float qi, float qj, float qk);

    
};

#endif