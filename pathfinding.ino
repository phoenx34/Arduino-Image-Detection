
//#define DEBUG
#define POWER

#define NUM_OF_SENSORS 8
#define MIDDLE_POS (((NUM_OF_SENSORS*1000) - 1000) / 2)

//#define EDGED_TRACK

#define WHITE_TRASHOLD 300
#define BLACK_TRASHOLD 100

#define FORWARD_MAX 255
#define REVERSE_MAX 20
#define CALIBRATION_SPEED 150

#ifndef engine_h
#define engine_h

#include "config.h"
#include "WProgram.h"

#define PwmPinMotorA 10
#define PwmPinMotorB 11
#define DirectionPinMotorA 12
#define DirectionPinMotorB 13

#define MotorA 0
#define MotorB 1

#define FORWARD 0
#define REVERSE 1

#define REVOLVE_LEFT 0
#define REVOLVE_RIGHT 1

Button button = Button(14,PULLDOWN);
const int ledPin = 15;
PololuQTRSensorsRC qtr((unsigned char[]) {2,3,4,5,6,7,8,9}, NUM_OF_SENSORS);

void setup(){
#ifdef DEBUG
  Serial.begin(9600);
#endif

  pinMode(ledPin, OUTPUT);
  engineSetup();

  while(button.isPressed() == false);
  calibrate(&qtr);
  while(button.isPressed() == false);

  delay(2000);

  motor(MotorA,FORWARD_MAX);
  motor(MotorB,FORWARD_MAX);
}
void loop(){
  int pos = readLine(&qtr);

  if (pos != LINE_LOST){

    if (pos == EDGE_RIGHT){
      motor(MotorA,REVERSE_MAX);
      motor(MotorB,FORWARD_MAX);
    }else if (pos == EDGE_LEFT){
      motor(MotorA,FORWARD_MAX);
      motor(MotorB,REVERSE_MAX);
    }else{

      int error = pos - MIDDLE_POS;
      int m1Speed = FORWARD_MAX;
      int m2Speed = FORWARD_MAX;

      if (error < 0)
        m1Speed = map(error,-MIDDLE_POS,0,REVERSE_MAX,FORWARD_MAX);
      else
        m2Speed = map(error,0,MIDDLE_POS,FORWARD_MAX,REVERSE_MAX);

#ifdef DEBUG
      Serial.print(error);   Serial.print(" ");
      Serial.print(m1Speed); Serial.print(" "); Serial.println(m2Speed);
#endif

      motor(MotorA,m1Speed);
      motor(MotorB,m2Speed);
    }
  }
}

void engineSetup(){
  // motor pins must be outputs
  pinMode(PwmPinMotorA, OUTPUT);
  pinMode(PwmPinMotorB, OUTPUT);
  pinMode(DirectionPinMotorA, OUTPUT);
  pinMode(DirectionPinMotorB, OUTPUT);
}

void motor(int motor,int speed){
#ifdef POWER
  int pwmPin = 10;
  int dirPin = 12;
  int dir = HIGH;

  if(motor == MotorB){
    pwmPin = 11;
    dirPin = 13;
  }

  if(speed < 0){
    dir = LOW;
    speed = -1 * speed;
  }

  analogWrite(pwmPin, speed);
  digitalWrite(dirPin, dir);
#endif
}

void revolve(int speed , int direction){
  if (direction == REVOLVE_LEFT ){
    motor(MotorA,speed);
    motor(MotorB,-speed);
  }else{
    motor(MotorA,-speed);
    motor(MotorB,speed);
  } 
}

void calibrate(PololuQTRSensorsRC* qtr){
  revolve(CALIBRATION_SPEED,REVOLVE_LEFT);

  int i;
  for (i = 0; i < 125; i++){
    qtr->calibrate(QTR_EMITTERS_ON);
    delay(20);
  }

  revolve(0,REVOLVE_LEFT);
}

bool lineLost(unsigned int* vals){
  bool lost = true;

  if (TRACKING_WHITE == 0){
    for(int i=0;i<NUM_OF_SENSORS;i++){
      if (vals[i] > WHITE_TRASHOLD){
        lost =false;
      }
    }
  }else{
    for(int i=0;i<NUM_OF_SENSORS;i++){
      if (vals[i] < BLACK_TRASHOLD){
        lost =false;
      }
    }
  }
  return lost;
}

void detectTrackColor(unsigned int* vals){
  if ((vals[0] < WHITE_TRASHOLD) &&
      (vals[NUM_OF_SENSORS - 1] < WHITE_TRASHOLD))
    TRACKING_WHITE = 0;

  if ((vals[0] > BLACK_TRASHOLD) &&
      (vals[NUM_OF_SENSORS - 1]> BLACK_TRASHOLD))
    TRACKING_WHITE = 1;
}

int checkEdge(unsigned int* vals){ 
  if (TRACKING_WHITE == 0){
    if ((vals[NUM_OF_SENSORS - 1] > WHITE_TRASHOLD) &&
        (vals[NUM_OF_SENSORS - 2] > WHITE_TRASHOLD) &&
        (vals[NUM_OF_SENSORS - 3] > WHITE_TRASHOLD) &&
        (vals[0] < WHITE_TRASHOLD) &&
        (vals[1] < WHITE_TRASHOLD) &&
        (vals[2] < WHITE_TRASHOLD)){
      return EDGE_LEFT;
    }
    if ((vals[0] > WHITE_TRASHOLD) &&
        (vals[1] > WHITE_TRASHOLD) &&
        (vals[2] > WHITE_TRASHOLD) &&
        (vals[NUM_OF_SENSORS - 1] < WHITE_TRASHOLD) &&
        (vals[NUM_OF_SENSORS - 2] < WHITE_TRASHOLD) &&
        (vals[NUM_OF_SENSORS - 3] < WHITE_TRASHOLD)){
      return EDGE_RIGHT;
    }
  }else{
    if ((vals[NUM_OF_SENSORS - 1] < BLACK_TRASHOLD) &&
        (vals[NUM_OF_SENSORS - 2] < BLACK_TRASHOLD) &&
        (vals[NUM_OF_SENSORS - 3] < BLACK_TRASHOLD) &&
        (vals[0] > BLACK_TRASHOLD) &&
        (vals[1] > BLACK_TRASHOLD) &&
        (vals[2] > BLACK_TRASHOLD)){
      return EDGE_LEFT;
    }
    if ((vals[0] < BLACK_TRASHOLD) &&
        (vals[1] < BLACK_TRASHOLD) &&
        (vals[2] < BLACK_TRASHOLD) &&
        (vals[NUM_OF_SENSORS - 1] > BLACK_TRASHOLD) &&
        (vals[NUM_OF_SENSORS - 2] > BLACK_TRASHOLD) &&
        (vals[NUM_OF_SENSORS - 3] > BLACK_TRASHOLD)){
      return EDGE_RIGHT;
    }
  }
  return 0;
}

int readLine(PololuQTRSensorsRC* qtr){
  unsigned int val[NUM_OF_SENSORS];
  qtr->readCalibrated(val);

  detectTrackColor(val);

  int line = qtr->readLine(val,QTR_EMITTERS_ON,TRACKING_WHITE);

#ifdef DEBUG
  Serial.print(TRACKING_WHITE);   Serial.print(" ");
  Serial.print(line);   Serial.print(" R< ");
  Serial.print(val[0]); Serial.print(" ");
  Serial.print(val[1]); Serial.print(" ");
  Serial.print(val[2]); Serial.print(" ");
  Serial.print(val[3]); Serial.print(" ");
  Serial.print(val[4]); Serial.print(" ");
  Serial.print(val[5]); Serial.print(" ");
  Serial.print(val[6]); Serial.print(" ");
  Serial.print(val[7]); Serial.print(" >L ");

  if (lineLost(val) == true)
    Serial.print("Line Lost \n");
  if (checkEdge(val) != 0)
    Serial.print("Edge \n");
#endif

  if (lineLost(val) == true)
    return LINE_LOST;
#ifdef EDGED_TRACK
  else if (checkEdge(val) != 0)
    return checkEdge(val);
#endif
  else
    return line;
}
