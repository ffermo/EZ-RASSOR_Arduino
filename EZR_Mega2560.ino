/*
Arduino code to work with EZ-RASSOR software package.
Written by Francis Fermo
*/

#include <Wire.h>
#include <ros.h>
#include <std_msgs/Empty.h>
#include <geometry_msgs/Twist.h>
#include <Adafruit_MotorShield.h>
#include <AccelStepper.h>

// Constants for EZ-RASSOR wheel velocity.
#define MAX_LINEAR_SPS 66.50 // Steps per second to achieve one meter/second
#define MAX_ANGULAR_SPS 33.25 // Angular steps per second to achieve one radian/second (?)
#define WHEEL_RAD .120 // Radius of wheel in meters.
#define WHEEL_DIST .20955 // Distance between left and right wheels in meters.

// Drop of solder on motorshield determines the address inside Adafruit_Motorshield(). Empty parameter in Adafruit_MotorShield() means default address (0x60). Google "Stacking Motorshields" to learn more about the soldering process.
Adafruit_MotorShield FrontWheelMS = Adafruit_MotorShield(); // Address 0x60 will be used for front wheels.
Adafruit_MotorShield BackWheelMS = Adafruit_MotorShield(0x61); // Address 0x61 will be used for back wheels.

// Once motorshields have been assigned to an object, we can now put each motor from each shield into it's own object.
Adafruit_StepperMotor *fl_wheel = FrontWheelMS.getStepper(200, 1); // Front left wheel | M1-M2
Adafruit_StepperMotor *fr_wheel = FrontWheelMS.getStepper(200, 2); // Front right wheel | M3-M4
Adafruit_StepperMotor *bl_wheel = BackWheelMS.getStepper(200, 1); // Back left wheel | M1-M2
Adafruit_StepperMotor *br_wheel = BackWheelMS.getStepper(200, 2); // Back right wheel | M3-M4

// Node handler for ROS.
ros::NodeHandle nh;

void moveForward() {
 	fl_wheel->onestep(BACKWARD, DOUBLE);
 	bl_wheel->onestep(BACKWARD, DOUBLE);
 	fr_wheel->onestep(FORWARD, DOUBLE);
 	br_wheel->onestep(FORWARD, DOUBLE);
}

void moveBackward()
{
 	fl_wheel->onestep(FORWARD, DOUBLE);
 	bl_wheel->onestep(FORWARD, DOUBLE);
 	fr_wheel->onestep(BACKWARD, DOUBLE);
 	br_wheel->onestep(BACKWARD, DOUBLE);
}
 
void turnLeft()
{
 	fl_wheel->onestep(FORWARD, DOUBLE);
 	bl_wheel->onestep(FORWARD, DOUBLE);
 	fr_wheel->onestep(FORWARD, DOUBLE);
 	br_wheel->onestep(FORWARD, DOUBLE);
}
 
void turnRight()
{
 	fl_wheel->onestep(BACKWARD, DOUBLE);
 	bl_wheel->onestep(BACKWARD, DOUBLE);
 	fr_wheel->onestep(BACKWARD, DOUBLE);
 	br_wheel->onestep(BACKWARD, DOUBLE);
}

// Objects to handle angular and linear travel.
AccelStepper linearX(moveForward, moveBackward);
AccelStepper angularZ(turnLeft, turnRight);

// /wheel_instructions topic handler.
void twistCb(const geometry_msgs::Twist& msg)
{
	// Take twist messages and multiply by max speed to calculate current speed.
	linearX.setSpeed(msg.linear.x * MAX_LINEAR_SPS);
	angularZ.setSpeed(msg.angular.z * MAX_ANGULAR_SPS);
}

// Subscriber for topics and initializes callbacks (second parameter) for functions.
ros::Subscriber<geometry_msgs::Twist> subTwist("/wheel_instructions", &twistCb );

void setup()
{ 
	// Initializes serial connection to Mega. Baud rate used for ROS message traffic.
	nh.getHardware()->setBaud(115200);

	// Initializes node handler and subscribers.
	nh.initNode();
	nh.subscribe(subTwist);

	// Initializes motor shields.
	FrontWheelMS.begin();
	BackWheelMS.begin();

	// Sets clock frequency of I2C. Assists with limitations on motor speed.
	Wire.setClock(800000);

	// Sets speed for each motor in terms of steps/second.
    linearX.setMaxSpeed(MAX_LINEAR_SPS);
	linearX.setAcceleration(MAX_LINEAR_SPS);
    angularZ.setMaxSpeed(MAX_ANGULAR_SPS);
	angularZ.setAcceleration(MAX_ANGULAR_SPS);

    linearX.setSpeed(0.0);
    angularZ.setSpeed(0.0);
}

void loop()
{ 
    // Gets messages from ROS topics and calls twistCB.
	nh.spinOnce();

	// Runs linear or angular travel at calculated speeds.
	linearX.runSpeed();
	angularZ.runSpeed();
}
