#include <Wire.h>
#include "SD.h"
#include <WiFi.h>
#include <FirebaseESP32.h>

#define ENA 4
#define IN1 18
#define IN2 5 
#define ENB 15
#define IN3 0
#define IN4 2
#define RELAY1 13
#define RELAY2 27
#define MAX_SPEED 255
#define MIN_SPEED 0
#define DATABASE_URL "https://doan2-4397c-default-rtdb.firebaseio.com/" 
#define FIREBASE_HOST "doan2-4397c-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "75gIhC6SyoyuT5AD0tguoW9KJWciiXQDpuHDOVFv"
#define WIFI_SSID "Internet Free"
#define WIFI_PASSWORD "12345679Aa"

FirebaseData statusMotor;
FirebaseData setStatusMotor;

const int LS1_Pin = 12;
const int LS2_Pin = 14;

int resultStatusMotor = 0;

void setup() {
  Serial.begin(115200);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(RELAY1, OUTPUT);
  pinMode(LS1_Pin, INPUT_PULLUP);  //1 khi cham vao + NC
  pinMode(LS2_Pin, INPUT_PULLUP);  //1 khi cham vao + NC
  WiFiConnection();
  Firebase.setStreamCallback(statusMotor, streamCallback, streamTimeoutCallback);
  if (!Firebase.beginStream(statusMotor, "/motorStatus"))
  {
    // Could not begin stream connection, then print out the error detail
    Serial.println(statusMotor.errorReason());
  }
}

void streamCallback(StreamData data)
{

  // Print out all information

  Serial.println("Stream Data...");
  Serial.println(data.streamPath());
  Serial.println(data.dataPath());
  Serial.println(data.dataType());

  // Print out the value
  // Stream data can be many types which can be determined from function dataType

  if (data.dataTypeEnum() == fb_esp_rtdb_data_type_integer)
      Serial.println(data.to<int>());
  else if (data.dataTypeEnum() == fb_esp_rtdb_data_type_float)
      Serial.println(data.to<float>(), 5);
  else if (data.dataTypeEnum() == fb_esp_rtdb_data_type_double)
      printf("%.9lf\n", data.to<double>());
  else if (data.dataTypeEnum() == fb_esp_rtdb_data_type_boolean)
      Serial.println(data.to<bool>()? "true" : "false");
  else if (data.dataTypeEnum() == fb_esp_rtdb_data_type_string)
      Serial.println(data.to<String>());
  else if (data.dataTypeEnum() == fb_esp_rtdb_data_type_json)
  {
      FirebaseJson *json = data.to<FirebaseJson *>();
      Serial.println(json->raw());
  }
  else if (data.dataTypeEnum() == fb_esp_rtdb_data_type_array)
  {
      FirebaseJsonArray *arr = data.to<FirebaseJsonArray *>();
      Serial.println(arr->raw());
  }

  resultStatusMotor = data.to<int>();

}

void streamTimeoutCallback(bool timeout)
{
  if(timeout){
    // Stream timeout occurred
    Serial.println("Stream timeout, resume streaming...");
  }  
}

void WiFiConnection(){
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
}

void fan_Start(){
  digitalWrite(RELAY1, HIGH);
}

void fan_Stop(){
  digitalWrite(RELAY1, LOW);
}

void pump1_Start(){
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  digitalWrite(ENB, HIGH);
}

void pump1_Stop(){
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  digitalWrite(ENB, LOW);
}

void motor1_Dung() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(ENA, LOW);
}

void motor1_Dong(int speed) {
 speed = constrain(speed, MIN_SPEED, MAX_SPEED);
 digitalWrite(ENA, HIGH);
 digitalWrite(IN1, HIGH);
 analogWrite(IN2, 255 - speed);
}

void motor1_Mo(int speed) {
  speed = constrain(speed, MIN_SPEED, MAX_SPEED);
  digitalWrite(ENA, HIGH);
  digitalWrite(IN1, LOW);
  analogWrite(IN2, speed);
}

void loop() {
  int ls1Status = digitalRead(LS1_Pin); //=1 rèm đang mở đón nắng
  int ls2Status = digitalRead(LS2_Pin); //=1 rèm đang đóng che nắng
  if (resultStatusMotor == 0 && ls1Status == 1) {
    motor1_Dong(120);
  }
  else if (ls1Status == 1) {
    motor1_Dung();
    Firebase.setFloat(setStatusMotor, "/isOpenOrClose", 1);
  }
  if (resultStatusMotor == 1 && ls2Status == 1) {
    motor1_Mo(120);
  } else if (ls2Status == 1) {
    motor1_Dung();
    Firebase.setFloat(setStatusMotor, "/isOpenOrClose", 0);
  }
  delay(100);
}
