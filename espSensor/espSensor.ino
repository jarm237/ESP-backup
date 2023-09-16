#include <WiFi.h>
#include <DHT.h>
#include <Wire.h>
#include <BH1750.h>
#include <SD.h>
#include <esp_now.h>

#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
#define DHTPIN 19 
#define PIN_LM35 36
#define PIN_SOIL 34         
#define DHTTYPE DHT11 

//dia chi esp nhan
uint8_t webAddress1[] = {0x78, 0x21, 0x84, 0xDD, 0xF8, 0x74};
uint8_t webAddress2[] = {0x9C, 0x9C, 0x1F, 0xEA, 0xF4, 0x60};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message1 {
  int id;
  float humidity;
  float tempDHT11;
  float tempLM35;
  float moisture;
  uint16_t lux;
} struct_message1;

typedef struct struct_message2 {
  int id;
  int fanStatus;
  int motorStatus;
  int pump1Status;
  int pump2Status;
  int bulbStatus;
  int chenang;
} struct_message2;

// Create a struct_message called sensorData
struct_message1 sensorData;
struct_message2 statusData;

esp_now_peer_info_t peer_sensor_Info;

esp_now_peer_info_t peer_status_Info;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

int permin = 4095;
int permax = 3230;

BH1750 lightMeter;
DHT dht(DHTPIN, DHTTYPE);

// int tempThreshold = 0, humThreshold = 0, lightThreshold = 0, soilThreshold = 0;

void Initialization(){
  Serial.begin(115200); 
 }

void setup() {
  Initialization();
  dht.begin();
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_AP_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peer_sensor_Info.peer_addr, webAddress1, 6);
  peer_sensor_Info.channel = 0;  
  peer_sensor_Info.encrypt = false;

  if (esp_now_add_peer(&peer_sensor_Info) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  memcpy(peer_status_Info.peer_addr, webAddress2, 6);
  peer_status_Info.channel = 0;  
  peer_status_Info.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peer_status_Info) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

}

void loop() {
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
  sensorData.id = 1;
  sensorData.humidity = humidity;
  sensorData.tempDHT11 = tempDHT11;
  sensorData.tempLM35 = tempLM35;
  sensorData.moisture = moisture;
  sensorData.lux = lux;

  // Send message via ESP-NOW
  esp_err_t result1 = esp_now_send(webAddress1, (uint8_t *) &sensorData, sizeof(sensorData));
  

  Serial.print("Result1: ");
  Serial.println(result1);
  if (result1 == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }

  // Set values2 to send
  statusData.id = 1;
  statusData.fanStatus = 1;
  statusData.pump1Status = 1;
  statusData.pump2Status = 1;
  statusData.bulbStatus = 1;
  statusData.motorStatus = 1;
  statusData.chenang = 1;

  // Send message via ESP-NOW
  esp_err_t result2 = esp_now_send(webAddress2, (uint8_t *) &statusData, sizeof(statusData));

  Serial.print("Result2: ");
  Serial.println(result2); 
  if (result2 == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  Serial.print("id: ");
  Serial.println(statusData.id); 
  Serial.print("Fan: ");
  Serial.println(statusData.fanStatus); 
  Serial.print("Pump1: ");
  Serial.println(statusData.pump1Status);   
  Serial.print("Pump2: ");
  Serial.println(statusData.pump2Status);  
  Serial.print("Motor: ");
  Serial.println(statusData.motorStatus);   
  Serial.print("Led: ");
  Serial.println(statusData.bulbStatus);
  Serial.print("chenang: ");
  Serial.println(statusData.chenang); 
  // Serial.print("id: ");
  Serial.println(sensorData.id); 
  Serial.print("Temperature LM35: ");
  Serial.println(sensorData.tempLM35); 
  Serial.print("Temperature DHT11: ");
  Serial.println(sensorData.tempDHT11);   
  Serial.print("Humidity DHT11: ");
  Serial.println(sensorData.humidity);  
  Serial.print("Light BH1750: ");
  Serial.println(sensorData.lux);   
  Serial.print("Soil Moisture: ");
  Serial.println(sensorData.moisture);

  delay(3000);
}