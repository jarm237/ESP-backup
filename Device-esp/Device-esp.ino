#include <WiFi.h>
#include <FirebaseESP32.h>
#include <DHT.h>
#include <SD.h>

#define FIREBASE_HOST "https://sensor-7aff3-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "LhCZh1Jgb3scUbExVeFzj3HEkb7coL0cOhrlnk8K"

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

const int LS1_Pin = 12;
const int LS2_Pin = 14;

const char* SSID = "Internet Free";
const char* PASSWORD = "12345679Aa";

// Khởi tạo đối tượng FirebaseESP32
FirebaseData firebaseData;
FirebaseJson firebaseJson;
int fanStatus = 0;
int motorStatus = 0;
int pump1Status = 0;
int pump2Status = 0;
int mode = 0;
int tempThreshold = 0, humThreshold = 0, lightThreshold = 0, soilThreshold = 0;
int humidity = 0, tempDHT11 = 0, tempLM35 = 0, lux = 0, moisture = 0;
int openSystem = -1;
int t1 = -1, t2 = 0;
void setup() {
  Initialization();
  WiFiConnection();
  WiFi.begin(SSID, PASSWORD);
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(LS1_Pin, INPUT_PULLUP);  //1 khi cham vao + NC
  pinMode(LS2_Pin, INPUT_PULLUP);  //1 khi cham vao + NC
}

void Initialization(){
  Serial.begin(115200); 
 }

void WiFiConnection(){
  WiFi.begin(SSID, PASSWORD);
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
  fanStatus = 1;
  digitalWrite(RELAY1, HIGH);
  if(mode == 1){
    Firebase.setInt(firebaseData,"/status/fanStatus", fanStatus);
  }
}

void fan_Stop(){
  fanStatus = 0;
  digitalWrite(RELAY1, LOW);
  if(mode == 1){
    Firebase.setInt(firebaseData,"/status/fanStatus", fanStatus);
  }
}

void pump1_Start(){
  pump1Status = 1;
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  digitalWrite(ENB, HIGH);
  if(mode == 1){
    Firebase.setInt(firebaseData,"/status/pump1Status", pump1Status);
  }
}

void pump1_Stop(){
  pump1Status = 0;
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  digitalWrite(ENB, LOW);
  if(mode == 1){
    Firebase.setInt(firebaseData,"/status/pump1Status", pump1Status);
  }
}

void pump2_Start(){
  pump2Status = 1;
  digitalWrite(RELAY2, HIGH);
  if(mode == 1){
    Firebase.setInt(firebaseData,"/status/pump2Status", pump2Status);
  }
}

void pump2_Stop(){
  pump2Status = 0;
  digitalWrite(RELAY2, LOW);
  if(mode == 1){
    Firebase.setInt(firebaseData,"/status/pump2Status", pump2Status);
  }
}

void motor_Dung() {
  motorStatus = 0;
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(ENA, LOW);
  if(mode == 1) {
   Firebase.setInt(firebaseData,"/status/motorStatus", motorStatus); 
  } 
}

void motor_Dong(int speed) {
  motorStatus = 1;
  speed = constrain(speed, MIN_SPEED, MAX_SPEED);
  digitalWrite(ENA, HIGH);
  digitalWrite(IN1, HIGH);
  analogWrite(IN2, 255 - speed);
  if(mode == 1){
    Firebase.setInt(firebaseData,"/status/motorStatus", motorStatus);
  }
}

void motor_Mo(int speed) {
  motorStatus = 1;
  speed = constrain(speed, MIN_SPEED, MAX_SPEED);
  digitalWrite(ENA, HIGH);
  digitalWrite(IN1, LOW);
  analogWrite(IN2, speed);
  if(mode == 1){
    Firebase.setInt(firebaseData,"/status/motorStatus", motorStatus);
  }
}

void loop() {

  int ls1Status = digitalRead(LS1_Pin); //=1 rèm đang mở đón nắng
  int ls2Status = digitalRead(LS2_Pin); //=1 rèm đang đóng che nắng

  //gui du lieu len firebase
  Firebase.setFloat(firebaseData, "/system/openSwitch", ls1Status);
  Firebase.setFloat(firebaseData, "/system/closeSwitch", ls2Status);

  //doc du lieu tu firebase gui ve
  if (Firebase.getInt(firebaseData, "/status/fanStatus")) {
      fanStatus = firebaseData.intData();
    }
  
  if (Firebase.getInt(firebaseData, "/status/motorStatus")) {
      motorStatus = firebaseData.intData();
    }

  if (Firebase.getInt(firebaseData, "/status/pump1Status")) {
      pump1Status = firebaseData.intData();
    }
  
  if (Firebase.getInt(firebaseData, "/status/pump2Status")) {
      pump2Status = firebaseData.intData();
    }
  
  if (Firebase.getInt(firebaseData, "/mode")) {
      mode = firebaseData.intData();
    }

  if (Firebase.getInt(firebaseData, "/settingThreshold/tempThreshold")) {
      tempThreshold = firebaseData.intData();
    }

  if (Firebase.getInt(firebaseData, "/settingThreshold/humThreshold")) {
      humThreshold = firebaseData.intData();
    }

  if (Firebase.getInt(firebaseData, "/settingThreshold/lightThreshold")) {
      lightThreshold = firebaseData.intData();
    }

  if (Firebase.getInt(firebaseData, "/settingThreshold/soilThreshold")) {
      soilThreshold = firebaseData.intData();
    }

  if (Firebase.getInt(firebaseData, "/sensor/dhtHum")) {
      humidity = firebaseData.intData();
    }

  if (Firebase.getInt(firebaseData, "/sensor/dhtTemp")) {
      tempDHT11 = firebaseData.intData();
    }

  if (Firebase.getFloat(firebaseData, "/sensor/lm35Temp")) {
      tempLM35 = firebaseData.floatData();
    }

  if (Firebase.getInt(firebaseData, "/sensor/light")) {
      lux = firebaseData.intData();
    }
  
  if (Firebase.getInt(firebaseData, "/sensor/soil")) {
      moisture = firebaseData.intData();
    }
  
  // Serial.print("Temp: ");
  // Serial.print(tempThreshold);
  // Serial.print("\t");
  // Serial.print("Hum: ");
  // Serial.println(humThreshold);
  // Serial.print("Light: ");
  // Serial.print(lightThreshold);  
  // Serial.print("\t");
  // Serial.print("Soil: ");
  // Serial.println(soilThreshold);
  // Serial.print("DHT11: ");
  // Serial.print(tempDHT11);
  // Serial.print("\t");
  // Serial.print("HumDHT: ");
  // Serial.println(humidity);
  // Serial.print("BH1750: ");
  // Serial.print(lux);  
  // Serial.print("\t");
  // Serial.print("Soil MOI: ");
  // Serial.println(moisture);
  // Serial.println(mode);
  switch(mode)
  {
    case 1:
      //nhiet do
      if(tempDHT11 == tempThreshold){
         if(humidity < humThreshold)
         {
           pump1_Start();
         }
         else{
           pump1_Stop();
         }
      }
      else if (tempDHT11 > tempThreshold)
      {
        if(humidity > humThreshold)
        {
          pump1_Stop();
        }
        else {
          pump1_Start();
        }
      }
      else {

        if(humidity < humThreshold){
          pump1_Start();
        }
        else {
          pump1_Stop();
        }

        if (lux < lightThreshold) {
          // don anh nang
          if(ls1Status == 1)
          {
            motor_Dung();
          }
          else if(ls2Status == 1) {
            motor_Mo(120);
            if(ls1Status == 1)
            {
              motor_Dung();
            }
          }
        }
        else {
          //che nang
          if(ls1Status == 1)
          {
            motor_Dong(120);
            if(ls2Status == 1)
            {
              motor_Dung();
            }
          }
          else if(ls2Status == 1)
          {
            motor_Dung();
          }
        }
      }
    // do am
      if(humidity == humThreshold){
        fan_Stop();
        if(tempDHT11 > tempThreshold)
        {
          pump1_Start();
        }
        else{
          pump1_Stop();
        }
      }
      else if(humidity > humThreshold)
      {
        fan_Start();
        pump1_Stop();
      }
      else{
        fan_Stop();
        if (tempDHT11 > tempThreshold) {
          pump1_Start();
        }
      }
      //do am dat
      if(moisture == soilThreshold)
      {
        pump2_Stop();
      }
      else if(moisture > soilThreshold)
      {
        pump2_Stop();
      }
      else {
        pump2_Start();
      }
      // anh sang
      if (lux == lightThreshold)
      {
        
      }
      else if(lux > lightThreshold)
      {
        //che nang
        if(ls1Status == 1)
        {
          motor_Dong(120);
          if(ls2Status == 1)
          {
            motor_Dung();
          }
        }
        else if(ls2Status == 1)
        {
          motor_Dung();
        }
      }

      else{
        if(tempDHT11 < tempThreshold) 
        {
          //don anh nang
          if(ls1Status == 1)
          {
            motor_Dung();
          }
          if(ls2Status == 1)
          {
            motor_Mo(120);
            if(ls1Status == 1)
            {
              motor_Dung();
            }
          }
        }
        
        else{
          //che nang
          if(ls1Status == 1)
          {
            motor_Dong(120);
            if(ls2Status == 1)
            {
              motor_Dung();
            }
          }
          else if(ls2Status == 1)
          {
            motor_Dung();
          }
        }
      }

    default:
      if(pump1Status == 1 ){
        pump1_Start();
        
      }
      else {
        pump1_Stop();
      }

      if(pump2Status == 1 ){
        pump2_Start();
      }
      else {
        pump2_Stop();
      }

      if (fanStatus == 1){
        fan_Start();
      }
      else {
        fan_Stop();
      }

      if (motorStatus == 1)
      {

        if(ls1Status == 1 && ls2Status == 0){
          openSystem = 1;
        }
        else if(ls1Status == 0 && ls2Status == 1){
          openSystem = 0;
        }

        //che nang
        if((openSystem == 1 || t1 == 1) && t2 == 0)
        {
          motor_Dong(120);
          t1 = 1;
          if(ls2Status == 1)
          {
            motor_Dung();
            t1 = 0;
            Firebase.setInt(firebaseData,"/status/motorStatus", motorStatus);
          }
        }
        else if(openSystem == 0 || t2 == 1)
        {
          motor_Mo(120);
          t2 = 1;
          if(ls1Status == 1)
          {
            motor_Dung();
            t2 = 0;         
            Firebase.setInt(firebaseData,"/status/motorStatus", motorStatus); 
          }
        }
      }
      else{
        motor_Dung();
      }
  }
}

