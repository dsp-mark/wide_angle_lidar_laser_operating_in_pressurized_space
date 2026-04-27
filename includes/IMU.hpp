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

    static constexpr uint8_t BNO_ADDR = 0x4A;
    static constexpr uint8_t REPORT_ROTATION_VECTOR = 0x05;

    bool write_bytes(const uint8_t* data, size_t len);

    bool read_bytes(uint8_t* data, size_t len);

    bool send_feature_command(uint8_t report_id, uint16_t interval_us);

    bool read_report(uint8_t* report, size_t& len);

    bool parse_rotation_vector(const uint8_t* report, size_t len, float &qr, float &qi, float &qj, float &qk);

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