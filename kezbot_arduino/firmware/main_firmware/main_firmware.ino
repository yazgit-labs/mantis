#define USE_USBCON
#include <ros.h>
#include <ros/time.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Pose2D.h>
#include <geometry_msgs/Pose.h>
#include <nav_msgs/Odometry.h>
#include <Wire.h>
#include <QuadratureEncoder.h>
#include <Odometer.h>
#include <math.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <Servo.h>

//servo pin setup
int fwdServoPin = 6;
int turnServoPin = 7;
int fwdThrust = 90;
int turnThrust = 90;
Servo fwdMotor;
Servo turnMotor;

//localizaton varialbles
float xVector = 0;
float yVector = 0;
float dWay = 0;
float ddWay = 0;
float hold = 0;

long hold_millis = 0;
long millis_passed = 0;
long before_millis = 0;

float current_speed;

//bno055 setup
Adafruit_BNO055 bno = Adafruit_BNO055(55);
imu::Vector<3> gyro;
int bnoRstPin = 26;
imu::Quaternion quat;


//encoderPinSetup
int encoderLA = 36;
int encoderLB = 38;
int encoderRA = 40;
int encoderRB = 42;

//object definitions
Encoder leftEncoder(LEFT);
Encoder rightEncoder(RIGHT);
Odometer pyhsical_odom;

//function definitions
void rightHandlerA();   //functions for encoder interrupt handlers
void rightHandlerB();
void leftHandlerA();
void leftHandlerB();

inline double degToRad(double deg);
float map_func(float value, float inMin, float inMax, float outMin, float outMax);
void reset_BNO055();


/*
    ------------- ROS ------------
*/
ros::NodeHandle  nh;
geometry_msgs::Pose2D pose2d;
geometry_msgs::Twist speed_msg;
geometry_msgs::Pose pose;
double x = 1.0;
double y = 0.0;
double theta = 1.57;
void cmd_vel_handle( const geometry_msgs::Twist& msg);
ros::Publisher pose2d_publisher("/pose2d", &pose2d);
ros::Publisher pose_publisher("/pose", &pose);
ros::Publisher speed_publisher("/speed", &speed_msg);
ros::Subscriber<geometry_msgs::Twist> cmd_vel_subscriber("/cmd_vel", &cmd_vel_handle );
/*
    ------------- ROS ------------
*/




void setup()
{

  digitalWrite(13, LOW);
  pinMode(bnoRstPin, OUTPUT);

  //rosnode initialize
  nh.initNode();
  nh.subscribe(cmd_vel_subscriber);
  nh.advertise(pose2d_publisher);
  nh.advertise(pose_publisher);
  nh.advertise(speed_publisher);

  //encoder setup for odometer
  rightEncoder.attach(encoderRA, encoderRB);
  leftEncoder.attach(encoderLA, encoderLB);
  leftEncoder.initialize();
  rightEncoder.initialize();
  attachInterrupt(digitalPinToInterrupt(leftEncoder.greenCablePin), leftHandlerA , CHANGE);
  attachInterrupt(digitalPinToInterrupt(leftEncoder.yellowCablePin), leftHandlerB , CHANGE);
  attachInterrupt(digitalPinToInterrupt(rightEncoder.greenCablePin), rightHandlerA , CHANGE);
  attachInterrupt(digitalPinToInterrupt(rightEncoder.yellowCablePin), rightHandlerB , CHANGE);

  reset_BNO055();

  //initialize bno055 absolute orientation sensor
  while (!bno.begin()) {
    nh.logerror("NO BNO055 detected. Check your wiring.");
    reset_BNO055();
  }

  bno.setExtCrystalUse(true);
  delay(100);

  //PWM motor controller setup
  fwdMotor.attach(fwdServoPin);
  turnMotor.attach(turnServoPin);

  turnMotor.write(90);
  fwdMotor.write(90);

}

void loop()
{

  //get values from encoders for odometer
  hold = pyhsical_odom.getWay() / float(100);
  dWay = hold - ddWay;
  ddWay = hold;

  hold_millis = millis();
  millis_passed = hold_millis - before_millis ;
  before_millis = hold_millis;

  sensors_event_t event;
  bno.getEvent(&event);
  quat = bno.getQuat();
  quat.normalize();
//  imu::Vector<3> vec(0.0, 0.0, 1.0);
//  quat.rotateVector(vec);
  yVector = yVector + dWay * sin(degToRad(map_func(event.orientation.x,0,360,360,0)));
  xVector = xVector + dWay * cos(degToRad(map_func(event.orientation.x,0,360,360,0)));

  //correct the orientation
  pose2d.x = xVector;
  pose2d.y = yVector;
  pose2d.theta = degToRad(map_func(event.orientation.x,0,360,360,0));

  current_speed = dWay / millis_passed;

  // Pose2D publisher
  pose2d_publisher.publish( &pose2d );

  // Pose -> Position
  pose.position.x = pose2d.x;
  pose.position.y = pose2d.y;
  pose.position.z = 0.0;
  pose.orientation.x = quat.x();
  pose.orientation.y = quat.y();
  pose.orientation.z = quat.z();
  pose.orientation.w = quat.w();

  // Twist -> Linear
  speed_msg.linear.x = current_speed;// speed that is currently going
  speed_msg.linear.y = 0;
  speed_msg.linear.z = 0;
  // Twist -> Angular
  speed_msg.angular.x = 0;
  speed_msg.angular.y = 0;
  speed_msg.angular.z = degToRad(event.gyro.x);

  pose_publisher.publish(&pose);
  speed_publisher.publish(&speed_msg);

  nh.spinOnce();

  delay(1);
}


void rightHandlerA() {
  rightEncoder.handleInterruptGreen();
  pyhsical_odom.rightEncoderTick = rightEncoder.encoderTicks;
}
void rightHandlerB() {
  rightEncoder.handleInterruptYellow();
  pyhsical_odom.rightEncoderTick = rightEncoder.encoderTicks;
}

void leftHandlerA() {
  leftEncoder.handleInterruptGreen();
  pyhsical_odom.leftEncoderTick = leftEncoder.encoderTicks;
}
void leftHandlerB() {
  leftEncoder.handleInterruptYellow();
  pyhsical_odom.leftEncoderTick = leftEncoder.encoderTicks;
}

inline double degToRad(double deg) {
  return deg * M_PI / 180.0;
}

float map_func(float value, float inMin, float inMax, float outMin, float outMax) {
  return (value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}

void cmd_vel_handle( const geometry_msgs::Twist& msg) {
  float fwd_message = msg.linear.x;
  float turn_message = msg.angular.z;

  fwdMotor.write(
    int(map_func(fwd_message, -1.3, 1.3 , 160 , 20))
  );
  turnMotor.write(
    int(map_func(turn_message, -2.5, 2.5 , 20, 160))
  );
}

void reset_BNO055(){
    digitalWrite(bnoRstPin, LOW);
    delay(100);
    digitalWrite(bnoRstPin, HIGH);
    delay(100);
}
