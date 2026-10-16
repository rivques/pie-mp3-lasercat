#include <math.h>
#include "DifferentialDrive.h"

// This class represents a differential drive controller. It takes a refrence to a left and right motor, and several paramaters about the robot.
DifferentialDrive::DifferentialDrive(Adafruit_DCMotor *left_motor, Adafruit_DCMotor *right_motor, float wheel_base, float wheel_radius, float max_angular_vel, float angular_bias) {
  m_left_motor = left_motor;
  m_right_motor = right_motor;
  m_wheel_base = wheel_base;
  m_wheel_radius = wheel_radius;
  m_max_angular_vel = max_angular_vel;
  m_angular_bias = angular_bias;
}


void DifferentialDrive::run_motors(float left_speed, float right_speed) {
  // First convert speed values to values between 0 and 255
  long left_speed_val = floor(255 * left_speed/m_max_angular_vel);
  long right_speed_val = floor(255 * right_speed/m_max_angular_vel);
  // values can be passed in as negative or postive but we need them to be positive
  left_speed_val = abs(left_speed_val);
  right_speed_val = abs(right_speed_val);

  // The sign of the speed value for each motor sets the direction the motor should turn. + = forward; - = backwards; 0 = stop
  if (left_speed > 0) {
    m_left_motor->run(FORWARD);
  } else if (left_speed < 0) {
    m_left_motor->run(BACKWARD);
  } else {
    m_left_motor->run(RELEASE);
  }
  if (right_speed > 0) {
    m_right_motor->run(FORWARD);
  } else if (right_speed < 0) {
    m_right_motor->run(BACKWARD);
  } else {
    m_right_motor->run(RELEASE);
  }

  // makes sure the values are not out of bounds 
  if (left_speed_val > 255 || right_speed_val > 255) {
    Serial.print("rescaling: "); Serial.print(left_speed_val); Serial.print(", "); Serial.println(right_speed_val);
    long highest = max(left_speed_val, right_speed_val);
    left_speed_val = left_speed_val * 255 / highest;
    right_speed_val = right_speed_val * 255 / highest;
    Serial.print("to: "); Serial.print(left_speed_val); Serial.print(", "); Serial.println(right_speed_val);
  }

  // Set the motor speeds using the base motor commands
  // Angular bias is a magic number used to help account for one motor being more powerful thanthe other.
  // It is added in this stage so we can have the most direct control over how it affects motor speeds. 
  // Serial.print("left speed: "); Serial.println(left_speed_val);
  // Serial.print("right speed: "); Serial.println(right_speed_val);
  m_left_motor->setSpeed(left_speed_val+m_angular_bias/2);
  m_right_motor->setSpeed(right_speed_val-m_angular_bias/2);
}

// Function to stop the motors
void DifferentialDrive::stop() {
  run_motors(0, 0);
}


// Main public method. Takes in a linear and angular velocity and converts it to wheel angular velocities using the standard formula
void DifferentialDrive::drive(float forward_vel, float angular_vel) {
  float left_speed = 1/m_wheel_radius * (forward_vel - (angular_vel*m_wheel_base/2));
  float right_speed = 1/m_wheel_radius * (forward_vel + (angular_vel*m_wheel_base/2));
  // calls private internal method to convert angular velocity to PWM commands
  run_motors(left_speed, right_speed);
}

