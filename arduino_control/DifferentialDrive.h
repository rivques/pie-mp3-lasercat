#ifndef DIFFDRIVE_H
#define DIFFDRIVE_H

#include <Adafruit_MotorShield.h>

class DifferentialDrive {
  private:
    Adafruit_DCMotor *m_left_motor;
    Adafruit_DCMotor *m_right_motor;

    void run_motors(float left_speed, float right_speed);

  public:
    float m_wheel_base;
    float m_wheel_radius;
    float m_max_angular_vel;
    float m_angular_bias;
    DifferentialDrive(Adafruit_DCMotor *left_motor, Adafruit_DCMotor *right_motor, float wheel_base, float wheel_radius, float max_angular_vel, float angular_bias);
    void stop();
    void drive(float forward_vel, float angular_vel);
};

#endif