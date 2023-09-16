#include <WiFi.h>
#include <DHT.h>
#include <SD.h>
#include <esp_now.h>
#include <BH1750.h>
#include <Wire.h>
#include <DHT.h>

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

#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
#define DHTPIN 19 
#define PIN_LM35 36
#define PIN_SOIL 34         
#define DHTTYPE DHT11 

//dia chi esp nhan
uint8_t webAddress[] = {0x78, 0x21, 0x84, 0xDD, 0xF8, 0x74};

const int LS1_Pin = 12;
const int LS2_Pin = 14;

int permin = 4095;
int permax = 3230;
int fan = 0, motor = 0, pump1 = 0, pump2 = 0, bulb = 0;
float humidity,tempDHT11, tempLM35, moisture;
uint16_t lux;

BH1750 lightMeter;
DHT dht(DHTPIN, DHTTYPE);

typedef struct struct_message1 {
  float humidity;
  float tempDHT11;
  float tempLM35;
  float moisture;
  uint16_t lux;
  int fanStatus;
  int motorStatus;
  int pump1Status;
  int pump2Status;
  int bulbStatus;
  int chenang;
} struct_message1;

typedef struct struct_message2 {
  int fanStatus;
  int motorStatus;
  int pump1Status;
  int pump2Status;
  int bulbStatus;
  int chenang;
  int mode;
  int humThreshold;
  int tempThreshold;
  int soilThreshold;
  int lightThreshold;
} struct_message2;

// Create a struct_message called sendData
struct_message1 sendData;
// Create a struct_message called receiveData
struct_message2 receiveData;

esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status);
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

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
  dht.begin();
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);

  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  } 
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
    // Register peer
  memcpy(peerInfo.peer_addr, webAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
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

  fan = receiveData.fanStatus;
  motor = receiveData.motorStatus;
  pump1 = receiveData.pump1Status;
  pump2 = receiveData.pump2Status;
  bulb = receiveData.bulbStatus;
  
  int ls1Status = digitalRead(LS1_Pin); //=1 rèm đang mở đón nắng
  int ls2Status = digitalRead(LS2_Pin); //=1 rèm đang đóng che nắng

  switch(receiveData.mode)
  {
    case 0:
      //quat
      if(fan == 1)
      {
        fan_Start();
      }
      else
      {
        fan_Stop();
      }

      //pump1
      if(pump1 == 1)
      {
        pump1_Start();
      }
      else
      {
        pump1_Stop();
      }

      //pump2
      if(pump2 == 1)
      {
        pump2_Start();
      }
      else
      {
        pump2_Stop();
      }

      //motor
      if(motor == 1 && receiveData.chenang == 1)
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
    
      break;
    case 1:
      //Nhiet do: Den suoi + Bom phun suong
      if(tempDHT11 >= receiveData.tempThreshold - 2 && tempDHT11 <= receiveData.tempThreshold + 2)
      {
        pump1_Stop();
        bulb_Stop();
        pump1 = 0;
        bulb = 0;
      }
      else if(tempDHT11 < receiveData.tempThreshold - 2)
      {
        pump1_Stop();
        bulb_Start();
        pump1 = 0;
        bulb = 1;
      }
      else
      {
        pump1_Start();
        bulb_Stop();
        pump1 = 1;
        bulb = 0;
      }

      // Do am: Quạt + bơm phun sương
      if(humidity >= receiveData.humThreshold - 2 && humidity <= receiveData.humThreshold + 2)
      {
        pump1_Stop();
        fan_Stop();
        pump1 = 0;
        fan = 0;
      }
      else if(humidity < receiveData.humThreshold - 2)
      {
        pump1_Start();
        fan_Stop();
        pump1 = 1;
        fan = 0;
      }
      else
      {
        pump1_Stop();
        fan_Start();
        pump1 = 0;
        fan = 1;
      }
      sendData.humidity = humidity;
      sendData.tempDHT11 = tempDHT11;
      sendData.tempLM35 = tempLM35;
      sendData.moisture = moisture;
      sendData.lux = lux;
        
      sendData.fanStatus = fan;
      sendData.motorStatus = motor;
      sendData.pump1Status = pump1;
      sendData.pump2Status = pump2;
      sendData.bulbStatus = bulb;
      
      // Send message via ESP-NOW
      esp_err_t result1 = esp_now_send(webAddress, (uint8_t *) &sendData, sizeof(sendData));
        
      if (result1 == ESP_OK) {
        Serial.println("Sent with success");
      }
      else {
        Serial.println("Error sending the data");
      }

      break;
  }



}

void loop() 
{
  /* DHT11 */
  float humidity = dht.readHumidity();
  float tempDHT11 = dht.readTemperature();
  /* LM35 */
  float tempLM35 = (5.0*analogRead(PIN_LM35)*100.0/4096.0);
  /* Soil Moisture */
  int analogSoil = analogRead(PIN_SOIL);
  if (analogSoil < permax)
  {
    analogSoil = permax;
  }
    if (analogSoil > permin)
  {
    analogSoil = permin;
  }
  float moisture = abs(float(float(permin - analogSoil) / float(permax - permin)) * 100);
  /* BH1750 */
  uint16_t lux = lightMeter.readLightLevel();
  
  // Set values to send

  sendData.humidity = humidity;
  sendData.tempDHT11 = tempDHT11;
  sendData.tempLM35 = tempLM35;
  sendData.moisture = moisture;
  sendData.lux = lux;

  sendData.fanStatus = fan;
  sendData.motorStatus = motor;
  sendData.pump1Status = pump1;
  sendData.pump2Status = pump2;
  sendData.bulbStatus = bulb;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(webAddress, (uint8_t *) &sendData, sizeof(sendData));
  
  Serial.print("result: ");
  Serial.println(result);
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }

  delay(3000);
}

