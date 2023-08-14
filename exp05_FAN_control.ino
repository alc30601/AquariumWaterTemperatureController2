//-------------------------------------------------
// Aquarium Water Temperature Controller 2
//-------------------------------------------------

#include <OneWire.h>
#include <DallasTemperature.h>

#define FAN_PWM_PIN 3
#define DS18B20_PIN 2
#define SWITCH_PIN 4

// between T0 - T1, FAN duty is proportional by Temperature.
// Lower than T0, FAN duty is set to   0 (0%)
// Upper than T1, FAN duty is set to 250 (100%)
#define T0 28.0   // lower threshold for temperature control
#define T1 30.0   // upper threshold for temperature control

#define ONEPERIOD 2000  // one period in ms

// Instance for OneWire connected Temperature sensor (DS18B20)
OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);

//-------------------------------------------------
void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);         // For LED Indicator
  analogWrite(FAN_PWM_PIN, 0);      // For FAN
  sensors.setResolution(9);             // For Temperature Sensor
  pinMode(SWITCH_PIN, INPUT_PULLUP);    // For Switch
  Serial.begin(9600);                   // For USB-Serial
}

//-------------------------------------------------
// read integer value from PC via USB-serial
// return the received integer value
// This function is used for debug purpose.
int readValueFromPC(void)
{
  String line;
  int length;
  int readValue;

  line = Serial.readStringUntil('\n');
  length = line.length();
  if(length > 0){
    readValue = line.toInt();
  }
  else {
    readValue = -1;
  }
  return readValue;
}

//-------------------------------------------------
// Get Temperature from DS18B20 via OneWire
// by using libralies below.
// https://github.com/PaulStoffregen/OneWire
// https://github.com/milesburton/Arduino-Temperature-Control-Library
float getTemperature(void)
{
  float temperature = 0.0;
  sensors.requestTemperatures();
  temperature = sensors.getTempCByIndex(0);  
  return temperature;
}

//-------------------------------------------------
// Calculate FAN PWM duty.
// Duty can take from 0 to 255 - this means 0% to 100%
// 
int calculateFanPower(float temperature)
{
  int value = 0;

  if(temperature < T0) {
    value = 0;
  }
  else if(T1 <= temperature) {
    value = 250;
  }
  else {
    value = 125 * (temperature - T0);
  }
  return value;
}

//-------------------------------------------------
int readSwitch(void)
{
  int value = digitalRead(SWITCH_PIN);
  return value;
} 

//-------------------------------------------------
// Blink LED a constant period
// blink speed depends on the FAn duty
// If switch is Off, LED always turn off.
void indicateLED(int sw, int duty)
{
  // The higher temperature, the faster the blink speed.
  // lower limit is 1000ms, higher limit is 100ms.
  // the domain (that is duty) is limited between 0 to 250.
  int blink = (int)(-3.6*(float)duty + 1000.0);

  unsigned long startTime = millis();
  unsigned long endTime = startTime + ONEPERIOD;

  while(millis() <= endTime) {
    if(sw == 1) {
      digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
      delay(blink);                    // wait for a second
    }
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    delay(blink); 
  }
}

//-------------------------------------------------
// log data to PC
void logSerial(int sw, float temperature, int fan)
{
  Serial.print(" Switch: ");
  Serial.print(sw);

  Serial.print(" Temperature: ");
  Serial.print(temperature);

  Serial.print(" PWM duty: ");
  Serial.print(fan);

  Serial.println("");
}

//-------------------------------------------------
void loop()
{
  int fanPower = 0;
  float temperature = 0.0;
  int sw = 0;

  // Get temperature from DS18B20
  temperature = getTemperature();

  // calculate duty value for FAN control depend on temperature
  fanPower = calculateFanPower(temperature);

  // Read switch On/Off
  // If sw is Off FAN must be stopped.
  sw = readSwitch();
  if(sw == 0){
    fanPower = 0;
  }

  // set the calculated duty value to FAN
  analogWrite(FAN_PWM_PIN, fanPower);

  // current data are sent to PC via USB-serial
  logSerial(sw, temperature, fanPower);

  // Blink on board LED
  indicateLED(sw, fanPower);
}

