#include <Button.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>

boolean debug = true;            // extra debug info?

boolean movement;                   // holds movement detection
long previousMillis = 0;            // previous checked time
long interval = 5000;               // amount of seconds required to say movement has stopped
int calibration = 10;            // time in seconds used in calibration
int avgBPM = 0;                // average BPM to be used as baseline, comes from calibration

volatile int myBPM = 0;          // for calculating average BPM  baseline
long snoozeMillis = 0;          // timekeeper for snoozing checking
long snoozeTime = 10000;        // time between snoozes
float percentValue = 0.95;      // how much lower must it go


//  Variables
int pulsePin = 5;                 // Pulse Sensor purple wire connected to analog pin 0
int blinkPin = 13;                // pin to blink led at each beat
int fadePin = 10;                  // pin to do fancy classy fading blink at each beat
int fadeRate = 0;                 // used to fade LED on with PWM on fadePin

// Volatile Variables, used in the interrupt service routine!
volatile int BPM;                   // int that holds raw Analog in 0. updated every 2mS
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // int that holds the time interval between beats! Must be seeded! 
volatile boolean Pulse = false;     // "True" when User's live heartbeat is detected. "False" when not a "live beat". 
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.
static boolean serialVisual = true;   // Set to 'false' by Default.  Re-set to 'true' to see Arduino Serial Monitor ASCII Visual Pulse 


const int motorPin = 9;
Button          button(11);

void setup() {
    pinMode(motorPin, OUTPUT);
    
    Serial.begin(115200);             // we agree to talk fast!
    Serial.println("setup");
    interruptSetup();                 // sets up to read Pulse Sensor signal every 2mS 

}

void loop() {
   static enum { CRASHED, CALIBRATION,  DETECTION, WAKEYWAKEY, SNOOZE } state = CALIBRATION;

  checkBeat();
  //checkMovement();

  
  switch (state) {
      case CRASHED: {
        if(debug){    
        Serial.print("state = ");
        Serial.println(state);
        }
        
        break;
      }
 
    
  case CALIBRATION: {
       
     if (BPM < 50){
        if(debug){    
        Serial.println("BPM < 50");
        }
       
       break;
      }
      if(debug){    
        Serial.println("state = calibration");
        }
    // fill array with BPM measurements over 50000 milliseconds, then take the average of that
    // and put that in the global variable avgBPM for the baseline reading
    for (int i = 0; i<calibration; i++){
      myBPM = myBPM + BPM;

        if(debug){    
        Serial.print("BPM = ");
        Serial.println(BPM);
        }

      delay (1000);
    }
    avgBPM = myBPM / calibration;
    Serial.print("avgBPM: ");
    Serial.print(avgBPM);
    Serial.println(".");
    state = DETECTION;
    break;  
    }
 

    
    
    case DETECTION: {
        if(debug){    
        Serial.println("state = detection");
        }
   // Detect the onset sleep
   // accelerometer is low for X
   // BPM is low for x

      // if weird reading , break
     if (BPM < 10) break;
         
     // check if falling asleep
     state = (BPM <= (avgBPM*percentValue) && !movement) ? WAKEYWAKEY : DETECTION;    
     break;  
    }
  
  
  
  case WAKEYWAKEY: {
        if(debug){    
        Serial.println("state = wakeywakey");
        }  
    
     switch(button.pressed()) {
        case SHORT:
           state = SNOOZE;
           digitalWrite(motorPin, LOW); 
           snoozeMillis = millis();
           break;
        case LONG:
          digitalWrite(motorPin, LOW); 
          state = CALIBRATION;
          break;
        default:
            digitalWrite(motorPin, HIGH);
            state = WAKEYWAKEY;
            break;
      }
       break;
   }
    
    
    
    
   case SNOOZE: {
        if(debug){    
        Serial.println("state = snooze");
        }
 
       // User ihas indicated is still awake
       if (millis() - snoozeMillis> snoozeTime){
         state = WAKEYWAKEY;
       }
      break;
    }

    
  }
  delay(10);
  

  
}


  
  
void checkMovement(){
      //check current state
     if (movement = true){
       if (millis()-previousMillis>interval){
         movement = false;
       }
     }
       
       /*accelerometer = positive*/
     if (1){
       if (!movement){
         movement = true;
       }
     previousMillis = millis();
     }   
 }
  
  
 void checkBeat(){
     
    serialOutput() ;       
    //  A Heartbeat Was Found
    // Quantified Self "QS" true when arduino finds a heartbeat
      if (QS == true){     
        serialOutputWhenBeatHappens();   // A Beat Happened, Output that to serial.     
            QS = false;                      // reset the Quantified Self flag for next time    
        }          
      delay(20);                             //  take a break
 }

