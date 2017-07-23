//    Banto as Shipped  July 2017
//
//
//  200 step motor 
//  10 micro steps
//  2000 steps per Rev
//  
#include <AccelStepper.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

const int myInput = AUDIO_INPUT_LINEIN;
//const int myInput = AUDIO_INPUT_MIC;
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            audioInput;     //xy=109.16667175292969,132
AudioAnalyzeRMS          rms_L;          //xy=779.1666870117188,133
AudioAnalyzePeak         peak_L;         //xy=781.1666870117188,89.00000762939453
AudioConnection          patchCord1(audioInput, 0, peak_L, 0);
AudioConnection          patchCord2(audioInput, 1, rms_L, 0);
AudioControlSGTL5000     audioShield;    //xy=161.1666717529297,634.0000305175781
// GUItool: end automatically generated code

// Define Pins
#define   STEPPER_STEP  33
#define   STEPPER_DIR   34
#define   STEPPER_LIMIT 2
#define   HOME_LIMIT   32
#define   END_LIMIT    31

#define   ANALOG_IN_00 A21
#define   ANALOG_IN_01 A22
#define   ANALOG_IN_02 A19
#define   ANALOG_IN_03 A20
//#define   ANALOG_IN_04 A21
//#define   ANALOG_IN_05 A22

#define   PING_PIN_01_TRIG 35
#define   PING_PIN_01_ECHO 36
#define   PING_PIN_03  9

#define   MIN_SWEEP   0
#define   MAX_SWEEP   500
#define   HOME_MAX    -1000

#define   RED_LED   16
#define   GREEN_LED 17

#define BARKER_DISTANCE   48   //inch to dectect some one out there
elapsedMillis fps;
elapsedMillis noAction;

uint8_t cnt=0;
int average[1000];
int averageCount =0;
int averageTotal =0;
int dot;
int dotSize = 1;
int dotThreshold =10;
AccelStepper stepper(AccelStepper::DRIVER, STEPPER_STEP, STEPPER_DIR); // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5
int analog_in;
int stepSizeBig = 7;
int stepSizeSmall =3;
int stepThresholdBig = 20;
int stepThresholdSmall = 10;
long currentDistance = 0;
//   int average = 0;

int currentPosition  = 0;

void setup(){  
  pinMode(HOME_LIMIT,INPUT_PULLUP);
  pinMode(END_LIMIT,INPUT_PULLUP);
  stepper.setMaxSpeed(2000.0);
  stepper.setSpeed(2000);
  stepper.setAcceleration(1000.0);	
  Serial.begin(115200);
//  while (!Serial) {       //Remove in not on the coumputer.
//  }
 //homeNoLimit();
 //showLimit();
 //homeLimitTest();
 delay(500);
 Serial.println(digitalRead(HOME_LIMIT));
 homeLimit();
 AudioMemory(6);
 audioShield.enable();
 audioShield.inputSelect(AUDIO_INPUT_MIC);
 audioShield.volume(0.5);
 Serial.begin(9600);
 pinMode(RED_LED,OUTPUT);
 pinMode(GREEN_LED,OUTPUT);
 digitalWrite(RED_LED,HIGH);
 digitalWrite(GREEN_LED,HIGH);
 pingDistanceTwoPinSetUp( PING_PIN_01_TRIG, PING_PIN_01_ECHO);
  delay(500);
}

int state = 1;
uint8_t leftPeak,leftRMS;


void loop(){
   if(fps > 50) {
    update_analog();
    if (peak_L.available() && rms_L.available()) {
      leftPeak  = peak_L.readPeakToPeak() * 15;
      leftRMS  =   rms_L.read() * 30  ;
      analog_in = 0;
   if ( leftPeak > stepThresholdBig){
      analog_in = stepSizeBig;    // No negitive
      }
      else{
      if ( leftPeak > stepThresholdSmall){
          analog_in = stepSizeSmall;
      }
    } 
   if (analog_in != 0){
      noAction =0;
      //state = 1;
  //showBArGragh(leftPeak,leftRMS);   
      }
    else{                   // This is where the Atrack mode happens
      currentDistance =   findDistance();
      if (BARKER_DISTANCE > currentDistance){
        if (state == 1){      
          if (noAction > 12000){  
              analog_in = 10;
              noAction = 0;
              state = 2;
              }
        }  
        if (state == 2){      
          if (noAction > 12000){  
              analog_in =- 10;
              noAction =0;
              state = 1;
              }
        } 
      }   
     }
   currentPosition = currentPosition + analog_in; 

   showAllData();
   Serial.println();   
  if (currentPosition > MAX_SWEEP ){
      //Serial.println("Reset");
      currentPosition = MIN_SWEEP ;  // may change speed here too??
      stepper.setMaxSpeed(50000.0);
      stepper.setSpeed(100000);
      stepper.setAcceleration(5000.0);
      stepper.runToNewPosition(currentPosition);
      delay(500);                                   
      //while(stepper.distanceToGo() > 0){
      //}
      //stepper.setMaxSpeed(5000.0);    //return to normal speed
      stepper.setSpeed(100000);
      stepper.setAcceleration(50000.0);
    }
    else{
    stepper.runToNewPosition(currentPosition);
    }
    fps=0;
    }
  }
}

void   showAllData(){
   Serial.print(analog_in,DEC);    
   Serial.print(" ,"); 
   Serial.print(currentPosition,DEC);    
   Serial.print(" ,");     
   Serial.print(leftPeak,DEC); 
   Serial.print(" ,"); 
   Serial.print(leftRMS,DEC); 
   Serial.print(" ,"); 
   Serial.print(stepThresholdSmall,DEC);
   Serial.print(" ,");      
   Serial.print(stepThresholdBig,DEC);
   Serial.print(" ,");     
   Serial.print(currentDistance,DEC);
   Serial.print(" ,");     
   Serial.print(stepSizeBig,DEC);
   Serial.print(" ,");     
   Serial.print(stepSizeSmall,DEC);
   Serial.print(",");
   Serial.print(state,DEC);
   Serial.print(",");  
   Serial.print(noAction,DEC);
   Serial.print(",");  
   Serial.print(digitalRead(HOME_LIMIT),DEC);
   }

void showBArGragh(int leftPeak,int leftRMS ){
      if (leftPeak > dotThreshold){
        if (dot == 1){
          Serial.print("           |           ");
          digitalWrite(RED_LED, LOW);
          //dot--;
          }
        else{
          dot =dotSize;
          Serial.print("           0           ");
          digitalWrite(RED_LED, HIGH);
//        Serial.print("                       ");
          }
        }
        else{
        Serial.print("                       ");
        digitalWrite(RED_LED, HIGH);
        dot = 1;
      }
      Serial.print("||");
      for(cnt=0; cnt < leftRMS; cnt++) {
        Serial.print("=");
      }
      for(; cnt < leftPeak; cnt++) {
        if (cnt > dotThreshold){
          Serial.print("-");
          }
        else{
          Serial.print(">");
          }
      }
      while(cnt++ < 30) {
        Serial.print(" ");
      }
  }

void runningAverage(){
        //average[averageCount]= rms_L.read();
      averageTotal = averageTotal + average[averageCount];
      averageCount++;
      if (averageCount == 999){
        averageCount =0;
      }
      averageTotal = averageTotal - average[averageCount];
      //Serial.println(fps);
    
}

long  pingDistanceTwoPin (int pingPinOut ,int pingPinIn){
  long duration;
  digitalWrite(pingPinOut, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPinOut, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPinOut, LOW);
  duration = pulseIn(pingPinIn, HIGH);
  return duration;  
  }
void pingDistanceTwoPinSetUp (int pingPinOut ,int pingPinIn){
  pinMode(pingPinOut, OUTPUT);
  pinMode(pingPinIn, INPUT);
  }

void update_analog(){
   // coment out the Var you do not want to update live. 
   //
   //
   stepSizeBig        = (1023-analogRead(ANALOG_IN_01))/40;
   stepSizeSmall      = (1023-analogRead(ANALOG_IN_00))/40;
   stepThresholdBig   = (1023-analogRead(ANALOG_IN_03))/20;
   stepThresholdSmall = (1023-analogRead(ANALOG_IN_02))/20;
   }


void homeNoLimit(){
  //Serial.println("Looking for home now");
  stepper.setMaxSpeed(2000.0);
  stepper.setSpeed(500);
  stepper.setAcceleration(1000.0);
       //Serial.println(i,DEC);
  stepper.runToNewPosition(HOME_MAX);
  //Serial.println("Home Done"); 
  stepper.setCurrentPosition(520);
  }

void showLimit(){
  Serial.println("TEST MODE");
  while(1==1){
    if (digitalRead(HOME_LIMIT) == HIGH)
    Serial.print("* ");
  else  
    Serial.print("- ");
  if (digitalRead(END_LIMIT) == HIGH)
    Serial.println("* ");
  else  
    Serial.println("- ");
   //Serial.print(digitalRead(HOME_LIMIT),DEC);
   //Serial.print(",");
   //Serial.println(digitalRead(END_LIMIT),DEC);
  delay(200);
  }
  }
  
void homeLimit(){
  int i= 0;
  //Serial.println("Looking for home now!!");
  stepper.setMaxSpeed(100.0);
  stepper.setSpeed(100);
  stepper.setAcceleration(1000.0);
       //Serial.println(i,DEC);
  stepper.moveTo(HOME_MAX);
  stepper.run();
while(stepper.distanceToGo() < -1){
    stepper.run();
    if (digitalRead(HOME_LIMIT)  == LOW){
      Serial.print("LOW   ");
      //stepper.stop();
    }
    else{
      delay(150);            // if it HIGH  re check in case it a spike.
      if (digitalRead(HOME_LIMIT)  == HIGH){
        Serial.print("HIGH  ");
        stepper.stop();
        stepper.runToPosition(); 
      }
      }
    Serial.println(stepper.distanceToGo(),DEC);   
}
  
  Serial.println("Hit Home Limit"); 
  Serial.println(digitalRead(HOME_LIMIT),DEC);
  stepper.stop();
  stepper.runToPosition();
  Serial.println("Home Done"); 
  stepper.setCurrentPosition(0);
  }


  
void homeLimitTest(){
  while(true){
  delay(300);
  if (digitalRead(HOME_LIMIT)  == HIGH) 
    Serial.println("LOW   ");
  }
}

long findDistance(){
  long currentDistanceNow,highDistance,lowDistance,aveTotal;
  int i;
/*  
  
  highDistance=0;
  lowDistance=100000;
  aveTotal =0;
  for (i=0;i++;i < 10){
    currentDistanceNow = pingDistanceTwoPin(PING_PIN_01_TRIG, PING_PIN_01_ECHO);
    aveTotal = aveTotal + currentDistanceNow;
    if (highDistance < currentDistanceNow){
      highDistance = currentDistanceNow;
      }
     if (lowDistance > currentDistanceNow){
      lowDistance = currentDistanceNow;
      }
    }
  aveTotal = highDistance;
  aveTotal = lowDistance;
  currentDistanceNow = aveTotal/8; 
*/
  // Not the right Fix but it works, need to fix this code. 
  currentDistanceNow = pingDistanceTwoPin(PING_PIN_01_TRIG, PING_PIN_01_ECHO);  
  if (currentDistanceNow < 10){
    currentDistanceNow = pingDistanceTwoPin(PING_PIN_01_TRIG, PING_PIN_01_ECHO);
    } 
  if (currentDistanceNow < 10){
    currentDistanceNow = pingDistanceTwoPin(PING_PIN_01_TRIG, PING_PIN_01_ECHO);
    }

     
  currentDistanceNow = microsecondsToInches(currentDistanceNow);
  return  currentDistanceNow;  
  }
  long microsecondsToInches(long microseconds) {
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds) {
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}
