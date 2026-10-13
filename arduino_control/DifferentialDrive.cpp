#include <math.h>
#include "DifferentialDrive.h"

DifferentialDrive::DifferentialDrive(Adafruit_DCMotor *left_motor, Adafruit_DCMotor *right_motor, float wheel_base, float wheel_radius, float max_angular_vel, float angular_bias) {
  m_left_motor = left_motor;
  m_right_motor = right_motor;
  m_wheel_base = wheel_base;
  m_wheel_radius = wheel_radius;
  m_max_angular_vel = max_angular_vel;
  m_angular_bias;
}


void DifferentialDrive::run_motors(float left_speed, float right_speed) {
  int left_speed_val = floor(255 * left_speed/m_max_angular_vel);
  int right_speed_val = floor(255 * right_speed/m_max_angular_vel);
  left_speed_val = abs(left_speed_val);
  right_speed_val = abs(right_speed_val);

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

  if (left_speed_val > 255) left_speed_val = 255;
  if (right_speed_val > 255) right_speed_val = 255; 

  m_left_motor->setSpeed(left_speed_val+m_angular_bias/2);
  m_right_motor->setSpeed(right_speed_val-m_angular_bias/2);
}

void DifferentialDrive::stop() {
  run_motors(0, 0);
}


void DifferentialDrive::drive(float forward_vel, float angular_vel) {
  float left_speed = 1/m_wheel_radius * (forward_vel - (angular_vel*m_wheel_base/2));
  float right_speed = 1/m_wheel_radius * (forward_vel + (angular_vel*m_wheel_base/2));
  run_motors(left_speed, right_speed);
}

