#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

SoftwareSerial mySoftwareSerial(10, 11);
DFRobotDFPlayerMini myDFPlayer;

const int WARNING_NORMAL_TH_SOUND = 1;
const int WARNING_NORMAL_EN_SOUND = 4;
const int WARNING_ANGRY_TH_SOUND = 2;
const int WARNING_ANGRY_EN_SOUND = 3;
const int ALERT_SOUND = 5;

const int LED_COUNT = 9;
const int MIC_PIN = A0;
//const int PIEZO_PIN = 9;
const int ALARM_PIN = A5;
const int ALARM_SECOND = 4;
const int DF_PLAYER_MINI_VOLUME = 30;
const int ANGRY_THRESHOLD = 120; //120 seconds
const int SAMPLE_WINDOW = 100; // Sample window width in mS (50 mS = 20Hz)
const int MAX_MIC_SOUND = 40; //50
const int MAX_SOUND_TOTAL = 5;
const float LOOP_DELAY = 10.0; //10.0

int ledPins[] = {2,3,4,5,6,7,8,12,13};

unsigned long previousAlarm = 0;
boolean angrySound = false;

unsigned int sample;
unsigned long maxSoundCounter = 0;

void setup() {
  setupDFPlayerMini();  

  Serial.begin(9600);
  
  for(int thisLed = 0; thisLed < LED_COUNT; thisLed++){
    pinMode(ledPins[thisLed], OUTPUT);
  }

  pinMode(ALARM_PIN, OUTPUT);
  //pinMode(PIEZO_PIN, OUTPUT);

  

}

void loop() {
  /*int adc = analogRead(A0); //Read the ADC value from amplifer
  int dB = (adc+83.2073) / 11.003; //Convert ADC value to dB using Regression values
  Serial.println(dB);

  delay(200);*/
  
  int sensorReading = analogRead(MIC_PIN);
  //Serial.println(sensorReading); //normal is 516

  int ledLevel = calculateLedLevel(); //map(sensorReading, 535, 555, 0, LED_COUNT);
  
  //Serial.print(" : ");
  //Serial.println(ledLevel);

  
  for(int thisLed = 0; thisLed < LED_COUNT; thisLed++){
    if(thisLed < ledLevel){
      digitalWrite(ledPins[thisLed], HIGH);
    }else{
      digitalWrite(ledPins[thisLed], LOW);
    }
  }

  //Serial.println(maxSoundCounter);
  //Serial.println(getSeconds() - previousAlarm);

  //
  if(ledLevel == 9){
    maxSoundCounter++;

    if(maxSoundCounter >= MAX_SOUND_TOTAL){
      maxSoundCounter = 0;
      
      playAlarm(ALARM_SECOND);
    }
  }else{
    maxSoundCounter = 0;
  }

  //
  delay(LOOP_DELAY);

}

int calculateLedLevel(){
  unsigned long startMillis = millis();  // Start of sample window
  unsigned int peakToPeak = 0;   // peak-to-peak level
 
  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;
 
   while (millis() - startMillis < SAMPLE_WINDOW)
   {
      sample = analogRead(MIC_PIN); 
      //Serial.println(sample);
      if (sample < 1024)  // toss out spurious readings
      {
         if (sample > signalMax)
         {
            signalMax = sample;  // save just the max levels
         }
         else if (sample < signalMin)
         {
            signalMin = sample;  // save just the min levels
         }
      }
   }
   
   peakToPeak = constrain(signalMax - signalMin, 0, MAX_MIC_SOUND);
 
   // map 1v p-p level to the max scale of the display
   int displayPeak = map(peakToPeak, 0, MAX_MIC_SOUND, 0, 9);
  
  

   /*if(peakToPeak > 15){
    Serial.print("clap : ");
    Serial.println(peakToPeak);
   }*/
   
   Serial.print(peakToPeak);
   Serial.print(" | ");
   Serial.print(displayPeak);
   Serial.println();

   return displayPeak;
}

void playAlarm(int second){
  unsigned long currentAlarmTime = getSeconds();
  
  //play alarm
  for(int i = 0; i < second; i++){
    //play beep sound
    playSound(ALERT_SOUND, 0);
    
    digitalWrite(ALARM_PIN, HIGH);
    //analogWrite(PIEZO_PIN, 128);
    delay(700);
    
    digitalWrite(ALARM_PIN, LOW);
    //analogWrite(PIEZO_PIN, 0);
    delay(300);
  }

  //stop
  digitalWrite(ALARM_PIN, LOW);
  //analogWrite(PIEZO_PIN, 0);  

  //play sound
  unsigned long timeElapsed = currentAlarmTime - previousAlarm;
  
  Serial.print("Play Alarm: ");
  Serial.println(timeElapsed);
  
  if((previousAlarm == 0) || (timeElapsed > ANGRY_THRESHOLD) || (angrySound)){
    angrySound = false;
  }else{
    angrySound = true;
  }

  if(!angrySound){
     //th
     playSound(WARNING_NORMAL_TH_SOUND, 5000);
     //en
     playSound(WARNING_NORMAL_EN_SOUND, 5000);
  }else{
     //th
     playSound(WARNING_ANGRY_TH_SOUND, 5000);
     //en
     playSound(WARNING_ANGRY_EN_SOUND, 5000);
  }
  
  previousAlarm = getSeconds();
  
  
  
}

void playSound(int num, int d){
  myDFPlayer.play(num);
  delay(d);
}

void setupDFPlayerMini(){
  mySoftwareSerial.begin(9600);

  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  
  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    /*while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }*/
  }
  Serial.println(F("DFPlayer Mini online."));

  myDFPlayer.volume(DF_PLAYER_MINI_VOLUME);  //Set volume value. From 0 to 30
}

unsigned long getSeconds(){
  return millis() / 1000;
}
