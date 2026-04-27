#ifndef IMU_HPP
#define IMU_HPP

#include <cstdint>
#include <cstddef>

struct Orientation {
    float yaw = 0.0f;
    float pitch = 0.0f;
    float roll = 0.0f;
};

class IMU {
private:
    int i2cFd = -1;
    Orientation current_position_;

    bool write_bytes(const uint8_t* data, size_t len);

    bool read_bytes(uint8_t* data, size_t len);

    bool send_feature_command(uint8_t reportId, uint16_t intervalUs);

    bool read_quaternion(float &qr, float &qi, float &qj, float &qk);

    // Takes in a quaternion and returns yaw, pitch, and roll
    static Orientation quaternion_to_rotational(float qr, float qi, float qj, float qk);

    static float wrap_180(float a);

public:
    IMU();

    ~IMU();

    bool begin();

    bool update();

    Orientation get_orientation() const;
};

#endif