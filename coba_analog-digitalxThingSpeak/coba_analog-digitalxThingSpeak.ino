#include <ThingSpeak.h>
#include <ESP8266WiFi.h>

const char* ssid = "xxxxx";   // your network SSID (name) 
const char* pass = "xxxxx";   // your network password
unsigned long myChannelNumber = 1236195;
const char * myWriteAPIKey = "U3OEV19XU0GX19L6";
WiFiClient  client;

//////////////////////////////////////////////////////////////////////////////////////////
#include <LiquidCrystal_I2C.h>
#include <Adafruit_ADS1015.h>
#include <Wire.h>
#include <BH1750.h>
#include "DHTesp.h"

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
  Serial.begin(115200);  // Initialize serial
  WiFi.mode(WIFI_STA); 
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  
  lcd.begin();
  lcd.backlight();
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
   // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }

  int16_t adc0, adc1, adc2, adc3;
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
  long duration, distance;
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

  // set the fields with the values
  ThingSpeak.setField(1, ads_Temperature_adc0);
  ThingSpeak.setField(2, lux);
  ThingSpeak.setField(3, outputPercentage);
  ThingSpeak.setField(4, distance);
  ThingSpeak.setField(5, tempdht22);
  ThingSpeak.setField(6, humdht22);

  // set the status
  //  ThingSpeak.setStatus(myStatus);
  
  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  delay(20000); // Wait 20 seconds to update the channel again


}
