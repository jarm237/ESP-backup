#include <Arduino.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
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
} struct_message;

struct_message myData;

/* 1. Define the WiFi credentials */
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

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  Serial.println("Received Data.................");
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(len);

  Serial.print("mode: ");
  Serial.println(myData.mode);
  Serial.print("bulbStatus: ");
  Serial.println(myData.bulbStatus);
  Serial.print("fanStatus: ");
  Serial.println(myData.fanStatus);
  Serial.print("motorStatus: ");
  Serial.println(myData.motorStatus);
  Serial.print("pump1Status: ");
  Serial.println(myData.pump1Status);
  Serial.print("pump2Status: ");
  Serial.println(myData.pump2Status);

  Serial.print("WiFi channel: ");
  Serial.println(WiFi.channel());
  
  Serial.println();
}
 
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  
  // Set device as a Wi-Fi Station
  // WiFi.mode(WIFI_AP_STA);
  WiFi.mode(WIFI_STA);

  int32_t channel = getWiFiChannel(WIFI_SSID);

  WiFi.printDiag(Serial); // Uncomment to verify channel number before
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  WiFi.printDiag(Serial); // Uncomment to verify channel change after

  // WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  // Serial.print("Connecting to Wi-Fi");
  // while (WiFi.status() != WL_CONNECTED)
  // {
  //   Serial.println(WiFi.status());
  //   delay(500);
  // }
  // Serial.println();
  Serial.print("WiFi channel: ");
  Serial.println(WiFi.channel());

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  Serial.print("Mac Address: ");
  Serial.println(WiFi.macAddress());
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {

}