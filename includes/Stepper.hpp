#ifndef STEPPER_HPP
#define STEPPER_HPP

#include "TMC2209.hpp"
#include <lgpio.h>
#include <cmath>

// Azimuth and Altitude now both use Stepper class
class Stepper {
public:
    int gpio_handle;
	int step_pin, dir_pin;
	TMC2209 tmc;
    int steps_per_unit;
	
	Stepper(int h, int step, int dir, uint8_t tmc_addr, int steps_per = 400) : gpio_handle(h), step_pin(step), dir_pin(dir), tmc(tmc_addr), steps_per_unit(steps_per) {
        lgGpioClaimOutput(gpio_handle, 0, step_pin, 0);
        // **********************
        lgGpioClaimOutput(gpio_handle, 9, dir_pin, 0);
        lgGpioWrite(gpio_handle, step_pin, 0);
	}
	
	void configure() {
		// Steppers always use a 600mA, 1/16 microstep TMC2209 set up
		tmc.configure_stepper(600, 16);
	}
	
	// Move a single step in a given direction
	void move(int dir, int pulse_us = 1000) {
		lgGpioWrite(gpio_handle, dir_pin, dir > 0 ? 1 : 0);
        lgGpioWrite(gpio_handle, step_pin, 1);
        lguSleep(0.000001 * pulse_us);
        lgGpioWrite(gpio_handle, step_pin, 0);
        lguSleep(0.000001 * pulse_us); 
	}

	// Move n steps in a given direction
	void move_steps(int steps, int pulse_us = 1000) {
		// Use the sign of steps to move clockwise (1) or counterclockwise (-1)
		int dir = (steps >= 0) ? 1 : -1;
		int count = std::abs(steps);

		for (int i = 0; i < count; ++i){
			move(dir, pulse_us);
		}
	}

    void move_degrees(double degrees, int pulse_us = 1000) {
        move_steps(static_cast<int>(degrees * steps_per_unit), pulse_us);
    }

    void raster_scan(double degrees, int pulse_us = 1000) {
        std::cout << "Raster: - " << right_degrees << " to 0 to +" << right degrees << std::endl;

        move_degrees(-degrees, pulse_us);

        lguSleep(0.1);

        move_degrees(+degrees, pulse_us);

        lguSleep(0.1);

        move_degrees(+degrees, pulse_us);

        lguSleep(0.1);

        move_degrees(-degrees, pulse_us);
    }
};

#endif