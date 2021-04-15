/*
 * motor.h
 *
 *  Created on: May 4, 2017
 *      Author: addaon
 */

#ifndef MOTOR_H_
#define MOTOR_H_

#include "events.h"
#include "stm32f4xx_hal.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <tuple>

/* SPEED CONTROL STRATEGY:

For each motor, we take an interrupt on each Hall sensor transition.

Interrupt rate analysis: A motor has 10 pole pairs, and 6 hall Sensor
transitions per rotation per pole pair, so 60 interrupts per rotation.
A motor has a 15 cm diameter, so at 10 m/s we take 4000 interrupts/s
per motor, or 12000 interrupts/s total. This is acceptable.

When we take an interrupt, we advance the Hall sensor state machine.
One of three things is determined to have happened: The motor moved
forward, the motor moved backwards, or the motor took an illegal state
transition. The latter should only be observed due to noise or if a
second transition happens before the interrupt for the first is fully
processed. By the previous interrupt rate analysis, we have nearly
80 microseconds to process each interrupt, so it’s reasonable to treat
this as a real-time violation error.

Within each 1 ms processor tick, we accumulate a signed count of the
number of times the motor has moved forward (if negative, backwards).

Range analysis: With the numbers from the interrupt rate analysis,
a motor at 10 m/s will generate 4 counts in one millisecond. Therefore
a int8_t is much more than sufficient. We have space to reserve a value
for the error condition; we will use numeric_limits<int8_t>::min() both
to make positive and negative non-error ranges equal and to allow simpler
saturation logic.

Overflow of the signed count within a tick should be impossible by this
range analysis, and is thus treated as an overflow error, which is not
distinguished in handling from a real-time violation error. Because the
tick can only be incremented or decremented by one, it is sufficient to
allow it to overflow in both the positive and negative direction to
numeric_limits<int8_t>::min(), as long as it is not further incremented
or decremented once it contains an error value.

We use a PID controller to regulate the speed of the motor. Trying to
maintain a requested number of Hall sensor transitions per millisecond
is unviable because of the low velocity resolution. Instead, we accumulate
the Hall sensor transition count over a longer window of time. The longer
the window the higher velocity resolution we have, but the slower the
dynamics of our control loop are.

Window analysis: As above, at our maximum speed of 10 m/s, we get 4 Hall
sensor transitions per millisecond. 10 mm/s seems a likely minimum speed,
and gives a 10-bit dynamic range for velocity. At 10 mm/s, we get 4 Hall
sensor transitions per second. We could therefore accumulate for 250 ms,
and control "number of Hall sensor transitions per 250 ms". However,
when running at minimum speed this would mean that the minimum perceived
error was 100%. To get this down to a more reasonable 25%, we can
accumulate for 1000 ms, giving "number of Hall sensor transitions per
second" with an expected acceptable velocity range of 4 - 4000.

For each 1 ms processor tick, for each motor, we look at the accumulated
signed count. If it is the discriminated error value, we record that an
error occurred for this motor and discard the count value.

Safety analysis: Continuing to use the last known velocity means that we
will not, for example, detect that the motor is over-speeding. Were this
to happen to the robot as a whole, rather than just one motor, this could
cause us to significantly overrun the planned trajectory. To maintain
comfort here, it is sufficient to require that the velocity not be "too
stale". If we treat back-to-back errors as a hard error, then the worst
case for soft errors is alternating non-error and error signed counts.
If we instantaneously transition from zero velocity to 10 m/s, and our
set velocity is 5 m/s, it would take 500 ms for our accumulated velocity
to reach the set point under normal conditions. In the worst-case soft-
error condition, it would take 1000 ms. In the additional 500 ms, we
travel 2.5 m too far. A bit of thought will show this to be the worst case,
and it depends on instantaneous acceleration of the whole vehicle; more
analysis is needed, but for now I’m confident that with inertia this
condition is safe.

If the motor has sequential errors, we treat this as a hard error and stop
the system; otherwise, we continue to use the last known velocity and report
a soft error. If we do have a valid count, we update our accumulated velocity
appropriately, using a circular window to keep accurate count.

Range analysis: Although the expected range of a signed count is small, any
int8_t is valid (except numeric_limits<int8_t>::min()). Since we accumulate
for 1000 ms, the max value we can see is 1000*numeric_limits<int8_t>::max(),
and the min value is the inverse of that. 18 bits are required for this value;
we will use an int32_t. Overflow of this value is possible only in the case
of a logic error, and is treated as a hard error.

For each 1 ms processor tick, we feed the updated velocity value through
the PID controller and thus run the inner loop at 1 ms intervals. As
mentioned, the units of velocity in the PID controller are "number of Hall sensor
transitions per second", stored signed in an int32_t. For now, a simple PID
controller with feedforward and symmetric gains is used. No filtering is needed
because the accumulation over multiple ticks already acts as a low-pass filter.

Range analysis: We assume that the setpoint for the PID controller fits in 12
bits, and the current value fits in 18 bits. The largest possible error is thus
18 bits. The integral is updated once per ms (units are "number of Hall effect
sensor transitions / 1000"), and saturates (both for correctness and to reduce
windup) at a value that is at least 2^18 less than numeric_limits<int32_t>::max();
we rely on this to implement saturation. The derivative is calculated once per
ms (units are "number of Hall effect sensor transitions * 1000 per second^2"),
and fits in 19 bits. Tighter bounds can be placed on all of these if needed; but
for now, we use excess precision to simplify things. With Kp, Ki, Kd, and Kf all
32 bits, Kp*error + Ki*integral + Kd*derivative + Kf*setpoint fits in 62 bits if
integral fits in 28 bits; we will use 64-bit arithmetic.

Tuning: At a throttle setting of 512, we see between 640 and 650 ticks per second
with no load. (This suggests that our estimate of 4000 above is wrong, either due
to number of ticks per revolution being wrong or max speed being wrong.) We thus set
Kf so that, when a speed of 645 is requested, a throttle setting of 512 results. We
then use Ziegler-Nichols around this setpoint. We find a critical Kp (with Ki and Kd
zero) of around 6 * 2^16, with a critical period of 2 seconds. We thus set Kp to
0.33 * 6 * 2^16, a Ki of Kp / 1 s, and a Kd of Kp * 0.67 s ("some overshoot"). This
seems to oscillate wildly at lower speeds, so we reduce the gains by another factor
of four to stabilize -- we should consider gain scheduling in the future. With these
values and no load, the maximum achievable velocity is around 700. Re-turned at 2.0
m/s using the same strategy; critical time was 1.6 s.

We use a slo-mo camera at 240 fps as a poor man's tachometer. At a setting of 250,
it takes us 59 frames complete a revolution. In that ~1/4 s, we see ~60 ticks. This
is likely an accurate number of ticks per revolution. The maximum velocity we can
stabilize our velocity at (still no-load) is 6.2 m/s, or 740, so we set this as the
maximum.

-Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-covered-switch-default -Wno-padded -Wno-global-constructors

TODO:
Figure out why observed throttle output never > 2.0V, and adjust max based on what we learn.
*/

#define TICKS_IN_WINDOW 500

class MotorVelocityState {
public:

	MotorVelocityState() = default;
	~MotorVelocityState() = default;
	MotorVelocityState(MotorVelocityState const &) = delete;
	MotorVelocityState &operator =(MotorVelocityState const &) = delete;

	enum HallSensorTransitionEvent {
		NO_TRANSITION,
		ERROR_OCCURRED,
		MOTOR_WENT_FORWARD,
		MOTOR_WENT_BACKWARD
	};

	enum MotorVelocityStateError {
		NO_ERROR,
		SOFT_ERROR,
		HARD_ERROR
	};

	struct UpdateResult {
		MotorVelocityStateError status;
		int32_t velocity;
	};

	void RecordHallSensorTransition(HallSensorTransitionEvent event);
	UpdateResult UpdateVelocityOnProcessorTick();

private:

	int8_t count = 0;
	int8_t window[TICKS_IN_WINDOW] = {0};
	size_t window_idx = 0;
	int32_t velocity = 0;
	MotorVelocityStateError error = NO_ERROR;

	constexpr static int8_t COUNT_ERROR = std::numeric_limits<int8_t>::min();
	constexpr static int32_t MAX_VELOCITY = TICKS_IN_WINDOW * std::numeric_limits<int8_t>::max();
};

class MotorPidController {
public:

	MotorPidController() = default;
	~MotorPidController() = default;
	MotorPidController(MotorPidController const &) = delete;
	MotorPidController &operator =(MotorPidController const &) = delete;

	void SetSetpoint(int32_t new_setpoint);
	int64_t ComputeNextOutput(int32_t current_value);

private:

	int32_t setpoint = 0;
	int32_t integral = 0;
	int32_t last_error = 0;

	// All tuning done at 2.0 m/s at 28.0 V
	constexpr static int32_t MAX_INTEGRAL = 1<<20;  // Set to make sure we can achieve full range output
	constexpr static int32_t Kp = (1 << 16) * 175 / 100 * 1 / 3;
	constexpr static int32_t Ki = Kp / 1600 * 2;
	constexpr static int32_t Kd = Kp * 1600 / 3 / 1000;
	constexpr static int32_t Kf = (1 << 16) * 30 / 100;
};

class Motor {
private:
	enum Mode {
		STOPPED,
		FORWARD,
		BACKWARD
	};

public:
	Motor(bool flip_halls,
			GPIO_TypeDef* pa, uint16_t a, GPIO_TypeDef* pb, uint16_t b, GPIO_TypeDef* pc, uint16_t c,
			GPIO_TypeDef* pd, uint16_t d, GPIO_TypeDef* pbrake, uint16_t brake, uint32_t tc)
		: gpio_port_a{pa}, gpio_pin_a{a},
		  gpio_port_b{flip_halls ? pc : pb}, gpio_pin_b{flip_halls ? c : b},
		  gpio_port_c{flip_halls ? pb : pc}, gpio_pin_c{flip_halls ? b : c},
		  gpio_port_dir{pd}, gpio_pin_dir{d}, gpio_port_brake(pbrake), gpio_pin_brake(brake), timer_channel{tc} {}
	~Motor() = default;
	Motor(Motor const &) = delete;
	Motor &operator =(Motor const &) = delete;

	void SetVelocity(int32_t velocity);
	void OnHallSensorTransitionInterrupt();
	void OnProcessorTick();

	constexpr static int32_t MAX_VELOCITY = std::lround(128 * HARD_SPEED_LIMIT);

private:

	GPIO_TypeDef* gpio_port_a;  // if null, motor disabled in software
	uint16_t gpio_pin_a;
	GPIO_TypeDef* gpio_port_b;
	uint16_t gpio_pin_b;
	GPIO_TypeDef* gpio_port_c;
	uint16_t gpio_pin_c;
	GPIO_TypeDef* gpio_port_dir;
	uint16_t gpio_pin_dir;
	GPIO_TypeDef* gpio_port_brake;
	uint16_t gpio_pin_brake;
	uint32_t timer_channel;

	bool initialized = false;
	int hall = 0;
	Mode mode = STOPPED;
	size_t soft_error_count = 0;

	/* The third-party motor controller we're using today has some strange behavior. The strangest is
	 * a 400 ms (experimentally measured) hold time on the DIRECTION signal, during which the THROTTLE
	 * must be 0 (1.0 V). Violation of this hold time will cause the motor to spin the wrong direction.
	 * To see this case, change the DIRECTION_COUNTDOWN_START to a value < 400 (0 works) and test the
	 * velocity sequence {+2, 0, -2, 0} -- when the -2 section starts, the hold time will be violated
	 * (because 0 holds the direction as forward) and the wheel will do bad things. */
	constexpr static uint32_t DIRECTION_COUNTDOWN_START = 400;
	bool current_direction = true;
	uint32_t direction_change_countdown = 0;

	MotorVelocityState velocity_state;
	MotorPidController pid_controller;

	void ReportSoftError();
	void AbortOnHardError() __attribute__((noreturn));
	std::tuple<int, int> NextHallStates();
	int ReadHallSensors();
	void SetBrake(bool brake_on);
	void SetDirection(bool go_forward);
	void SetThrottle(int32_t control);

	constexpr static int THROTTLE_SCALE_BITS = 16;
};

#endif /* MOTOR_H_ */
