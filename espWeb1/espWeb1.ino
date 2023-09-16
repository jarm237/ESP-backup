#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <FirebaseESP32.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

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

/* 1. Define the WiFi credentials */
#define WIFI_SSID         "Free Wifi"
#define WIFI_PASSWORD     "12345679Aa"

/* 2. Define the API Key */
#define API_KEY           "AIzaSyD8DFQJarbBjNkYWdAMYRdAx0sKwUdYJbc"
// #define API_KEY           "AIzaSyD8DFQJarbBjNkYWdAMYRdAx0sKwUdYJb"

/* 3. Define the RTDB URL */
#define DATABASE_URL      "https://sensor-7aff3-default-rtdb.firebaseio.com/" 

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL        "hnbtran23@gmail.com"
#define USER_PASSWORD     "Tran12372001"

bool signupOK = false;
String uid;

FirebaseAuth auth;
FirebaseConfig config;

// Define Firebase Data object
// FirebaseData fbdo_sensor;
// FirebaseData fbdo_status;
FirebaseData fbdo;

String sensorPath = "/sensor";

String dhtHumPath = "/dhtHum";
String dhtTempPath = "/dhtTemp";
String lightPath = "/lightHum";
String lm35TempPath = "/lm35TempHum";
String soilPath = "/soilHum";

String statusPath = "/status";

String bulbStatusPath = "/bulbStatus";
String fanStatusPath = "/fanStatus";
String motorStatusPath = "/motorStatus";
String pump1StatusPath = "/pump1Status";
String pump2StatusPath = "/pump2Status";

FirebaseJson jsonSensor;
FirebaseJson jsonStatus;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {

  Serial.println("Received Data.................");
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(len);

  Serial.print("dhtHum: ");
  Serial.println(myData.dhtHum);
  Serial.print("dhtTemp: ");
  Serial.println(myData.dhtTemp);
  Serial.print("light: ");
  Serial.println(myData.light);
  Serial.print("lm35Temp: ");
  Serial.println(myData.lm35Temp);
  Serial.print("soil: ");
  Serial.println(myData.soil);
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
  
  Serial.println();

  Serial.print("WiFi channel: ");
  Serial.println(WiFi.channel());

  if (Firebase.ready()) {
    jsonSensor.set(dhtHumPath, myData.dhtHum);
    jsonSensor.set(dhtTempPath, myData.dhtTemp);
    jsonSensor.set(lightPath, myData.light);
    jsonSensor.set(lm35TempPath, myData.lm35Temp);
    jsonSensor.set(soilPath, myData.soil);

    jsonStatus.set(bulbStatusPath, myData.bulbStatus);
    jsonStatus.set(fanStatusPath, myData.fanStatus);
    jsonStatus.set(motorStatusPath, myData.motorStatus);
    jsonStatus.set(pump1StatusPath, myData.pump1Status);
    jsonStatus.set(pump2StatusPath, myData.pump2Status);

    Serial.println("JSON set done");

    // if (Firebase.setJSON(fbdo, sensorPath, jsonSensor)) {
    //   Serial.println("UPLOAD SENSOR SUCCESSED");
    // }
    // else {
    //   Serial.println("UPLOAD SENSOR FAILED");
    //   Serial.println("REASON: " + fbdo.errorReason());
    // }

    // if (Firebase.setJSON(fbdo, statusPath, jsonStatus)) {
    //   Serial.println("UPLOAD STATUS SUCCESSED");
    // }
    // else {
    //   Serial.println("UPLOAD STATUS FAILED");
    //   Serial.println("REASON: " + fbdo.errorReason());
    // }
    if (Firebase.setInt(fbdo, "/sensor/dhtHum", myData.dhtHum)) {
      Serial.println("UPLOAD STATUS SUCCESSED");
    }
    else {
      Serial.println("UPLOAD STATUS FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_AP_STA);
  // WiFi.mode(WIFI_STA);


  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println(WiFi.status());
    delay(500);
  }
  Serial.println();
  Serial.print("WiFi channel: ");
  Serial.println(WiFi.channel());

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  // /* Sign up */
  // if (Firebase.signUp(&config, &auth, "", "")){
  //   Serial.println("ok");
  //   signupOK = true;
  // }
  // else{
  //   Serial.printf("%s\n", config.signer.signupError.message.c_str());
  // }

  // Assign the callback function for the long running token generation task
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  Firebase.reconnectWiFi(true);

  Firebase.begin(&config, &auth);

   // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.print(uid);

  /////////////////////////////////////////////////////////////////////////////////////

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  Serial.println("Success initializing ESP-NOW");

  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  if (Firebase.isTokenExpired()){
    Firebase.refreshToken(&config);
    Serial.println("Refresh token");
  }

}
