/* Firmware for Elsa
   Test version
   Author: Vincent Yuan, Modified: Nick Wu
     Modified: James Asefa
   Date: Oct 30, 2016
*/

#include <SoftwareSerial.h>
#include <stdlib.h>
#include <Servo.h>


#define TRIM 8
#define BAUD_RATE 115200
#define BUFFER_SIZE 6
// assign 128 to be not moving
// will be used to hold robot in place while
// reading from the buffer. Robot will be stopped initially.
#define UNMAPPED_STOP_SPEED 128

// DRIVING MODIFICATIONS
// modify FR, FL, BR, BL to change speeds: (+) correpsonds to faster speed, (-) for going reverse
// F:FRONT, B:BACK, R:RIGHT, L:LEFT e.g. FR = FRONT RIGHT WHEEL.
const int OFFSET = 1;
const int joystick_margin = 150;

// buffer inputs
int linear_x = 0;
int linear_y = 0;
int linear_z = 0;
int angular_x = 0;
int angular_y = 0;
int angular_z = 0;

// motor pins
const int left_motor_pin = 9;
const int right_motor_pin = 10;
// defines start of buffer
const char buffer_head = 'B';


// max and min linear speeds and stopping condition
const int linear_max = 100;
const int linear_min = 80;
const int linear_stop = 90;

// max and min angular speeds and stopping condition
const int angular_max = 100;
const int angular_min = 80;
const int angular_stop = 90;

Servo LeftM;
Servo RightM;


// ??
int R1 = 0, R2 = 0, R3 = 0, R4 = 0, B2 = 0, B4 = 0, Mode = 0;

int linear_x_h = 0, linear_x_l = 0, angular_z_h = 0, angular_z_l = 0;

void setup() {
  Serial.begin(BAUD_RATE);
  
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  
  LeftM.attach(left_motor_pin);
  RightM.attach(right_motor_pin);
  
  set_off();
}

void loop() {

  //if (Serial.read() == 'I') {Serial.print("DRIVE");}
  //else Serial.flushRX();
  //responding to queries from usb
  sig_read();
  
  /*Serial.println("Mode: ");Serial.println(Mode);Serial.println("R1: ");Serial.println(R1);
  Serial.println("R2: ");
  Serial.println(R2);
  Serial.println("R3: ");
  Serial.println(R3);*/
  //Serial.println("linear_x_h: ");Serial.println(linear_x_h);Serial.println("linear_x_l: ");Serial.println(linear_x_l);Serial.println("angular_z_h: ");Serial.println(angular_z_h);
  
  
  //serial_read();
  if (Mode == -1) { //Auto Mode
    serial_read(); 
    
    convert();
    drive(linear_x, angular_z);
    //Serial.print("angular_z_l: ");Serial.print(angular_z_l);Serial.print("linear_x: ");Serial.print(linear_x);Serial.print("angular_z: ");Serial.println(angular_z);
  }
  else if (Mode == 0) { //RC Mode
    //Serial.println(angular_z);
    linear_x = R1; 
    angular_z = R2;
    
    convert();
    drive(linear_x, angular_z);
    //Serial.print("angular_z_l: ");Serial.print(angular_z_l);Serial.print("linear_x: ");Serial.print(linear_x);Serial.print("angular_z: ");Serial.println(angular_z);
    Serial.flushRX();
  }
  else { //STOP MODE
    linear_x = linear_stop; 
    angular_z = angular_stop;
    
    convert();
    drive(linear_x, angular_z);
   // Serial.print("angular_z_l: ");Serial.print(angular_z_l);Serial.print("linear_x: ");Serial.print(linear_x);Serial.print("angular_z: ");Serial.println(angular_z);
    Serial.flushRX();
  }
  //Serial.print("linear_x: ");Serial.print(linear_x);Serial.print(" linear_x: ");Serial.print(linear_x);Serial.print(" angular_z: ");Serial.println(angular_z);

}

void set_off() {
  int linear_x_mid = 1550; //RX standard - radio signal midpoint
  int angular_z_mid = 1550; //RY standard - radio signal midpoint
  
  // joystick_margin is an error margin for joystick control
  // e.g. if the joystick is moved just a little bit, it is assumed that no movement 
  // is desired.
  linear_x_h = linear_x_mid + joystick_margin; 
  linear_x_l = linear_x_mid - joystick_margin;
  
  angular_z_h = angular_z_mid + joystick_margin; 
  angular_z_l = angular_z_mid - joystick_margin;
}


void sig_read() {
  R1 = pulseIn(2, HIGH); // 1140 - 1965 RX LEFT-RIGHT -> turn on spot
  R2 = pulseIn(3, HIGH); // 1965 - 1140 RY UP-DOWN -> Steer left/right y axis (turn while moving)
  R3 = pulseIn(4, HIGH); // 1970 - 1115 linear_x UP-DOWN -> forward/backward
  R4 = pulseIn(5, HIGH); //1970 - 1115 mode
  
  //Serial.print("R1: "); Serial.print(R1);Serial.print(" R2: "); Serial.print(R2);Serial.print(" R3: "); Serial.println(R3);Serial.println("Mode: ");Serial.println(Mode);
  if (R1 < linear_x_h && R1 > linear_x_l) 
    R1 = linear_stop;
  else 
    R1 = map (R1, 1140, 1965, 0, 255);

  if (R2 < angular_z_h && R2 > angular_z_l) 
    R2 = angular_stop;
  else 
    R2 = map (R2, 1140, 1965, 0, 255);
  
  //Serial.print("R3: ");Serial.println(R3);
  if (1100 < R3 && R3 < 1400) 
    Mode = 1; // STOP mode?
  else if (1400 < R3 && R3 < 1700) 
    Mode = 0; // auto mode
  else if (1700 < R3 && R3 < 2000) 
    Mode = -1; // RC mode
  else 
    Mode = -2; // STOP mode
  if (abs(R1 - 90) < TRIM) // error control
    R1 = linear_stop;
  if (abs(R2 - 90) < TRIM) 
    R2 = angular_stop;
}

void serial_read(){
    //reading in 6 chars from Serial
  if (Serial.available() > BUFFER_SIZE) {

      // buffer_head identifies the start of the buffer
      if (Serial.read() == buffer_head) {
          linear_x = Serial.read();
          linear_y = Serial.read();
          linear_z = Serial.read();
          angular_x = Serial.read();
          angular_y = Serial.read();
          angular_z = Serial.read();
      } else {
          linear_x = angular_z = UNMAPPED_STOP_SPEED;
      }

  } else {
      linear_x = angular_z = UNMAPPED_STOP_SPEED;
    }
    
  //Serial.end();
  //Serial.begin(9600);
  
  //flushRX defined here: https://forum.sparkfun.com/viewtopic.php?f=32&t=32715 
  Serial.flushRX();
}

void convert() {
  if (linear_x > 255)
    linear_x = 255; 
  else if (linear_x < 0)
    linear_x = 0;
  
  if (angular_z > 255)
    angular_z = 255; 
  else if (angular_z < 0)
    angular_z = 0;
  
  // mapping should map to a percentage of max and min duty cycle
  // for our motors
  linear_x = map(linear_x, 0, 255, linear_min, linear_max);
  angular_z = map(angular_z, 0, 255, angular_min, angular_max);
}


/*
 * moves the robot - turning is taken into account.
 */
void drive(int angular_speed, int linear_speed){
  
  // mapping should map to a percentage of max and min duty cycle
  // for our motors
  // we are using the Talon SRX - looks like duty cycle is between 2.9 - 100ms
  // seen here https://www.ctr-electronics.com/Talon%20SRX%20User's%20Guide.pdf
  int left_throttle = linear_speed + (angular_speed - angular_stop);
  int right_throttle = linear_speed - (angular_speed - angular_stop);
  
  servo_write(LeftM, left_throttle);
  servo_write(RightM, right_throttle);
}

// this writes to motor using duty cycle rather than servo arm angle
void servo_write(Servo motor, int throttle) {
  // throttle can be as high as 110 and as low as 70 after calculation
  // PWM input pulse high time can be between 1 and 2 ms. So 1000-2000 microseconds
  throttle = map(throttle, 70, 110, 1000, 2000); 
  motor.writeMicroseconds(throttle);
}
