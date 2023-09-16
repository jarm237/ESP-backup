#include <Arduino.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>

// REPLACE WITH YOUR RECEIVER MAC Address
// uint8_t broadcastAddress1[] = {0x78, 0x21, 0x84, 0xDD, 0xF8, 0x74}; //{0x78, 0x21, 0x84, 0xDD, 0xF8, 0x74}
uint8_t broadcastAddress[] = {0x9C, 0x9C, 0x1F, 0xEA, 0xF4, 0x60}; //{0x9C, 0x9C, 0x1F, 0xEA, 0xF4, 0x60}

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
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
} struct_message;

struct_message myData;

esp_now_peer_info_t peerInfo;

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

// callback when data is sent
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
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
 
  // Set device as a Wi-Fi Station
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

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

int bulb = 0;
int fan = 0;
int motor = 0;
int pump1 = 0;
int pump2 = 0;
 
void loop() {
  // Set values to send
  myData.dhtHum = (int) random(50 ,100);
  myData.dhtTemp = (int) random(25 ,40);
  myData.light = (int) random(2500 ,3000);
  myData.lm35Temp = (int) random(25,35);
  myData.soil = (int) random(80, 90);

  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');

    if (command == "00") {
      bulb = 0;
      Serial.print("Bulb serial write: ");
      Serial.println(bulb);
    }
    else if (command == "01") {
      bulb = 1;
      Serial.print("Bulb serial write: ");
      Serial.println(bulb);
    }
    else if (command == "10") {
      fan = 0;
      Serial.print("Fan serial write: ");
      Serial.println(fan);
    }
    else if (command == "11") {
      fan = 1;
      Serial.print("Fan serial write: ");
      Serial.println(fan);
    }
    else if (command == "20") {
      motor = 0;
      Serial.print("Motor serial write: ");
      Serial.println(motor);
    }
    else if (command == "21") {
      motor = 1;
      Serial.print("Motor serial write: ");
      Serial.println(motor);
    }
    else if (command == "30") {
      pump1 = 0;
      Serial.print("Pump1 serial write: ");
      Serial.println(pump1);
    }
    else if (command == "31") {
      pump1 = 1;
      Serial.print("Pump1 serial write: ");
      Serial.println(pump1);
    }
    else if (command == "40") {
      pump2 = 0;
      Serial.print("Pump2 serial write: ");
      Serial.println(pump2);
    }
    else if (command == "41") {
      pump2 = 1;
      Serial.print("Pump2 serial write: ");
      Serial.println(pump2);
    }
  }

  myData.bulbStatus = bulb;
  myData.fanStatus = fan;
  myData.motorStatus = motor;
  myData.pump1Status = pump1;
  myData.pump2Status = pump2;
  
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  
  Serial.print("Result: ");
  Serial.println(result);
  Serial.print("Size: ");
  Serial.println(sizeof(myData));
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  delay(5000);
}