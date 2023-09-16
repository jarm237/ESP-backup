#include <esp_now.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <SD.h>

// #define FIREBASE_HOST "https://sensor-7aff3-default-rtdb.firebaseio.com/"
// #define FIREBASE_AUTH "LhCZh1Jgb3scUbExVeFzj3HEkb7coL0cOhrlnk8K"


// const char* SSID = "Note8";
// const char* PASSWORD = "phat0211";

// Khởi tạo đối tượng FirebaseESP32
FirebaseData firebaseData;
FirebaseJson firebaseJson;

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
    int id;
    float humidity;
    float tempDHT11;
    float tempLM35;
    float moisture;
    uint16_t lux;
} struct_message;

// Create a struct_message called receiveData
struct_message receiveData;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&receiveData, incomingData, sizeof(receiveData));

  Serial.print("ID: ");
  Serial.println(receiveData.id);
  Serial.print("Hum: ");
  Serial.println(receiveData.humidity);
  Serial.print("Temp: ");
  Serial.println(receiveData.tempDHT11);
  Serial.print("Temp35: ");
  Serial.println(receiveData.tempLM35);
  Serial.print("moisture: ");
  Serial.println(receiveData.moisture);  
  Serial.print("Light: ");
  Serial.println(receiveData.lux);

  float humidity = receiveData.humidity;
  float tempDHT11 = receiveData.tempDHT11;
  float tempLM35 = receiveData.tempLM35;
  float moisture = receiveData.moisture;
  float lux = receiveData.lux;
  
  // Firebase.setFloat(firebaseData, "/sensor/dhtHum", humidity);
  // Firebase.setFloat(firebaseData, "/sensor/dhtTemp", tempDHT11);
  // Firebase.setFloat(firebaseData, "/sensor/lm35Temp", tempLM35);
  // Firebase.setFloat(firebaseData, "/sensor/light", lux);
  // Firebase.setFloat(firebaseData, "/sensor/soil", moisture);

}

// void WiFiConnection(){
//   WiFi.begin(SSID, PASSWORD);
//   Serial.print("Connecting to Wi-Fi");
//   while (WiFi.status() != WL_CONNECTED)
//   {
//     Serial.print(".");
//     delay(300);
//   }
//   Serial.println();
//   Serial.print("Connected with IP: ");
//   Serial.println(WiFi.localIP());
//   Serial.print("Wi-Fi Channel: ");
//   Serial.println(WiFi.channel());
  
//   Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
//   Firebase.reconnectWiFi(true);
// }

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  
  // WiFi.begin(SSID, PASSWORD);
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(1000);
  //   Serial.print(".");
  // }

  // Set device as a Wi-Fi Station
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  } 
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
}
void loop()
{

}