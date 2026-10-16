#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "DifferentialDrive.h"

struct TargetPoint {
  float heading;
  float distance;
  bool exists;
  unsigned long time;
};

int left_motor_port = 4;
int right_motor_port = 3;
float wheel_radius = 0.065/2; // m
float wheel_base = 0.17; //m
float max_angular_velocity = 15.7; // from data sheet and estimation

Adafruit_MotorShield MotorShield = Adafruit_MotorShield(); //Initialize motor shield
Adafruit_DCMotor *left_motor = MotorShield.getMotor(left_motor_port);
Adafruit_DCMotor *right_motor = MotorShield.getMotor(right_motor_port);

DifferentialDrive drive = DifferentialDrive(left_motor, right_motor, wheel_base, wheel_radius, max_angular_velocity, 0); // Initialize helper class

TargetPoint target; //stores target info
unsigned long timeout = 500; //milis
unsigned long disconnect_timeout = 2000; // millis

// Start with just a P (for heading and distance) loop. will add I and D if needed.
float kP_heading = -0.15; // Values set through trial and error 
float kP_distance = 0.5 / 0.75;

float search_lin_vel = 0.2; // m/sec

bool enabled = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.setTimeout(500);
  if (!MotorShield.begin()) { // Initialize motor shield
    Serial.println("could not initialize motor shield :(");
    while (1);
  }
  Serial.println("Succesfully connected to motor shield");
}

void loop() {
  get_target(); // Read any sent serial data from Pi
  // Serial.println(target.exists);
  if (!enabled) {
    // If we are disabled stop to ensure no motion. Also servse as a guard clause
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
  angular_velocity = kP_heading*target.heading; //Calculate velocties to command with P loops
  linear_velocity = 0.5;
  Serial.print("cmd_vel_ang: "); Serial.println(angular_velocity);
  Serial.print("cmd_vel_lin: "); Serial.println(linear_velocity);
  drive.drive(linear_velocity, angular_velocity);
}


void search_behavior() {
  // Serial.println("Searching");
  // drive.drive(0, search_ang_vel);
  if (millis() % 2000 < 1000) {
    // alternate forward and reverse
    drive.drive(search_lin_vel, 0);
  } else {
    drive.drive(-search_lin_vel, 0);
  }
  return;
}


void get_target() {
  if (Serial.available() > 0) { //check if there is serial to be read.
    // Serial.println("Got message");
    String buffer = Serial.readStringUntil('\n'); //reads until we recive a new line. Times out after .5s
    // Serial.println(buffer);
    if (isDigit(buffer.charAt(0))) { //If we are getting a number this is a target point
      // Serial.print("number");
      enabled = true; //
      int comma_index = buffer.indexOf(',');
      String dist_str = buffer.substring(0, comma_index);
      String angle_str = buffer.substring(comma_index+1);
      target.heading = angle_str.toFloat();
      target.distance = dist_str.toFloat() / 1000.0; //dist provided in mm converted to m
      // Serial.print("heading: "); Serial.println(target.heading);
      // Serial.print("dist: "); Serial.println(target.distance);
      target.exists = true;
      target.time = millis();
    } else if ( buffer == "SHUTDOWN") { //Python program is stopping, stop arudino code
      drive.stop(); 
      enabled = false;
    } else if (buffer == "NONE") { //No target found
      // Serial.println("NONE - " + String(millis() - target.time) + "ms since last pos");
      if ((millis() - target.time) > timeout) { //if it has been too long we forget about the last point
        // Serial.print("Timeout");
        target.exists = false;
      }
      if (!target.exists) { //If the target has already timed out and we are ideling update timestamp so we dont trigger disconect
        target.time = millis();
      }
    } else { // Handels any malformed messages
      target.exists = false;
    }
    
  } else if ((millis() - target.time) > disconnect_timeout) { //If it has been too long since we got a message assume Pi is dead
    if (enabled == true) enabled = false;
  }
}

