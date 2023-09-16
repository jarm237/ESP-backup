#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <FirebaseESP32.h>

uint8_t broadcastAddress[] = {0x78, 0x21, 0x84, 0xDD, 0xF8, 0x74};

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

esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

/* 1. Define the WiFi credentials */
#define WIFI_SSID         "Free Wifi"
#define WIFI_PASSWORD     "12345679Aa"

/* 2. Define the API Key */
#define API_KEY           "AIzaSyD8DFQJarbBjNkYWdAMYRdAx0sKwUdYJbc"

/* 3. Define the RTDB URL */
#define DATABASE_URL      "https://sensor-7aff3-default-rtdb.firebaseio.com/" 

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL        "hnbtran23@gmail.com"
#define USER_PASSWORD     "Tran12372001"

FirebaseAuth auth;
FirebaseConfig config;

// Define Firebase Data object
FirebaseData data;

String dataPath = "/";

String childPath[14] = {"/mode",
                        "/settingThreshold/hourMotor", 
                        "/settingThreshold/hourPump", 
                        "/settingThreshold/humThreshold", 
                        "/settingThreshold/lightThreshold", 
                        "/settingThreshold/soilThreshold",
                        "/settingThreshold/tempThreshold", 
                        "/settingThreshold/timeMotor", 
                        "/settingThreshold/timePump", 
                        "/status/bulbStatus", 
                        "/status/fanStatus", 
                        "/status/motorStatus", 
                        "/status/pump1Status", 
                        "/status/pump2Status"};

void dataStreamCallback(MultiPathStreamData stream)
{
  size_t numChild = sizeof(childPath) / sizeof(childPath[0]);
  int toSend = 0;

  for (size_t i = 0; i < numChild; i++)
  {
    if (stream.get(childPath[i]))
    {
      Serial.printf("Test stream path: %s, value: %s", stream.dataPath.c_str(), stream.value.c_str());
      Serial.println();
      switch (i)
      {
        case 0:
        {
          myData.mode = stream.value.toInt();
          break;
        }
        case 1:
        {
          myData.hourMotor = stream.value.toInt();
          break;
        }
        case 2:
        {
          myData.hourPump = stream.value.toInt();
          break;
        }
        case 3:
        {
          myData.humThreshold = stream.value.toInt();
          break;
        }
        case 4:
        {
          myData.lightThreshold = stream.value.toInt();
          break;
        }
        case 5:
        {
          myData.soilThreshold = stream.value.toInt();
          break;
        }
        case 6:
        {
          myData.tempThreshold = stream.value.toInt();
          break;
        }
        case 7:
        {
          myData.timeMotor = stream.value.toInt();
          break;
        }
        case 8:
        {
          myData.timePump = stream.value.toInt();
          break;
        }
        case 9:
        {
          myData.bulbStatus = stream.value.toInt();
          break;
        }
        case 10:
        {
          myData.fanStatus = stream.value.toInt();
          break;
        }
        case 11:
        {
          myData.motorStatus = stream.value.toInt();
          break;
        }
        case 12:
        {
          myData.pump1Status = stream.value.toInt();
          break;
        }
        case 13:
        {
          myData.pump2Status = stream.value.toInt();
          break;
        }
      }

      toSend = 1;
    }
  }

  if (toSend) {
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.print("Error sending the data, result = ");
      Serial.println(result);
    }
  }
}

void streamTimeoutCallback(bool timeout)
{
  Serial.println("Time out ...............");
}


void setup() {

  Serial.begin(115200);

  WiFi.mode(WIFI_AP_STA);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println(WiFi.status());
    delay(500);
  }
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);

  Firebase.begin(&config, &auth);
  Serial.println("Connected to Firebase");


  /////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////// Init ESP-NOW  ///////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  Serial.println("Success initializing ESP-NOW");

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

  if (!Firebase.beginMultiPathStream(data, dataPath))
    Serial.printf("Data stream begin error, %s\n\n", data.errorReason().c_str());
  
  delay(1000);
  
  Firebase.setMultiPathStreamCallback(data, dataStreamCallback, streamTimeoutCallback);

}

void loop() {

}