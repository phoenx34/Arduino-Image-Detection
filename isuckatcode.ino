#include <Servo.h>
#include <PS3BT.h>
#include <usbhub.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif

#include <QTRSensors.h>
#include <Button.h>

#define DEBUG
#define POWER
#define AUTONOMOUS
#define USER_CONTROL
#define BLOCK_VISIBLE

#define PULLDOWN LOW
#define PULLUP HIGH

#define NUM_OF_SENSORS 8
#define MIDDLE_POS (((NUM_OF_SENSORS*1000) - 1000) / 2)

//#define EDGED_TRACK

#define WHITE_TRASHOLD 300
#define BLACK_TRASHOLD 100

#define FORWARD_MAX 255
#define REVERSE_MAX 20
#define CALIBRATION_SPEED 150

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

#define LINE_LOST -1
#define EDGE_LEFT -2
#define EDGE_RIGHT -3

int TRACKING_WHITE = 1;


boolean movingForward = true;
boolean movingBackwards = false;
boolean turningRight = false;
boolean turningLeft = false;
boolean worstCaseBreak = false;
bool printTemperature, printAngle;

Button button = Button(14,PULLDOWN);
const int ledPin = 15;
QTRSensorsRC qtr((unsigned char[]) {2,3,4,5,6,7,8,9}, NUM_OF_SENSORS);

int light_pins[9] ={A0, A1, A2, A3, A4, A5, A6, A7, A8};
int calibrated = 300;

/*
 Example sketch for the PS3 Bluetooth library - developed by Kristian Lauszus
 For more information visit my blog: http://blog.tkjelectronics.dk/ or
 send me an e-mail:  kristianl@tkjelectronics.com
 */


USB Usb;
//USBHub Hub1(&Usb); // Some dongles have a hub inside

BTD Btd(&Usb); // You have to create the Bluetooth Dongle instance like so
/* You can create the instance of the class in two ways */
//PS3BT PS3(&Btd); // This will just create the instance
PS3BT PS3(&Btd, 0x58, 0xC0, 0x71, 0xDA, 0xBF, 0x3D); // This will also store the bluetooth address - this can be obtained from the dongle when running the sketch



int stepper1PWM = 2;
int stepper1DIR = 3;
int stepper2PWM = 4;
int stepper2DIR = 5;

// Calibrate sensors for similarity matching between
// blue and white values. Runs before color detecting/autonomous section
// However, this still can be more accurate because the white color will
// always detect some blue. What percentage do we use?
// At what point between 0 and 255 does blue become white, and vice versa?
// Generally you would avoid this by comparing two resistors per data point,
 // instead of trying to compare each to some "predefined" convergence point
void setup() {

    Serial.begin(115200);
#if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
  if (Usb.Init() == -1) {
    Serial.print(F("\r\nOSC did not start"));
    while (1); //halt
  }
  Serial.print(F("\r\nPS3 Bluetooth Library Started"));


  pinMode(ledPin, OUTPUT);
  engineSetup();

  while(button.isPressed() == false);
  calibrate(&qtr);
  while(button.isPressed() == false);

  delay(2000);

  motor(MotorA,FORWARD_MAX);
  motor(MotorB,FORWARD_MAX);

  
  
}

void loop() {

  boolean is_block = true; //For now it will detect the block infinitely.
  int block_counter = 0;
  if(is_block == true && block_counter < 5 )
  {
    int letter = do_color_detection();
    block_counter++;
    Serial.println("The block was detected as:");
    //Serial.println(letter);
  }

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
  Usb.Task();

    Serial.println("HALT1");
  //myservo.write(0);
    analogWrite(stepper1PWM,0);
    analogWrite(stepper2PWM,0);
    analogWrite(stepper1DIR,0);
    analogWrite(stepper2DIR,0);
    movingForward = false;
    movingBackwards = false;
    turningRight = false;
    turningLeft = false;
    worstCaseBreak = false;

      //delay(200);
          //Serial.print(F("\r\nLeftHatX: "));
          //Serial.print(PS3.getAnalogHat(LeftHatX));
          //Serial.print(F("\tLeftHatY: "));
          //Serial.print(255 - PS3.getAnalogHat(LeftHatY));
      //if(PS3.PS3Connected){
    if (PS3.PS3Connected || PS3.PS3NavigationConnected) { // The Navigation controller only have one joystick
    

      if (PS3.getAnalogHat(RightHatX) > 137 || PS3.getAnalogHat(RightHatX) < 117 || PS3.getAnalogHat(RightHatY) > 137 || PS3.getAnalogHat(RightHatY) < 117) {
        
        if(PS3.PS3Connected){
      
              if(PS3.getAnalogHat(RightHatY) > ((int)(255/2) +20) && PS3.getAnalogHat(RightHatY) < ((int)(255)+5) && PS3.getAnalogHat(RightHatX) < 177 && PS3.getAnalogHat(RightHatX) > 90){
                //if(PS3.getAnalogHat(RightHatX) < 3){
                if(turningLeft == false && turningRight == false && movingBackwards == false){
                  movingBackwards = true;
                  analogWrite(stepper1DIR,-1);
                  analogWrite(stepper2DIR,-1);
                  analogWrite(stepper1PWM,(PS3.getAnalogHat(RightHatY) - 3));
                  analogWrite(stepper2PWM,(PS3.getAnalogHat(RightHatY) - 3));
                  
                  Serial.print("Reverse:  ");
                  Serial.print(PS3.getAnalogHat(RightHatY));
                }
              } 
             
             else if(PS3.getAnalogHat(RightHatY) < ((int)(255/2)-20) && PS3.getAnalogHat(RightHatY) > ((int)(0)-5) && PS3.getAnalogHat(RightHatX) < 177 && PS3.getAnalogHat(RightHatX) > 90){
              //if(PS3.getAnalogHat(RightHatX) < 2){
              if(turningLeft == false && turningRight == false && movingForward == false){
                movingForward = true;
                analogWrite(stepper1DIR,0);
                analogWrite(stepper2DIR,0);
                analogWrite(stepper1PWM,(255 - abs(PS3.getAnalogHat(RightHatY))) );
                analogWrite(stepper2PWM,(255 - abs(PS3.getAnalogHat(RightHatY))) );
                Serial.print("Forward:  ");
                Serial.print(PS3.getAnalogHat(RightHatY));
                //delay(50);
              }
              //}
      
            }
      
            //delay(20);
            //+10
            else if(PS3.getAnalogHat(RightHatX) > ((int)(145)+20) && PS3.getAnalogHat(RightHatX) < ((int)(255+5)) && PS3.getAnalogHat(RightHatY) < 170 && PS3.getAnalogHat(RightHatY) > 90){        
             if(movingForward == false && movingBackwards == false && turningLeft == false){
                turningLeft = true;
                analogWrite(stepper1DIR,0);
                analogWrite(stepper2DIR,-1);
                analogWrite(stepper1PWM,(PS3.getAnalogHat(RightHatX) - 3));
                analogWrite(stepper2PWM,(155 - ((PS3.getAnalogHat(RightHatX))+ 100)));
                Serial.print("Turning Right:  ");
                Serial.print("\n");
                Serial.print((155 - (PS3.getAnalogHat(RightHatX))) + 100);
                Serial.print("\t RAW ANALOG_RIGHT_HAT_X: ");
                Serial.print(PS3.getAnalogHat(RightHatX));
              }
             }
             //-10
             else if(PS3.getAnalogHat(RightHatX) < ((int)(100)) && PS3.getAnalogHat(RightHatX) > ((int)(0-5)) && PS3.getAnalogHat(RightHatY) < 170 && PS3.getAnalogHat(RightHatY) > 90){
              if(movingForward == false && movingBackwards == false && turningRight == false){
                turningRight = true;
                analogWrite(stepper1DIR,-1);
                analogWrite(stepper2DIR,0);
                analogWrite(stepper1PWM,(155 - ((PS3.getAnalogHat(RightHatX))- 100)));
                analogWrite(stepper2PWM,(PS3.getAnalogHat(RightHatX)) + 25);
                Serial.print("Turning Left:  ");
                //Serial.print("\n");
                Serial.print((155 - (PS3.getAnalogHat(RightHatX) )) + 100);
                Serial.print("\t RAW ANALOG_RIGHT_HAT_X: ");
                Serial.print(PS3.getAnalogHat(RightHatX)+25);
              }
             }
             else{
                    Serial.println("HALT2");
                    //myservo.write(0);
                    analogWrite(stepper1PWM,0);
                    analogWrite(stepper2PWM,0);
                    analogWrite(stepper1DIR,0);
                    analogWrite(stepper2DIR,0);
                    movingForward = false;
                    movingBackwards = false;
                    turningRight = false;
                    turningLeft = false;
                    worstCaseBreak = false;
      
              
             }
              //Serial.println();
              
            }
      }
        
          
        
    
        // Analog button values can be read from almost all buttons
        if (PS3.getAnalogButton(L2) || PS3.getAnalogButton(R2)) {
          Serial.print(F("\r\nL2: "));
          Serial.print(PS3.getAnalogButton(L2));
          if (PS3.PS3Connected) {
            Serial.print(F("\tR2: "));
            Serial.print(PS3.getAnalogButton(R2));
          }
        }
      }
}

int do_color_detection()
{
  int cell_data[9];                                               // You have to populate an array containing the analog values of the resistance array. Arduino code is stupid.
  int j = 0;
  for(j = 0; j < 9; j++)
  {
    //delay(100);
    cell_data[j] = analogRead(light_pins[j]);
    Serial.println(cell_data[j]);
  }
  int i;
  double compare_me;
  int white_count = 0;
  for(i = 0; i < 9; i++)
  {
    compare_me = (double) similarity(cell_data[i]);

    //Serial.println("Percentage of white:");
    //Serial.println(compare_me);

    if(compare_me > 50 && compare_me < 100)     // No need to check if it is blue, because the int only should grow if the scan is white.
    {
      white_count += 1;
    }

  }

  if(white_count >= 7 )
  {
    return 1;                         // 1 = M
  }
  if(white_count == 5)
  {
    return 2;                         // 2 = E
  }
  if(white_count == 4 || white_count == 6)
  {
    return 3;                         // 3 if A
  }
  return 4;                           // 4 if T   *no need to check against a value, white count will never be negative, and has to be less than 4, so default it to T.
                                            // We also do not have a case where the block does not have a letter, so doing this is fine.
}

double similarity(int cell6)
{
  int toAbs = cell6 - calibrated;
  int diff = abs(toAbs);
  double similar = ((double)diff / (double)calibrated) * 100;
  return similar;
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

void calibrate(QTRSensorsRC* qtr){
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

int readLine(QTRSensorsRC* qtr){
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

