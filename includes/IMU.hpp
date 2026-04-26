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

    bool writeBytes(const uint8_t* data, size_t len);

    bool readBytes(uint8_t* data, size_t len);

    bool sendFeatureCommand(uint8_t reportId, uint16_t intervalUs);

    bool readQuarternion(float &qr, float &qi, float &qj, float &qk);

    static Orientation quaternionToYPR(float qr, float qi, float qj, float qk);

    static float wrap180(float a);

public:
    IMU();

    ~IMU();

    bool begin();

    bool update();

    Orientation getOrientation() const;
};

#endif