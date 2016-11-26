#include <SoftwareSerial.h>
#include <stdlib.h>
#include <math.h>
#include <Servo.h>


#define TRIM 8
#define BAUD_RATE 9600
#define WIDTH 10
#define BUFFER_SIZE 6
// assign 128 to be not moving
// will be used to hold robot in place while
// reading from the buffer. Robot will be stopped initially.
#define UNMAPPED_STOP_SPEED 128

// DRIVING MODIFICATIONS
// modify FR, FL, BR, BL to change speeds: (+) correpsonds to faster speed, (-) for going reverse
// F:FRONT, B:BACK, R:RIGHT, L:LEFT e.g. FR = FRONT RIGHT WHEEL.
const int OFFSET = 1;

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


void setup(){
  Serial.begin(9600);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);

  // if analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  
  LeftM.attach(left_motor_pin);
  RightM.attach(right_motor_pin);
  
  set_off();
}


void loop() {
  
   R1 = pulseIn(2, HIGH);
   R2 = pulseIn(3, HIGH);
   R3 = pulseIn(4, HIGH);
   R4 = pulseIn(5, HIGH); 
   Serial.print("R1: "); Serial.print(R1);Serial.print(" R2: "); Serial.print(R2);Serial.print(" R3: "); Serial.println(R3);Serial.println("Mode: ");Serial.println(Mode);
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
  
  Serial.println("linear_speed = ");Serial.println(linear_x);
  Serial.println("angular speed = ");Serial.println(angular_z);
  
  int left_speed = LeftM.read();
  int right_speed = RightM.read();
  
  Serial.println("left speed = ");Serial.println(left_speed);
  Serial.println("right speed = ");Serial.println(right_speed);
  
  delay(50);
}

void set_off() {
  int linear_x_mid = 1550; //RX standard
  int angular_z_mid = 1550; //RY standard
  
  linear_x_h = linear_x_mid + 150; 
  linear_x_l = linear_x_mid - 150;
  
  angular_z_h = angular_z_mid + 150; 
  angular_z_l = angular_z_mid - 150;
}

void sig_read() {
  R1 = pulseIn(2, HIGH); // 1140 - 1965 RX LEFT-RIGHT -> turn on spot
  R2 = pulseIn(3, HIGH); // 1965 - 1140 RY UP-DOWN -> Steer left/right y axis (turn while moving)
  R3 = pulseIn(4, HIGH); // 1970 - 1115 linear_x UP-DOWN -> forward/backward
  R4 = pulseIn(5,HIGH); //1970 - 1115 mode
  
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
    Mode = 1;
  else if (1400 < R3 && R3 < 1700) 
    Mode = 0;
  else if (1700 < R3 && R3 < 2000) 
    Mode = -1;
  else 
    Mode = -2;
  //Serial.print("R1: "); Serial.println(R1);
  //Serial.print("R2: "); Serial.println(R2);
  if (abs(R1 - 90) < TRIM) 
    R1 = linear_stop;
  if (abs(R2 - 90) < TRIM) 
    R2 = angular_stop;
}

void serial_read(){
    //reading in 6 chars from Serial
  if (Serial.available() > BUFFER_SIZE){

      /* if (Serial.read() == 'B'){
      linear_x = (Serial.read()-'0')*100 + (Serial.read()-'0')*10 + (Serial.read()-'0');
      ly =(Serial.read()-'0')*100 + (Serial.read()-'0')*10 + (Serial.read()-'0');
      angular_z = (Serial.read()-'0')*100 + (Serial.read()-'0')*10 + (Serial.read()-'0');*/

      // buffer_head identifies the start of the buffer
      if (Serial.read() == buffer_head) {
          linear_x = Serial.read();
          linear_y = Serial.read();
          linear_z = Serial.read();
          angular_x = Serial.read();
          angular_y = Serial.read();
          angular_z = (double) Serial.read();
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

void drive(int angular_speed, int linear_speed){

  LeftM.write(linear_speed + (angular_speed - angular_stop));
  RightM.write(linear_speed - (angular_speed - angular_stop));
   /* if (angular_speed < angular_stop) {
      
    } else {
      LeftM.write(linear_speed - (angular_speed - angular_stop));
      RightM.write(linear_speed + (angular_speed - angular_stop));
    } */
}

void convert(){
  /* Serial.read() reads values in [0,255]
   * we map them to lower and upper speeds defined above
   * for both linear and angular velocity
  */
  linear_x = map (linear_x, 0, 255, linear_min, linear_max);
  angular_z = map (angular_z, 0, 255, angular_min, angular_max);
}


