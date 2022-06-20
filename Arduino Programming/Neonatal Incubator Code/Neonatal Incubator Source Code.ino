#define gatePin  9  // Must be a PWM pin to control fan speed.
#define relayPin 8   // Relay pin for auto-cutoff heater after desired temp. is reached. 
#define buzzPin  7   // Emerrgency Buzzer Signal Pin.
#include<SimpleDHT.h> // Library required for dht11(Temperature) sensor. 
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// GPIO where the DS18B20 is connected to
const int oneWireBus = 2;     

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

// We are using two I2C 16X2 LCD's with same SCl and SCA but with different address.
// LCD-1 :  System Temperature and Set Temperature 
LiquidCrystal_I2C lcd1(0x27,20,4);  // set the LCD address 0x27 ( All are open)
// LCD-2  : Status for Heater-On or Heater-Off.
LiquidCrystal_I2C lcd2(0x26,20,4);  // set the LCD address 0x26(A0 connected)

// DHT-11 Sensor Initialization

int dhtPin=10; // digital pin used by temperature sensor.
SimpleDHT11 dht11(dhtPin);

// Using Arduino pins 2 and 3 for setting temperature ( Increase / Decrease). 
const int btn1Pin=3; 
const int btn2Pin=4;
const unsigned long btn1Interval=100;
const unsigned long btn2Interval=100;
const unsigned long lcdInterval=100;
unsigned long prevBtn1Time=0;
unsigned long prevBtn2Time=0;
unsigned long prevLcdTime=0;
double  btn1Value;
double  btn2Value;
int setUpTime=5000; //  This the SetUpTime for System to start.
double setTemp=32.0;

const int ledPin=13;
int heaterState=0;
int fanSpeed=160;

void setup(){
  Serial.begin(9600);
  pinMode(ledPin,OUTPUT);
  pinMode(gatePin,OUTPUT);
  pinMode(relayPin,OUTPUT);
  pinMode(buzzPin, OUTPUT);
  // Initialize LCD-1
  lcd1.init();
  lcd1.backlight();
  Serial.println("Initializing...");
  lcd1.print("Initialiazing...");

  // Initialize LCD-2
  lcd2.init();
  lcd2.backlight();
  Serial.println("Initializing...");
  lcd2.print("Initialiazing...");

  pinMode(relayPin,OUTPUT);
  // Reading the temperature 
  sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);
  Serial.print(temperatureC);
  Serial.println("ºC");

  
  
  delay(setUpTime);

  // Clearing both  LCD screen. 
  lcd1.clear();
  lcd2.clear();
  
}
void loop(){

  // ----------------------- DHT-11 Humidity Sensor Section ------------------------
  byte temperature=0;
  byte humidity=0;
  int err = SimpleDHTErrSuccess;
  if ((err = dht11.read(&temperature,&humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT11 failed, err="); 
    Serial.print(SimpleDHTErrCode(err));
    Serial.print(","); 
    Serial.println(SimpleDHTErrDuration(err));
    delay(1000);
    return;
  }

  // Setting the Speed of fan to 62 % of its full capacity. ( 160/255 *100 = 62)
  // This fan will run all the time
  analogWrite(gatePin,fanSpeed);
  
  sensors.requestTemperatures(); 
  double temperatureC = sensors.getTempCByIndex(0);
  Serial.print("Temperature :-> ");
  Serial.print(temperatureC);
  Serial.print("ºC");
  Serial.print(" | Humidity :-> ");
  Serial.print(humidity); 
  Serial.println(" %");
  
  unsigned long currentTime=millis();


  // >>>>>>>>>>>>>>>>>>>>>>>>>>>>  CUT - OFF Section <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
   if(setTemp>temperatureC){
    // Turning -On Peltier by turning on relay.
    
    digitalWrite(relayPin,LOW);
    digitalWrite(ledPin,HIGH);
    // Show Heater Status ON
    heaterState=1;
    lcdDisplay2(heaterState,humidity);
    fanSpeed=160;
    delay(300);
   }
   else if(setTemp<=temperatureC){
    // Turning -Off Peltier by turning on relay.
    digitalWrite(relayPin,HIGH);
    digitalWrite(ledPin,LOW);
    // Show Heater Status OFF
    heaterState=0;
    lcdDisplay2(heaterState,humidity);
    fanSpeed=225;
    delay(500);
   }
    
  
  // Task_1: Read Button_1 Status 
  if(currentTime-prevBtn1Time>=btn1Interval){
    btn1Value=digitalRead(btn1Pin);
    prevBtn1Time=currentTime;
   }
  // Task_2: Read Button_2 Status
  if(currentTime-prevBtn2Time>=btn2Interval){
    btn2Value=digitalRead(btn2Pin);
    prevBtn2Time=currentTime;
   }
   // Task_3: Display set temperature information on LCD Screen.
   if(currentTime-prevLcdTime>=lcdInterval){
      if(btn1Value!=0){
       setTemp=setTemp+0.5;   // 0.5 *C Increase / Press  ( Least Count - 0.5 *C) 
        lcd1.clear();
        lcdDisplay1(temperatureC,setTemp);
     }
     else if(btn2Value!=0){
      setTemp=setTemp-0.5;  // 0.5 *C   Decrease / Press  ( Least Count - 0.5 *C) 
      lcd1.clear();
      lcdDisplay1(temperatureC,setTemp);
     }
     else
        {
          lcdDisplay1(temperatureC,setTemp);
        }
     prevLcdTime=currentTime;
   }

   // ----------------------- Emergency Event  ------------------------ 
   emergencyProtocol(temperatureC);
   
   // Delay for entire loop 
   delay(100);
}

void lcdDisplay1(double currtemp,double setTemp){
  lcd1.setCursor(0,0);
  lcd1.print("In_Temp: ");
  lcd1.print(currtemp);
  lcd1.print(char(223));
  lcd1.print("C");
  lcd1.setCursor(0,1);
  lcd1.print("S_Temp : ");
  lcd1.print(setTemp);
  lcd1.print(char(223));
  lcd1.print("C");
}
void lcdDisplay2(int heaterState,int humidity){
  lcd2.setCursor(0,0);
  if(heaterState==1){
  lcd2.print("<<<  Heating >>>");
  lcd2.setCursor(0,1);
  lcd2.print("   Humid - ");
  lcd2.print(humidity);
  lcd2.print(" %");
  }
  else
  {
    lcd2.print("<<<  Normal >>>");
    lcd2.setCursor(0,1);
    lcd2.print("   Humid - ");
    lcd2.print(humidity);
    lcd2.print(" %");
  }
}
void emergencyProtocol(double temperatureC){
  if(temperatureC >38.50)
   {
      // Turning-On Buzzer
      for(int i=0;i<5;i++)
      {
        digitalWrite(buzzPin,HIGH);
        delay(100);
        digitalWrite(buzzPin,LOW);
        delay(100);
      }
      // Turning-Off Peltier
      digitalWrite(relayPin,LOW);
      delay(500);
   }
}
  
