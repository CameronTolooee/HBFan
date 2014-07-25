#line 1 "ThermometerRead.ino"
#include <Time.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into pin 3 on the Arduino
#define ONE_WIRE_BUS 3
// Setup a oneWire instance to communicate with any OneWire devices
#include "Arduino.h"
void setup(void);
void initTemp (String code);
void fanOn();
void fanOff();
void checkTemps();
void handleCommand(String code);
String readCommand();
void loop(void);
#line 8
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);


const int FAN_POWER = 10;
const int SENSOR_POWER = 7;
int thresholdTemp = -1;
long totalFanTime = 0;
long startTime;
long fanActivity = -1;
int toggle = 0;

DeviceAddress thermometerA = { 0x28, 0x46, 0xBA, 0xE6, 0x03, 0x00, 0x00, 0x8F };
DeviceAddress thermometerB = { 0x28, 0x22, 0xDC, 0xE6, 0x03, 0x00, 0x00, 0x1D };

void setup(void)
{
  // start serial port
  Serial.begin(9600);
  
  // Power supply for the fan
  pinMode(FAN_POWER, OUTPUT);
  pinMode(SENSOR_POWER, OUTPUT);
  digitalWrite(SENSOR_POWER, HIGH);
  digitalWrite(FAN_POWER, LOW);
  
  // Start up the library
  sensors.begin();
  // set the resolution to 10 bit (good enough?)
  sensors.setResolution(thermometerA, 10);
  sensors.setResolution(thermometerB, 10);
}

void initTemp (String code) {
  char buf[3];
  code.substring(1,3).toCharArray(buf, sizeof(buf));
  thresholdTemp = atoi(buf);
  fanActivity = now();
  startTime = -10; 
}

void fanOn(){
  long diff = now() - fanActivity;
  Serial.println(diff);
  if(diff > 10 && toggle == 0){
      toggle = 1;
      digitalWrite(FAN_POWER, HIGH);
      fanActivity = now();
  }
}

void fanOff(){
  long diff = now() - fanActivity;
  if(diff > 10 && toggle == 1){
      toggle = 0;
      digitalWrite(FAN_POWER, LOW);
      totalFanTime += now() - fanActivity;
      fanActivity = now();
  }
}


void checkTemps()
{
  if(thresholdTemp != -1){
    sensors.requestTemperatures();
    float temp_A= DallasTemperature::toFahrenheit(sensors.getTempC(thermometerA));
    float temp_B = DallasTemperature::toFahrenheit(sensors.getTempC(thermometerB));
    if (temp_A == -127.00 || temp_B == -127.00) {
      Serial.print("Error");
    } else if (temp_A > thresholdTemp  || temp_B > thresholdTemp) {
      fanOn();
      Serial.print(temp_A);
      Serial.print(", ");
      Serial.println(temp_B);
    } else if (temp_A <= thresholdTemp && temp_B <= thresholdTemp) {
      fanOff();
      Serial.print(temp_A);
      Serial.print(", ");
      Serial.println(temp_B);
    }
  }
}

// Handles the serial command given 
void handleCommand(String code)
{
  switch (code[0]) {
    case 't':
        sensors.requestTemperatures();
        break;
    case 'i': initTemp(code);
        break;
    case 'p': Serial.write(totalFanTime);
    Serial.write(startTime - now());
    Serial.write(totalFanTime/(startTime - now()));
    default: ;
        
    }
}

// reads command from serial, commands are 5 characters, 
// 1: indentifier
// 2: number
String readCommand()
{
  int bytes = Serial.available(); 
  String command = "";
  char character;
  if(bytes >= 5) { //if buffer has 5 or more characters, read 5 at a time
    for (int i = 0; i < 5; ++i) {
      character = Serial.read();
      command.concat(character);
    }
  } 
  return command; // returns empty string if invalid
}

void loop(void)
{ 
  checkTemps();
  String command = readCommand();
  if (!command.equals("")){
    handleCommand(command); 
  }
}



