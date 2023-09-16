#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <Firebase_ESP_Client.h>
// #include <FirebaseESP32.h>

uint8_t GatewayMac[] =      {0x02, 0x10, 0x11, 0x12, 0x13, 0x14};   //{0x9C, 0x9C, 0x1F, 0xEA, 0xF4, 0x60} this is mac address of this esp

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message_received_sensor {
  int id;
  int fanStatus;
  int motorStatus;
  int pump1Status;
  int pump2Status;
  int bulbStatus;
  int chenang;
} struct_message_received_sensor;

// Create a struct_message_received_sensor called myData_sensor
struct_message_received_sensor myData_sensor;
// ------------------------------------------------------------------------------------------
// ESP-NOW GATEWAY CONFIGS
// ------------------------------------------------------------------------------------------
// Router WiFi Credentials (runnning on 2.4GHz and Channel=1)
#define WIFI_SSID       "Free Wifi"
#define WIFI_PASS       "12345679Aa"

#define SOFTAP_SSID       "Wifi Free"
#define SOFTAP_PASS       "12345679Aa"

// #define UNITS             1

// Insert Firebase project API Key
#define API_KEY "AIzaSyD8DFQJarbBjNkYWdAMYRdAx0sKwUdYJbc"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://sensor-7aff3-default-rtdb.firebaseio.com/" 

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "hnbtran23@gmail.com"
#define USER_PASSWORD "Tran12372001"

//Define Firebase Data object
FirebaseData fbdo;                        //for upload
FirebaseData fbdo_fanStatus;              //for listen to firebase change
FirebaseData fbdo_motorStatus;
FirebaseData fbdo_pump1Status;
FirebaseData fbdo_pump2Status;
FirebaseData fbdo_bulbStatus;
FirebaseData fbdo_chenang;

FirebaseAuth auth;
FirebaseConfig config;

// bool signupOK = false;

WiFiMulti wifiMulti;

int status = WL_IDLE_STATUS;

void initVariant() {
  WiFi.mode(WIFI_AP);
  esp_wifi_set_mac(WIFI_IF_AP, &GatewayMac[0]);
}

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {

  Serial.println("Received Data.................");
  memcpy(&myData_sensor, incomingData, sizeof(myData_sensor));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("ID: ");
  Serial.println(myData_sensor.id);
  Serial.print("fanStatus: ");
  Serial.println(myData_sensor.fanStatus);
  Serial.print("motorStatus: ");
  Serial.println(myData_sensor.motorStatus);
  Serial.print("pump1Status: ");
  Serial.println(myData_sensor.pump1Status);
  Serial.print("pump2Status: ");
  Serial.println(myData_sensor.pump2Status);
  Serial.print("bulbStatus: ");
  Serial.println(myData_sensor.bulbStatus);
  Serial.print("chenang: ");
  Serial.println(myData_sensor.chenang);
  Serial.println();

  if (Firebase.RTDB.setInt(&fbdo, "/status/motorStatus", myData_sensor.motorStatus)) {
    Serial.println("UPLOAD MOTOR STATUS SUCCESSED");
  }
  else {
    Serial.println("UPLOAD FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}

// void fanStatus_streamCallback(StreamData data)
void streamCallback(FirebaseStream data)
{

  // Print out all information

  Serial.println("Stream Data...");
  Serial.println(data.streamPath());
  Serial.println(data.dataPath());
  Serial.println(data.dataType());

  Serial.println(data.to<int>());

}

void streamTimeoutCallback(bool timeout)
{
  if(timeout){
    // Stream timeout occurred
    Serial.println("Stream timeout, resume streaming...");
  }  
}
 
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  //Wifi mode AP + set MAC
  initVariant();

  Serial.print("Connecting to WiFi...");

  // Set device in AP mode to begin with
  WiFi.mode(WIFI_AP_STA);                         // AP _and_ STA is required (!IMPORTANT)
  // WiFi.mode(WIFI_STA);

  wifiMulti.addAP(WIFI_SSID, WIFI_PASS);      // I use wifiMulti ... just by habit, i guess ....
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Come here - we are connected
  Serial.println(" Done");

  //Print WiFi data
  Serial.println("Set as AP_STA station.");
  Serial.print  ("SSID: "); Serial.println(WiFi.SSID());
  Serial.print  ("Channel: "); Serial.println(WiFi.channel());
  delay(1000);

  //Config gateway AP - set SSID and channel 
  int channel = WiFi.channel();
  if (WiFi.softAP(SOFTAP_SSID, SOFTAP_PASS, channel, 1)) {
    Serial.println("AP Config Success. AP SSID: " + String(SOFTAP_SSID));
  } else {
    Serial.println("AP Config failed.");
  }

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // /* Sign up */
  // if (Firebase.signUp(&config, &auth, "", "")) {
  //   Serial.println("Firebase Sign Up ok");
  //   signupOK = true;
  // }
  // else {
  //   Serial.printf("%s\n", config.signer.signupError.message.c_str());
  // }
  
  Serial.print("Connecting to Firebase....");

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Done");

  Firebase.RTDB.setStreamCallback(&fbdo_fanStatus, streamCallback, streamTimeoutCallback);
  Serial.println("Set Call Back Done");
  //stream for listen firebase change
  if (!Firebase.RTDB.beginStream(&fbdo_fanStatus, "/status/fanStatus")) {
    Serial.print("Stream fanStatus begin error, ");
    Serial.println(fbdo_fanStatus.errorReason());
  }
  else {
    Serial.println("Stream fanStatus begin");
  }



  // Init ESP-NOW
  if (esp_now_init() == ESP_OK) {
    Serial.println("Success initializing ESP-NOW");
    // Serial.print  ("WiFi mode: "); Serial.println(channel);
  }
  else {
    Serial.println("Error initializing ESP-NOW");
    ESP.restart();     
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {
  // if (Firebase.ready() && signupOK) {
  //   if (!Firebase.RTDB.readStream(&fbdo_fanStatus)) {
  //     Serial.print("Stream fanStatus read error, ");
  //     Serial.println(fbdo_fanStatus.errorReason().c_str());
  //   }
  //   if (fbdo_fanStatus.streamAvailable()) {
  //     if (fbdo_fanStatus.dataType() == "int") {
  //       Serial.println();
  //       Serial.print("fanStatus change: ");
  //       Serial.println(fbdo_fanStatus.intData());
  //     }
  //   }
  // }
  if (WiFi.status() == WL_CONNECTED) {
    // Serial.println("Wifi status WL_CONNECTED.......................");
    if (!Firebase.RTDB.readStream(&fbdo_fanStatus)) {
      Serial.println("Firebase KO stream.......................");
      int channel = WiFi.channel();
      Serial.print("Wifi channel in loop ");
      Serial.println(channel);
      if (WiFi.softAP(SOFTAP_SSID, SOFTAP_PASS, channel, 1)) {
        Serial.println("Disconnect, change STA channel for ESP-NOW");
      } else {
        Serial.println("AP Config failed.");
      }
      reconnect();
    }
  }
}


void reconnect() {
  
  initVariant();  
  Serial.println("Truoc While........................................................");
  // Loop until we're reconnected
  // while
  if (!Firebase.RTDB.readStream(&fbdo_fanStatus)) {
    Serial.println("Trong While.......................");
    status = WiFi.status();
    if ( status != WL_CONNECTED) {
      WiFi.mode(WIFI_AP_STA);                         // AP _and_ STA is required (!IMPORTANT)
      // WiFi.mode(WIFI_STA);
      Serial.print("Reconnect to Wifi...");
      wifiMulti.addAP(WIFI_SSID, WIFI_PASS);      // I use wifiMulti ... just by habit, i guess ....
      while (wifiMulti.run() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Done");
    }
    Serial.println("ReConnecting to Firebase");
    // Attempt to connect (clientId, username, password)
    // if (Firebase.signUp(&config, &auth, "", "")) {
    //   Serial.println("Firebase RE Sign Up ok");
    //   signupOK = true;

      Firebase.begin(&config, &auth);
      Firebase.reconnectWiFi(true);
      Serial.println("Firebase Reconnect ok");

      Firebase.RTDB.setStreamCallback(&fbdo_fanStatus, streamCallback, streamTimeoutCallback);
      
      if (!Firebase.RTDB.beginStream(&fbdo_fanStatus, "/status/fanStatus")) {
        Serial.print("Stream fanStatus begin error, ");
        Serial.println(fbdo_fanStatus.errorReason());
      }
      else {
        Serial.println("Stream fanStatus begin");
      }

    // } else {
      // Serial.print( "[FAILED] [ rc = " );
      // // Serial.print( config.signer.signupError.message.c_str() );
      // Serial.println( " : retrying in 5 seconds]" );
      // // Wait 5 seconds before retrying
      // delay( 5000 );
    // }
  }
  Serial.println("Sau While..............................................");

}