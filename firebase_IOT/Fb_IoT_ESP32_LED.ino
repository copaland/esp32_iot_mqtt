
//Firebase_IOT_ESP32_LED: Control LEDs 

//Firebase-ESP-Client by Mobizt
/*********
#define in1_Pin    16             // Define the Input pin
#define in2_Pin    17             // Define the Input pin
#define in3_Pin    18             // Define the Input pin
#define in4_Pin    19             // Define the Input pin

#define out1_Pin    26             // Define the Relay Control pin
#define out2_Pin    27             // Define the Relay Control pin
#define out3_Pin    32             // Define the Relay Control pin
#define out4_Pin    33             // Define the Relay Control pin
*********/


#define ESP32
//Firebase IoT Switch  
#include "secrets.h"
#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h> //by Mobizt

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

//Define Firebase Data object
FirebaseData fbData;

FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

//If using Relay Module
int LED_1 = 26; //initialize 5 Pin
int LED_2 = 27; //initialize 4 Pin
int LED_3 = 32; //initialize 0 Pin
int LED_4 = 33; //initialize 2 Pin



void setup() {

  Serial.begin(115200);

  pinMode(LED_1,OUTPUT);//  
  pinMode(LED_2,OUTPUT);//  
  pinMode(LED_3,OUTPUT);//  
  pinMode(LED_4,OUTPUT);// 

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

    // Assign the user sign in credentials
  //auth.user.email = USER_EMAIL;
  //auth.user.password = USER_PASSWORD;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    if (Firebase.RTDB.getString(&fbData,"/AutoHome/S1")) {
      //if (fbData.dataType() == "string") {
        String state1 = fbData.stringData();
        Serial.print("event: ");
        Serial.println(state1);
        if (state1=="1"){
          digitalWrite(LED_1,HIGH); //Device1 is ON
        } else if (state1=="0"){
          digitalWrite(LED_1,LOW);//Device1 if OFF
        }
      //}
    }
    
    if (Firebase.RTDB.getString(&fbData,"/AutoHome/S2")) {
      if (fbData.dataType() == "string") {
        String state2 = fbData.stringData();
      if (state2=="1"){
        digitalWrite(LED_2,HIGH);//Device2 is ON
      } else if (state2=="0"){
        digitalWrite(LED_2,LOW);// Device2 os OFF
      }}}
      
    if (Firebase.RTDB.getString(&fbData,"/AutoHome/S3")) {
      if (fbData.dataType() == "string") {
        String state3 = fbData.stringData();
        if (state3=="1"){
          digitalWrite(LED_3,HIGH);//Device3 is ON
        } else if (state3=="0"){
          digitalWrite(LED_3,LOW);//Device3 is OFF
      }}}
                        
    if (Firebase.RTDB.getString(&fbData,"/AutoHome/S4")) {
      if (fbData.dataType() == "string") {
        String state4 = fbData.stringData();
        if (state4=="1"){
          digitalWrite(LED_4,HIGH);//Device4 is ON
        } else if (state4=="0"){
          digitalWrite(LED_4,LOW);//Device4 is OFF
      }}}
  }
}


