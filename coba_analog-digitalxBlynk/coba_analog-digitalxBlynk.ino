#include <PCF8574.h>
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>


//////////////////////////////////////////////////////////////////////////////////////////
#include <LiquidCrystal_I2C.h>
#include <Adafruit_ADS1015.h>
#include <BH1750.h>
#include <DHTesp.h>

const char* ssid = "BURJONI";   // your network SSID (name) 
const char* pass = "tanyajoni";   // your network password
const char* auth = "lbUTcSbiLpYyP_IHpTUGQUJlW5faFSVF"; //auth token
WiFiClient  client;

#define TRIGGER 2
#define ECHO 0
#define relay1 D6
#define relay2 D7

Adafruit_ADS1115 ads(0x48);  /* Use this for the 16-bit version */
LiquidCrystal_I2C lcd(0x27, 16, 2);
BH1750 lightMeter(0x23);
DHTesp dht;
BlynkTimer timer;
PCF8574 pcf8574(0x39);

#define ADC_16BIT_MAX 65536
int outputVal = 0;
int soilMoistPercentage;
float ads_bit_Voltage;
float lm35_constant;
int16_t adc0, adc1;
float ads_Voltage_adc0 = 0.0f;
float templm35 = 0.0f;

void setup(void) 
{
  Serial.begin(115200);  // Initialize serial
  WiFi.mode(WIFI_STA); 
//lcd setting
  lcd.begin();
  lcd.backlight();
  
//pinMode HCSR
  pinMode(TRIGGER, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(BUILTIN_LED, OUTPUT);

//ADS setting
  ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  ads.begin();
  
//konversi nilai LM35
  float ads_InputRange = 6.144f;
  ads_bit_Voltage = (ads_InputRange * 2) / (ADC_16BIT_MAX - 1);
  lm35_constant = 10.0f / 1000;

//cahaya
  lightMeter.begin();
  
//dht22
  dht.setup(14, DHTesp::DHT22); // Connect DHT sensor to GPIO 14
//relay
  pinMode(relay1, INPUT);
  pinMode(relay2, INPUT);

  Blynk.begin(auth, ssid, pass);
  timer.setInterval(1000L, sendSensor);
}

void sendSensor() 
{

  //adc0 - suhu (LM35)
  adc0 = ads.readADC_SingleEnded(0);
  ads_Voltage_adc0 = adc0 * ads_bit_Voltage;
  templm35 = ads_Voltage_adc0 / lm35_constant;


  //adc1 - soil moisture level
  adc1 = ads.readADC_SingleEnded(1);
  soilMoistPercentage = map(adc1, 0, 32767, 100, 0);


  //sensor cahaya
  uint16_t lightGY = lightMeter.readLightLevel();
  
  
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

  //Blynk
  Blynk.virtualWrite(V1, humdht22); //hum dht22
  Blynk.virtualWrite(V2, tempdht22); //temp dht22
  Blynk.virtualWrite(V3, templm35); //temp lm35
  Blynk.virtualWrite(V4, lightGY); //light
  Blynk.virtualWrite(V5, soilMoistPercentage); //soil
  Blynk.virtualWrite(V6, distance); //distance
  
  //lcd lm35
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(templm35);

  // lcd light
  lcd.print("L:");
  lcd.print(lightGY);

  //lcd soil
  lcd.print("M:");
  lcd.print(soilMoistPercentage);
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
}

void loop(){
  Blynk.run();
  timer.run();
  if(templm35 > 28){
  digitalWrite(relay1, LOW);
  } else {
  digitalWrite(relay1, HIGH);
  }

  if(soilMoistPercentage > 60){
    digitalWrite(relay2, LOW);
  } else {
    digitalWrite(relay2, HIGH);
  }
}
