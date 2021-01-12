#include <AntaresESP8266HTTP.h>
#include <ArduinoJson.h>

#define ACCESSKEY "549e0645e0e025ab:ef10660f27ac1689"
#define WIFISSID "xxxxx"
#define PASSWORD "xxxxx"

#define projectName "projectEmbed"
#define deviceName "ProjectEmbed1"

AntaresESP8266HTTP antares(ACCESSKEY);

#include <LiquidCrystal_I2C.h>
#include <Adafruit_ADS1015.h>
#include <Wire.h>
#include <BH1750.h>
#include <DHTesp.h>

#define TRIGGER 2
#define ECHO 0


Adafruit_ADS1115 ads(0x48);  /* Use this for the 16-bit version */
LiquidCrystal_I2C lcd(0x27, 16, 2);
BH1750 lightMeter(0x23);
DHTesp dht;

#define ADC_16BIT_MAX 65536
int outputVal = 0;
int outputPercentage;
float ads_bit_Voltage;
float lm35_constant;


void setup(void) 
{
  Serial.begin(115200);
  lcd.begin();
  lcd.backlight();

  antares.setDebug(true);
  antares.wifiConnection(WIFISSID,PASSWORD);
  pinMode(TRIGGER, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(BUILTIN_LED, OUTPUT);
 
  ads.setGain(GAIN_TWOTHIRDS);
  ads.begin();
  float ads_InputRange = 6.144f;
  ads_bit_Voltage = (ads_InputRange * 2) / (ADC_16BIT_MAX - 1);
  lm35_constant = 10.0f / 1000;

  lightMeter.begin();
//dht22
  dht.setup(14, DHTesp::DHT22); // Connect DHT sensor to GPIO 14
}

void loop(void) 
{
  int16_t adc0, adc1;
  float ads_Voltage_adc0 = 0.0f;
  float ads_Temperature_adc0 = 0.0f;

  //adc0 - suhu (LM35)
  adc0 = ads.readADC_SingleEnded(0);
  ads_Voltage_adc0 = adc0 * ads_bit_Voltage;
  ads_Temperature_adc0 = ads_Voltage_adc0 / lm35_constant;

  //adc1 - soil moisture level
  adc1 = ads.readADC_SingleEnded(1);
  outputPercentage = map(adc1, 0, 32767, 100, 0);
  delay(1000);

  //sensor cahaya
  uint16_t lux = lightMeter.readLightLevel();
  
  //sensor jarak
  float duration, distance;
  digitalWrite(TRIGGER, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER, LOW);
  duration = pulseIn(ECHO, HIGH);
  distance = (duration/2) / 29.1;
  
  //sensor DHT22
  delay(dht.getMinimumSamplingPeriod()+ 200);
  float humdht22 = dht.getHumidity();
  float tempdht22 = dht.getTemperature();
  
  //lcd lm35
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(ads_Temperature_adc0);

  // lcd light
  lcd.print("L:");
  lcd.print(lux);

  //lcd soil
  lcd.print("M:");
  lcd.print(outputPercentage);
  delay(1000); 
  
  //lcd jarak
  lcd.setCursor(0, 1);
  lcd.print("D:");
  lcd.print(distance);

  //lcd dht22temp
  lcd.print("T:");
  lcd.print(tempdht22);

  //lcd dht22hum
  lcd.print("H:");
  lcd.print(humdht22);
  delay(1000);
  
  // Add variable data to storage buffer
  antares.add("temperature", ads_Temperature_adc0);
  antares.add("light", lux);
  antares.add("soil moisture", outputPercentage);
  antares.add("jarak", distance);
  antares.add("temperature dht22",tempdht22);
  antares.add("humidity dht22",humdht22);

   // Send from buffer to Antares
  antares.send(projectName, deviceName);
  delay(10000);
}
