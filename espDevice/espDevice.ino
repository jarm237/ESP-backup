#include <WiFi.h>
#include <DHT.h>
#include <SD.h>
#include <esp_now.h>

#define ENA 4
#define IN1 18
#define IN2 5 
#define ENB 15
#define IN3 0
#define IN4 2
#define RELAY1 13
#define RELAY2 27
#define RELAY3 25
#define MAX_SPEED 255
#define MIN_SPEED 0

const int LS1_Pin = 12;
const int LS2_Pin = 14;

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
    int id;
    int fanStatus;
    int motorStatus;
    int pump1Status;
    int pump2Status;
    int bulbStatus;
    int chenang;
} struct_message;

// Create a struct_message called receiveData
struct_message receiveData;


void Initialization(){
  Serial.begin(115200); 
 }

void fan_Start(){
  digitalWrite(RELAY1, HIGH);
}

void fan_Stop(){
  digitalWrite(RELAY1, LOW);
}

void bulb_Start(){
  digitalWrite(RELAY3, HIGH);
}

void bulb_Stop(){
  digitalWrite(RELAY3, LOW);
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

void pump2_Start(){
  digitalWrite(RELAY2, HIGH);
}

void pump2_Stop(){
  digitalWrite(RELAY2, LOW);
}

void motor_Dung() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(ENA, LOW);
}

void motor_che(int speed) {
  speed = constrain(speed, MIN_SPEED, MAX_SPEED);
  digitalWrite(ENA, HIGH);
  digitalWrite(IN1, HIGH);
  analogWrite(IN2, 255 - speed);
}

void motor_koche(int speed) {
  speed = constrain(speed, MIN_SPEED, MAX_SPEED);
  digitalWrite(ENA, HIGH);
  digitalWrite(IN1, LOW);
  analogWrite(IN2, speed);
}

void setup() {
  Initialization();

  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  } 
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  pinMode(LS1_Pin, INPUT_PULLUP);  //1 khi cham vao + NC
  pinMode(LS2_Pin, INPUT_PULLUP);  //1 khi cham vao + NC
}

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&receiveData, incomingData, sizeof(receiveData));
  int ls1Status = digitalRead(LS1_Pin); //=1 rèm đang mở đón nắng
  int ls2Status = digitalRead(LS2_Pin); //=1 rèm đang đóng che nắng
  Serial.print("id: ");
  Serial.println(receiveData.id); 
  Serial.print("Fan: ");
  Serial.println(receiveData.fanStatus); 
  Serial.print("Pump1: ");
  Serial.println(receiveData.pump1Status);   
  Serial.print("Pump2: ");
  Serial.println(receiveData.pump2Status);  
  Serial.print("Motor: ");
  Serial.println(receiveData.motorStatus);   
  Serial.print("Led: ");
  Serial.println(receiveData.bulbStatus);
  Serial.print("chenang: ");
  Serial.println(receiveData.chenang); 
  if(receiveData.id == 1)
  {
    if(receiveData.fanStatus == 1)
    {
      fan_Start();
    }
    else
    {
      fan_Stop();
    }

    if(receiveData.pump1Status == 1)
    {
      pump1_Start();
    }
    else
    {
      pump1_Stop();
    }

    if(receiveData.pump2Status == 1)
    {
      pump2_Start();
    }
    else
    {
      pump2_Stop();
    }

    if(receiveData.bulbStatus == 1)
    {
      bulb_Start();
    }
    else
    {
      bulb_Stop();
    }

    if(receiveData.motorStatus == 1 && receiveData.chenang == 1)
    { 
      //che nang
      if(ls1Status == 1)
      {
        motor_che(120);
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
    else if(receiveData.motorStatus == 1 && receiveData.chenang == 0)
    {
      //koche
      if(ls2Status == 1)
      {
        motor_koche(120);
        if(ls1Status == 1)
        {
          motor_Dung();
        }
      }
      else if(ls1Status == 1)
      {
        motor_Dung();
      }
    }
    else
    {
      motor_Dung();
    }
  }


}

void loop() 
{

 
}

