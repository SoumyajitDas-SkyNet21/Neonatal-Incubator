#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <DHT.h>
#define Type DHT11
int senseTempPin=5;
DHT HT(senseTempPin,Type);
LiquidCrystal_I2C lcd(0x27,16,2);
const int btn1Pin=6;
const int btn2Pin=7;
const int relayPin=2;
const int mtrPin1=3;
const int mtrPin2=4;
const unsigned long tempSensorInterval=1000;
const unsigned long btn1Interval=100;
const unsigned long btn2Interval=100;
const unsigned long lcdInterval=100;
unsigned long prevBtn1Time=0;
unsigned long prevBtn2Time=0;
unsigned long prevLcdTime=0;
unsigned long prevTempTime=0;
int btn1Value;
int btn2Value;
int setUpTime=2000;
int setTemp;
int tempC;
void setup(){
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  HT.begin();
  pinMode(relayPin,OUTPUT);
  pinMode(btn1Value,OUTPUT);
  pinMode(btn2Value,OUTPUT);
  Serial.println("Initializing...");
  lcd.print("Initializing...");
  setTemp=HT.readTemperature();
  delay(setUpTime);
  lcd.clear();
}
void loop(){
  unsigned long currentTime=millis();
  // Task_1: Read Temperature Sensor Value 
  if(currentTime-prevTempTime>=tempSensorInterval){
    tempC=HT.readTemperature();
    }
  // Task_2: Turn-On Heater(Light_Bulb) if setTemp>tempC
  if(setTemp>tempC){
    digitalWrite(relayPin,HIGH);
    digitalWrite(mtrPin1,LOW);
    digitalWrite(mtrPin2,LOW);
  }
  else if(setTemp<tempC){
    digitalWrite(mtrPin1,HIGH);
    digitalWrite(mtrPin2,LOW);
    digitalWrite(relayPin,LOW);
  }
  else{
    digitalWrite(mtrPin1,LOW);
    digitalWrite(mtrPin2,LOW);
    digitalWrite(relayPin,LOW);
  }
  // Task_3: Read Button_1 Status 
  if(currentTime-prevBtn1Time>=btn1Interval){
    btn1Value=digitalRead(btn1Pin);
    prevBtn1Time=currentTime;
   }
  // Task_4: Read Button_2 Status
  if(currentTime-prevBtn2Time>=btn2Interval){
    btn2Value=digitalRead(btn2Pin);
    prevBtn2Time=currentTime;
   }
   // Task_5: Display set temperature information on LCD Screen.
   if(currentTime-prevLcdTime>=lcdInterval){
      if(btn1Value!=0){
        if(setTemp<50 and setTemp>0){
         setTemp=setTemp+1;
          lcd.clear();
          lcdDisplay(tempC,setTemp);
        }
     }
     else if(btn2Value!=0){
      if(setTemp<50 and setTemp>0){
        setTemp=setTemp-1;
        lcd.clear();
        lcdDisplay(tempC,setTemp);
       }
     }
     else
        {
          lcdDisplay(tempC,setTemp);
        }
     prevLcdTime=currentTime;
   }
   Serial.println(currentTime);
}
void lcdDisplay(int currtemp,int setTemp){
  lcd.setCursor(0,0);
  lcd.print("Room_Temp: ");
  lcd.print(currtemp);
  lcd.print(char(223));
  lcd.print("C");
  lcd.setCursor(0,1);
  lcd.print("Set_Temp : ");
  lcd.print(setTemp);
  lcd.print(char(223));
  lcd.print("C");
}
