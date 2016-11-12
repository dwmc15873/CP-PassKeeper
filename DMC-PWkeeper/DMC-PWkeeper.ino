// ---------------------------------------------------------------------------
// OgreDrew's Mental Regulator and Password Keeper
// by Drew McCray (ogredrew@gmail.com)
// code inspired by Circuit Playground Password Vault
// by John Park for Adafruit Industries
// ---------------------------------------------------------------------------

#include <Adafruit_CircuitPlayground.h>
#include <Wire.h>
#include <SPI.h>
#include <Keyboard.h>
#include <math.h>

//
//Unlock Sequence, pads #1, #3, #10, & #12, 6 digits
int LockCode[]   = { 10, 10, 10, 10, 10, 10}; 

// Password List. Blank entries could be bad, m'kay. 

char* PASSWORDS[] = {
  "PW1" ,
  "PW2" ,
  "PW3" ,
  "PW4" ,
  "PW5" ,
  "PW6" ,
  "PW7" ,
  "PW8" ,
  "PW9" ,
  "PW0" } ;
  
//Global Variables
int UnLockCode[] = {  0,  0,  0,  0,  0,  0}; // Used to compare entered code against configured code
long LastInput = 0;                           //Used for auto-locking timeout
long CurrentTime = 0;                         //For doing the timeout math
bool Timeout = 0;                             //To keep track of if we have timed out
int ModeSwitch = 0;                           //Are we being a throbber, or a vault?
bool Locked = 1;                              //Is the PW bank locked?
int InputCount = 0;                           //What character of lock code are we on
int PasswordSlot = 0;                         //Which password slot will we be sending?
  
void setup() {
    CircuitPlayground.begin();
    CircuitPlayground.redLED(true);
    LastInput = millis();
    Serial.begin(9600);
    CircuitPlayground.playTone(932,200);  //startup beep 
    CircuitPlayground.setBrightness(20);  //fairly dim, but these neos are BRIGHT
    CircuitPlayground.clearPixels();      //clean up if any were previously on
    Keyboard.begin();
    delay(200);
    CircuitPlayground.redLED(false);
}

void loop() {   CurrentTime = millis();
// Look for timeout condition
    if ((CurrentTime - LastInput) > 60000 && (Timeout == false)) {// Timeout at 1 minute
      Timeout = true;
      LastInput = millis();
      }
// Check slider for mode
    ModeSwitch = CircuitPlayground.slideSwitch(); 
    if (ModeSwitch == 0){
      Timeout = false;
      LastInput = millis();
      for(int i=0;i<10;i++){
          UnLockCode[i]=0;
      }
      InputCount = 0;
      Locked = 1;
      iThrob();
    }
    else if ((ModeSwitch == 1) && (Timeout == 1)){
      iThrob();
    }
    else {
    PWKeeper();
    }
}


void iThrob(){
   float sine = (exp(sin(millis()/2000.0*PI)) - 0.36787944)*108.0; //breathing sine based on runtime
   for(int i=0;i<10;i++){
         CircuitPlayground.setPixelColor(i, 0, sine, 0);
   }
}

void PWKeeper(){
  if (Locked == 1) {//check the cap switches for entry
    //Serial.print("PWKeeper Locked ");
    if (InputCount == 0){
       for(int i=0;i<10;i++){
        CircuitPlayground.setPixelColor(i, 255, 0, 0);
        delay(100);
      } 
    }
    if (InputCount < 6){
      int pad1=CircuitPlayground.readCap(1);
      int pad3=CircuitPlayground.readCap(3);
      int pad10=CircuitPlayground.readCap(10);
      int pad12=CircuitPlayground.readCap(12);
    
      //Anything from the touchpads?
      if (pad1 <25 && pad3<25 && pad10<25 && pad12<25) {
        //Serial.print("Waiting for input ");
      }
      else {
        LastInput = millis(); 
        if (pad1>25) {
          UnLockCode[InputCount]=1;//add code entry to the array
          CircuitPlayground.setPixelColor(InputCount,255,255,0);
          InputCount++;
          delay(400);
        } 
        else if (pad3>25) {
          UnLockCode[InputCount]=3;//add code entry to the array
          CircuitPlayground.setPixelColor(InputCount,255,255,0);
          InputCount++;
          delay(400);
        } 
        else if (pad10>25) {
          UnLockCode[InputCount]=10;//add code entry to the array
          CircuitPlayground.setPixelColor(InputCount,255,255,0);
          InputCount++;
          delay(400);
        } 
        else if (pad12>25) {
          UnLockCode[InputCount]=12;//add code entry to the array
          CircuitPlayground.setPixelColor(InputCount,255,255,0);
          InputCount++;
          delay(400);
        } 
      }
      if((InputCount>=6)){
        Serial.println("Checking Code");
        int codeCompare = memcmp(LockCode, UnLockCode, sizeof(LockCode)); //0 = match
        if (codeCompare==0){
          Serial.println("Valid! Unlocking. \n");
          Locked = 0; //WooHoo, Unlocked
          for (int i=0;i<10;i++) {
            CircuitPlayground.setPixelColor(i, 0, 255, 0);
            delay(100);
          }
          delay(500);
          for (int i=0;i<10;i++) {
             CircuitPlayground.setPixelColor(i, 0, 0, 128);
            delay(100); 
          }
        } 
      else{
        CircuitPlayground.playTone(232,200);  //beep for bad code
        InputCount = 0;
      } 

      }
    }
  }
  else if (Locked == 0){
    CircuitPlayground.setPixelColor(PasswordSlot, 128, 0, 128);
    bool Go1 = CircuitPlayground.leftButton();
    bool Select1 = CircuitPlayground.rightButton();
    delay(10);
    bool Go2 = CircuitPlayground.leftButton();
    bool Select2 = CircuitPlayground.rightButton();
    if (Select1 && !Select2) {
        LastInput = millis();
        PasswordSlot++; //increments the password slot
        CircuitPlayground.setPixelColor(PasswordSlot-1, 0, 0, 128); //Remmove the previous marker
        PasswordSlot = PasswordSlot % 10; //We only have 10 slots, so this fixes rollover
        CircuitPlayground.setPixelColor(PasswordSlot, 128, 0, 128);
    }    
    if (Go1 && !Go2){
      LastInput = millis(); 
      CircuitPlayground.playTone(432,200);
      delay(100);
      CircuitPlayground.setPixelColor(PasswordSlot, 255, 0, 255);
      Keyboard.print(PASSWORDS[PasswordSlot]) ;
      CircuitPlayground.playTone(500,300);
    }
  }
}
