#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cstring>

static constexpr const char* I2C_DEV = "/dev/i2c-1";
static constexpr uint8_t BNO_ADDR = 0x4A;
static constexpr uint8_t REPORT_ROTATION_VECTOR = 0x05;

static bool write_bytes(int fd, const uint8_t* data, size_t len) {
    return ::write(fd, data, len) == (ssize_t)len;
}

static bool enable_rotation_vector(int fd) {
    uint8_t cmd[12] = {
        0x00, 0x01, 0x00, 0xFD,
        REPORT_ROTATION_VECTOR, 
        0x00, 0x00, 0x00,
        0x88, 0x13, 0x00, 0x00
    };

    return write_bytes(fd, cmd, sizeof(cmd));
}

static void dump_hex(const uint8_t* buf, size_t len) {
    for (size_t i=0; i < len; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(buf[i]) << " ";
    }

    std::cout << std::dec << "\n";
}

int main() {
    int fd = open(I2C_DEV, O_RDWR);

    if (fd < 0) {
        std::cerr << "Failed to open " << I2C_DEV << "\n";
        return 1;
    }

    if (ioctl(fd, I2C_SLAVE, BNO_ADDR) < 0) {
        std::cerr << "Failed to set I2C address" << std::endl;
        close(fd);
        return 1;
    }

    if (!enable_rotation_vector(fd)) {
        std::cerr << "Failed to enable rotation vector report" << std::endl;
        close(fd);
        return 1;
    }

    std::cout << "Reading raw packets" << std::endl;

    for (int n = 0; n < 200; ++n) {
        uint8_t buf[64] = {0};
        ssize_t r = read(fd, buf, sizeof(buf));

        if (r <= 0){
            std::cerr << "Read failed" << std::endl;
            break;
        }

        std::cout << "Packet len=" << r << " : ";
        dump_hex(buf, static_cast<size_t>(r));
        usleep(20000);
    }

    close(fd);
    return 0;
}