#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "DifferentialDrive.h"

struct TargetPoint {
  float heading;
  float distance;
};

int left_motor_port = 3; //TODO set these
int right_motor_port = 4;
float wheel_radius = 0.065; // m
float wheel_base = 0.17; //m
float max_angular_velocity = 6; //TODO set this

Adafruit_MotorShield MotorShield = Adafruit_MotorShield();
Adafruit_DCMotor *left_motor = MotorShield.getMotor(left_motor_port);
Adafruit_DCMotor *right_motor = MotorShield.getMotor(right_motor_port);

DifferentialDrive drive = DifferentialDrive(left_motor, right_motor, wheel_base, wheel_radius, max_angular_velocity);


// Start with just a P (for heading and distance) loop. will add I and D if needed.
float kP_heading = 1; //TODO Set values
float kP_distance = 1;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  if (!MotorShield.begin()) {
    Serial.println("could not initialize motor shield :(");
    while (1);
  }
  Serial.println("Succesfully connected to motor shield");
}

void loop() {
  if (has_target()) {
    chase_behavior();
  } else {
    search_behavior();
  }
  delay(5);
}


void chase_behavior() {
  //Behavior to run when there is a laser in sight
  TargetPoint target = get_target();
  float linear_velocity;
  float angular_velocity;
  angular_velocity = kP_heading*target.heading;
  linear_velocity = kP_distance*target.distance;
}


void search_behavior() {
  //Impliment
  return;
}


TargetPoint get_target() {
  TargetPoint target;
  // Set values
  target.heading = 0;
  target.distance = 1;
  return target;
}

bool has_target() {
  //TODO
  return true;
}

