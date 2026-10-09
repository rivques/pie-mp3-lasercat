#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "DifferentialDrive.h"

struct TargetPoint {
  float heading;
  float distance;
  bool exists;
  unsigned long time;
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

TargetPoint target;
unsigned long timeout = 250; //milis
unsigned long disconnect_timeout = 2000; // millis

// Start with just a P (for heading and distance) loop. will add I and D if needed.
float kP_heading = 1; //TODO Set values
float kP_distance = 1;

float search_ang_vel = 0.5; // rad/sec

bool enabled = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.setTimeout(500);
  if (!MotorShield.begin()) {
    Serial.println("could not initialize motor shield :(");
    while (1);
  }
  Serial.println("Succesfully connected to motor shield");
}

void loop() {
  get_target();
  if (!enabled) {
    drive.stop();
    return;
  }
  if (target.exists) {
    chase_behavior(target);
  } else {
    search_behavior();
  }
  delay(5);
}


void chase_behavior(TargetPoint target) {
  //Behavior to run when there is a laser in sight
  float linear_velocity;
  float angular_velocity;
  angular_velocity = kP_heading*target.heading;
  linear_velocity = kP_distance*target.distance;
  drive.drive(linear_velocity, angular_velocity);
}


void search_behavior() {
  drive.drive(0, search_ang_vel);
  return;
}


void get_target() {
  if (Serial.available() > 0) { //check if there is serial to be read.
    String buffer = Serial.readStringUntil('\n'); //times out after .5s
    if (isDigit(buffer.charAt(0))) { //If we are getting a number this is a target point
      enabled = true; //
      int comma_index = buffer.indexOf(',');
      String angle_str = buffer.substring(0, comma_index);
      String dist_str = buffer.substring(comma_index+1);
      target.heading = angle_str.toFloat();
      target.distance = dist_str.toFloat() / 1000.0; //dist provided in mm converted to m
      target.exists = true;
      target.time = millis();
    } else if ( buffer == "SHUTDOWN") { //Python program is stopping, stop arudino code
      drive.stop(); 
      enabled = false;
    } else if (buffer == "NONE") { //No target found
      if (millis() - target.time > timeout) { //if it has been too long we forget about the last point
        target.exists = false;
      }
    } else {
      target.exists = false;
    }
    
  } else if (millis() - target.time > disconnect_timeout) { //If it has been too long since we got a message assume Pi is dead
    if (enabled == true) enabled = false;
  }
}

