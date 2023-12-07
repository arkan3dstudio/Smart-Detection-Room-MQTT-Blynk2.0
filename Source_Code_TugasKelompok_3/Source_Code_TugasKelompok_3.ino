/*
# File Name      : Smart Detection Room
# Group          : Kelompok- 3 
                   RIZA PAHLAWAN - 237038007
                   HILDA AYU TAMARA - 237038005
                   ADIL PRIMAN HATI HULU - 237038016
                   LEO DONNI NOFRIADI SITINDAON - 237038013

# Judul Research : Deteksi Suhu dan Api cerdas pada ruangan
                   Built-in ESP 8266 Wifi berbasis Kodular dan Protocol MQTT EMQX BROKER
                   dan Blynk Iot sebagai penyimpanan Data Logging Sensor.

# Deskripsi      : Perangkat Elektronika yang digunakan
                   untuk memonitoring suhu ruangan dan controling
                   secara realtime terhubung ke internet dengan media 
                   yang dapat di akses melalui aplikasi andriod smartphone 
                   yang telah di buat melalui Kodular menggunakan protocol & MQTT EMQX BROKER
                   aplikasi Blynk Iot (verse. 2.0) sebagai data sensor logging.
                
# Maker by       : Team Kelompok -3 Komunikasi & jaringan IOT 
# Start Created  : 2 Desember 2023
# Last Update    : 7 November 2023
# Revisi         : 0
# Version        : Prototype 1.1
# IoT Platform   : Kodular, MQTT EMQX BROKER, Blynk Iot v2.0
 
# Software Requirement
      - Arduino IDE v1.8.19
      - Kodular 
      - MQTTX 
# Library
      - Blynk
      - Arduino Json
      - LCD i2C 
      - DHT Library 
      - MQTT 
# Hardware Requirement
      - Main Core : Esp-8266  Built-in WiFi
      - Input     : Sensor DHT 11 & Flame Detector Sensor
      - Output    : Relay (High trigger), Buzzer, Led
      - Aksesoris : -
                    
# Catatan : Praktik ini mengembangkan sistem Smart Detection Room yang dapat memudahkan 
            pengguna untuk mendapatkan informasi mengenai kondisi suhu ruangan secara realtime. 
            Selain itu pengguna juga bisa melakukan pengendalian terhadap lampu, kipas angin dan 
            fire hidrant dari jarak jauh melalui jaringan internet. Smart Detection Room menggunakan 
            mikrokontroler ESP-8266 WeMOS D1, Sensor Suhu, Sensor Flame, LED sebagai pengelola dan pengumpul informasi. 
            Aplikasi android digunakan sebagai media untuk menampilkan informasi dan melakukan pengendalian terhadap Smart Detection Room.
            Sistem ini diharapkan bisa membantu masyarakat luas dalam memonitoring dan melindungi ruangan dari bahaya kebakaran
 */

 
#define BLYNK_FIRMWARE_VERSION        "1.0"
#define BLYNK_TEMPLATE_ID "TMPL6zCfWEKjS"
#define BLYNK_DEVICE_NAME "Monitoring Suhu"
#define BLYNK_AUTH_TOKEN "yS5_MCTYrBmInLDn2N3GniOG8CdvG4IQ"
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <MQTT.h>

#define APP_DEBUG
//#define USE_WEMOS_D1_MINI
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "tselhome";  // type your wifi name
char pass[] = "merdeka1";  // type your wifi password

#define DHTPIN D3         // Mention the digital pin where you connected 
#define DHTTYPE DHT11     // DHT 11  
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;
WiFiClient net;     // Object untuk wifi 
MQTTClient client;  // Object untuk MQTT

bool fan_status = false;
bool hydrant_status = false;

const int flamePin = D6;
const int PIN_FAN = D7;
const int PIN_FIRE_HYDRANT = D8;
const int BUZZER_PIN = D5;
const int PIN_1 = D4;
const int PIN_2 = D0;

// Inisialisasi Port Virtual Blynk 
#define BLYNK_CELCIUS        V0
#define BLYNK_MOISTURE       V1
#define BLYNK_FIRE_ALARM     V2
#define BLYNK_FAN            V3
#define BLYNK_HYDRANT        V4


void buzzer(){
  digitalWrite(BUZZER_PIN, HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN, LOW);
  delay(500);
}



void deteksi_sensor(){
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Failed To");
    lcd.setCursor(0, 1);
    lcd.print("  Read Sensor!");
    return;
  }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("T:"+ String(t)+"℃");
    lcd.setCursor(0, 1);
    lcd.print("H:"+ String(h)+"%");
    
 int val = digitalRead(flamePin); // Baca nilai dari sensor IR
  
  if (val == LOW) { // Jika sensor mendeteksi api atau panas
    Serial.println("Api Terdeteksi!");
    buzzer();
    lcd.setCursor(10, 0);
    lcd.print("API");
    lcd.setCursor(10, 1);
    lcd.print("DEKAT!");
  } else {
    digitalWrite(BUZZER_PIN, LOW); // Matikan buzzer
    delay(500);
    Serial.println("NAN !");
    lcd.setCursor(10, 0);
    lcd.print("API");
    lcd.setCursor(10, 1);
    lcd.print("NAN!");
  }
}


void send_sensor_blynk(){

  int val = digitalRead(flamePin); // Baca nilai dari sensor IR
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  Blynk.virtualWrite(BLYNK_MOISTURE, h);  
  Blynk.virtualWrite(BLYNK_CELCIUS, t);
  
  client.publish("Tugas/Kelompok3/IOT/Room/Temp", String(t)+ "°C", false, 1);
  client.publish("Tugas/Kelompok3/IOT/Room/Hum", String(h) + "%", false, 1);


   if(t > 32){
    Blynk.logEvent("temp_alert","Suhu Sudah Melebihi 30 Derajat !");
    client.publish("Tugas/Kelompok3/IOT/Room/Status/Temp", "Suhu > 30 Derajat !");
    }
    else{
    client.publish("Tugas/Kelompok3/IOT/Room/Status/Temp", "Suhu Normal !");
    }

    if(val == LOW){
      Blynk.logEvent("temp_alert2","API TERDETEKSI  !");
      client.publish("Tugas/Kelompok3/IOT/Room/Status/Fire", "API TERDETEKSI!");
      Blynk.virtualWrite(BLYNK_FIRE_ALARM, HIGH);
    }
    else{
      Blynk.virtualWrite(BLYNK_FIRE_ALARM, LOW);
      client.publish("Tugas/Kelompok3/IOT/Room/Status/Fire", "API NAN !");
    }
   }

BLYNK_WRITE(BLYNK_FAN) {
  int state1 = param.asInt();

  if (state1 == 1) {
  digitalWrite(PIN_FAN, HIGH);
  Serial.println("Kipas ON ");
    
  } else {
  digitalWrite(PIN_FAN, LOW);
  Serial.println("Kipas OFF ");
   }
}

BLYNK_WRITE(BLYNK_HYDRANT) {
  int state2 = param.asInt();

  if (state2 == 1) {
  digitalWrite(PIN_FIRE_HYDRANT, HIGH);
  Serial.println("FIRE HYDRANT ON ");
  } else {
  digitalWrite(PIN_FIRE_HYDRANT, LOW);
  Serial.println("FIRE HYDRANT OFF ");
  }
}

void messageReceived(String &topic, String &payload){
  Serial.println("Pesan masuk:" + topic + "-" +payload);

 if(topic == "Tugas/Kelompok3/IOT/Room/Control/Fan"){
    if(payload == "true"){
      digitalWrite(PIN_FAN, HIGH);
    } else {
      digitalWrite(PIN_FAN, LOW);
    }
   }
    
 if(topic == "Tugas/Kelompok3/IOT/Room/Control/Hydrant"){
    if(payload == "true"){
      digitalWrite(PIN_FIRE_HYDRANT, HIGH);
    } else {
      digitalWrite(PIN_FIRE_HYDRANT, LOW);
    }
   }

  if(topic == "Tugas/Kelompok3/IOT/Room/Control/Pin1"){
    if(payload == "true"){
      digitalWrite(PIN_1, HIGH);
    } else {
      digitalWrite(PIN_1, LOW);
    }
   }

  if(topic == "Tugas/Kelompok3/IOT/Room/Control/Pin2"){
    if(payload == "true"){
      digitalWrite(PIN_2, HIGH);
    } else {
      digitalWrite(PIN_2, LOW);
      }
    }
}


void setup(){
 lcd.begin();
  lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("KELOMPOK 3");
    lcd.setCursor(0, 1);
    lcd.print("SMARTROOM DEVICE");
    delay(3000);
    lcd.clear();
 // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(flamePin, INPUT);
  pinMode(PIN_FIRE_HYDRANT, OUTPUT);
  pinMode(PIN_FAN, OUTPUT);
  pinMode(PIN_1, OUTPUT);
  pinMode(PIN_2, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
    
  delay(100);
  lcd.setCursor(0, 0);
  lcd.print("Connecting to...");
  lcd.setCursor(0, 1);
  lcd.print(ssid);
  delay(2000);
  lcd.clear();
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  lcd.setCursor(0, 0);
  lcd.print("Initializing . .");
  lcd.setCursor(0, 1);
  lcd.print(ssid);
  delay(2000);
  lcd.clear();
  }
  lcd.setCursor(0, 0);
  lcd.print("WiFi connected...");
  delay(2000);
 
  Blynk.begin(auth, ssid, pass);
  client.begin("broker.emqx.io", net);
 
  dht.begin();
  while (!client.connect("WEMOS-ESP8266", "public", "public")) {
  delay(500);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Blynk Connected...");
  lcd.setCursor(0, 1);
  lcd.print("MQTT Connected...");
  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Start Device...");
  delay(1000);
  lcd.clear();
  timer.setInterval(2000L, deteksi_sensor);
  timer.setInterval(5000L, send_sensor_blynk);
  client.onMessage(messageReceived);
  client.subscribe("Tugas/Kelompok3/IOT/Room/Control/#");
}

void loop(){
  Blynk.run();
  timer.run();
  client.loop();
}
