#include <Arduino.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <DHT.h>
#include <SD.h>
#include <BH1750.h>
#include <Wire.h>

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

uint8_t espWeb1[] = {0x78, 0x21, 0x84, 0xDD, 0xF8, 0x74}; //{0x78, 0x21, 0x84, 0xDD, 0xF8, 0x74}

const int LS1_Pin = 12;
const int LS2_Pin = 14;

int permin = 4095;
int permax = 3230;
bool fan = 0, motor = 0, pump1 = 0, pump2 = 0, bulb = 0, chenang = 0;
float dhtHum, dhtTemp, lm35Temp, soil;
uint16_t light;

BH1750 lightMeter;
DHT dht(DHTPIN, DHTTYPE);

// Structure example to receive data
// Must match the sender structure
typedef struct received_message {
  int mode;
  int hourMotor; 
  int hourPump; 
  int humThreshold; 
  int lightThreshold; 
  int soilThreshold;
  int tempThreshold; 
  int timeMotor; 
  int timePump; 
  int bulbStatus; 
  int fanStatus; 
  int motorStatus; 
  int pump1Status; 
  int pump2Status;
  int chenang;
} received_message;

typedef struct send_message {
  int dhtHum;
  int dhtTemp; 
  int light; 
  int lm35Temp; 
  int soil; 
  int bulbStatus; 
  int fanStatus; 
  int motorStatus; 
  int pump1Status; 
  int pump2Status;
  int chenang;
} send_message;

received_message  myDataReceived;
send_message      myDataSend;

esp_now_peer_info_t peerInfo;

#define WIFI_SSID         "Free Wifi"
#define WIFI_PASSWORD     "12345679Aa"

int32_t getWiFiChannel(const char *ssid) {
  if (int32_t n = WiFi.scanNetworks()) {
      for (uint8_t i=0; i<n; i++) {
          if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
              return WiFi.channel(i);
          }
      }
  }
  return 0;
}

void Initialization(){
  Serial.begin(115200); 
 }

void fan_Start(){
  digitalWrite(RELAY1, HIGH);
  Serial.println("Fan Start ........");
}

void fan_Stop(){
  digitalWrite(RELAY1, LOW);
  Serial.println("Fan Stop ........");
}

void bulb_Start(){
  digitalWrite(RELAY3, HIGH);
  Serial.println("Bulb Start ........");
}

void bulb_Stop(){
  digitalWrite(RELAY3, LOW);
  Serial.println("Bulb Stop ........");
}

void pump1_Start(){
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  digitalWrite(ENB, HIGH);
  Serial.println("Pump 1 Start ........");
}

void pump1_Stop(){
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  digitalWrite(ENB, LOW);
  Serial.println("Pump 1 Stop ........");
}

void pump2_Start(){
  digitalWrite(RELAY2, HIGH);
  Serial.println("Pump 2 Start ........");
}

void pump2_Stop(){
  digitalWrite(RELAY2, LOW);
  Serial.println("Pump 2 Stop ........");
}

void motor_Dung() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(ENA, LOW);
  Serial.println("Motor Stop ........");
}

void motor_che(int speed) {
  speed = constrain(speed, MIN_SPEED, MAX_SPEED);
  digitalWrite(ENA, HIGH);
  digitalWrite(IN1, HIGH);
  analogWrite(IN2, 255 - speed);
  Serial.println("Motor Start ........");
}

void motor_koche(int speed) {
  speed = constrain(speed, MIN_SPEED, MAX_SPEED);
  digitalWrite(ENA, HIGH);
  digitalWrite(IN1, LOW);
  analogWrite(IN2, speed);
  Serial.println("Motor Start ........");
}

void setup() {
  Initialization();
  dht.begin();
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);

  WiFi.mode(WIFI_STA);

  int32_t channel = getWiFiChannel(WIFI_SSID);

  WiFi.printDiag(Serial); // Uncomment to verify channel number before
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  WiFi.printDiag(Serial); // Uncomment to verify channel change after

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  } 
  
  esp_now_register_recv_cb(OnDataRecv);

  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, espWeb1, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

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
  memcpy(&myDataReceived, incomingData, sizeof(myDataReceived));

  fan = myDataReceived.fanStatus;
  motor = myDataReceived.motorStatus;
  pump1 = myDataReceived.pump1Status;
  pump2 = myDataReceived.pump2Status;
  bulb = myDataReceived.bulbStatus;
  chenang = myDataReceived.chenang;
  
  int ls1Status = digitalRead(LS1_Pin); //=1 rèm đang mở đón nắng
  int ls2Status = digitalRead(LS2_Pin); //=1 rèm đang đóng che nắng

  switch (myDataReceived.mode)
  {
    case 0:
      //quat
      if (fan == 1) {
        fan_Start();
      } else {
        fan_Stop();
      }

      //pump1
      if (pump1 == 1) {
        pump1_Start();
      } else {
        pump1_Stop();
      }

      //pump2
      if (pump2 == 1) {
        pump2_Start();
      } else {
        pump2_Stop();
      }

      //motor
      if (motor == 1 && chenang == 1) { 
        //che nang
        if (ls1Status == 1) {
          motor_che(120);
          if (ls2Status == 1) {
            motor_Dung();
          }
        } else if (ls2Status == 1) {
          motor_Dung();
        }
      } else if (motorStatus == 1 && chenang == 0) {
        //koche
        if (ls2Status == 1) {
          motor_koche(120);
          if (ls1Status == 1) {
            motor_Dung();
          }
        } else if (ls1Status == 1) {
          motor_Dung();
        }
      } else {
        motor_Dung();
      }   
      break;
    case 1:
      //Nhiet do: Den suoi + Bom phun suong
      if (dhtTemp >= myDataReceived.tempThreshold - 2 && dhtTemp <= myDataReceived.tempThreshold + 2) {
        pump1_Stop();
        bulb_Stop();
        pump1 = 0;
        bulb = 0;
      } else if (dhtTemp < myDataReceived.tempThreshold - 2) {
        pump1_Stop();
        bulb_Start();
        pump1 = 0;
        bulb = 1;
      } else {
        pump1_Start();
        bulb_Stop();
        pump1 = 1;
        bulb = 0;
      }

      // Do am: Quạt + bơm phun sương
      if (dhtHum >= myDataReceived.humThreshold - 2 && dhtHum <= myDataReceived.humThreshold + 2) {
        pump1_Stop();
        fan_Stop();
        pump1 = 0;
        fan = 0;
      } else if (dhtHum < myDataReceived.humThreshold - 2) {
        pump1_Start();
        fan_Stop();
        pump1 = 1;
        fan = 0;
      } else {
        pump1_Stop();
        fan_Start();
        pump1 = 0;
        fan = 1;
      }

      

      myDataSend.dhtHum = dhtHum;
      myDataSend.dhtTemp = dhtTemp;
      myDataSend.lm35Temp = lm35Temp;
      myDataSend.soil = soil;
      myDataSend.light = light;
        
      myDataSend.fanStatus = fan;
      myDataSend.motorStatus = motor;
      myDataSend.pump1Status = pump1;
      myDataSend.pump2Status = pump2;
      myDataSend.bulbStatus = bulb;
      myDataSend.chenang = chenang;
      
      // Send message via ESP-NOW
      esp_err_t result1 = esp_now_send(espWeb1, (uint8_t *) &myDataSend, sizeof(myDataSend));
        
      if (result1 == ESP_OK) {
        Serial.println("Sent with success");
      }
      else {
        Serial.println("Error sending the data");
      }
      break;
  }
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:     ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  char macStr[24];
  snprintf(macStr, sizeof(macStr), " %02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("\nData Sent to: "); Serial.println(macStr);
  Serial.print("WiFi channel: ");
  Serial.println(WiFi.channel());
}

void loop() 
{
  /* DHT11 */
  float dhtHum = dht.readHumidity();
  float dhtTemp = dht.readTemperature();
  /* LM35 */
  float lm35Temp = (5.0*analogRead(PIN_LM35)*100.0/4096.0);
  /* Soil soil */
  int analogSoil = analogRead(PIN_SOIL);
  if (analogSoil < permax) {
    analogSoil = permax;
  }
  if (analogSoil > permin) {
    analogSoil = permin;
  }
  float soil = abs(float(float(permin - analogSoil) / float(permax - permin)) * 100);
  /* BH1750 */
  uint16_t light = lightMeter.readLightLevel();
  
  // Set values to send

  myDataSend.dhtHum = dhtHum;
  myDataSend.dhtTemp = dhtTemp;
  myDataSend.lm35Temp = lm35Temp;
  myDataSend.soil = soil;
  myDataSend.light = light;

  Serial.println(dhtHum);
  Serial.println(dhtTemp);
  Serial.println(lm35Temp);
  Serial.println(soil);
  Serial.println(light);

  myDataSend.fanStatus = fan;
  myDataSend.motorStatus = motor;
  myDataSend.pump1Status = pump1;
  myDataSend.pump2Status = pump2;
  myDataSend.bulbStatus = bulb;
  myDataSend.chenang = chenang;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(espWeb1, (uint8_t *) &myDataSend, sizeof(myDataSend));
  
  Serial.print("result: ");
  Serial.println(result);
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }

  delay(5000);
}

