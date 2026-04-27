IMU imu;

int main() {
    if (!imu.begin()) {
        return 1;
    }

    while (true) {
        if (imu.update()) {
            Orientation orient = imu.get_orientation();
            // Compare o.yaw and o.pitch to expected gimbal pose.
        }
    }
}