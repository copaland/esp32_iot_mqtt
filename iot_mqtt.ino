//IoTmqtt 
/*
library Manager Tap : Add library
-DHT sensor library
-PubSubClient
-FastLED
-ESP32Servo
*/
#include <Adafruit_Sensor.h>
#include <DHT_U.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h> // Include the Servo library
#include <FastLED.h> // Include the FastLED library

// Defining Pins
#define DHTPIN 12
#define SW_PIN 25 //Button Pin
#define RLED_PIN 26
#define GLED_PIN 27
#define SERVO_PIN 2 // Servo motor pin
#define NEO_PIN 4 // WS2812 LED strip pin
#define NUM_LEDS 16 // Number of LEDs in the strip

// DHT parameters
#define DHTTYPE    DHT22     // DHT 11
DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;

// Servo motor
Servo servo;

// WS2812 LED strip
CRGB leds[NUM_LEDS];

// MQTT Credentials
const char* ssid = "Wokwi-GUEST"; // Setting your AP SSID
const char* password = ""; // Setting your AP PSK
const char* mqttServer = "broker.hivemq.com";
// const char* mqttUserName = "bqzbdodo";
// const char* mqttPwd = "5oU2W_QN2WD8";
const char* clientID = "ujaisldaaasdfgh;laslksdja1"; // Client ID username+0001
const char* topic = "Tempdata"; // Publish topic

// Parameters for using non-blocking delay
unsigned long previousMillis = 0;
const long interval = 1000;
String msgStr = "";      // MQTT message buffer
float temp, humi;

bool ledState = LOW;
int oldKey = 0;

// Setting up WiFi and MQTT client
WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect(clientID)) {
      Serial.println("MQTT connected");
      client.subscribe("lights");
      client.subscribe("servo"); // Subscribe to servo topic
      client.subscribe("lights/neopixel"); // Subscribe to neopixel topic
      Serial.println("Topic Subscribed");
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);  // wait 5sec and retry
    }
  }
}

// Subscribe callback
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  String data = "";
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    data += (char)payload[i];
  }
  Serial.println();
  Serial.print("Message size: ");
  Serial.println(length);
  Serial.println();
  Serial.println("-----------------------");
  Serial.println(data);

  if (String(topic) == "lights") {
    if (data == "ON") {
      Serial.println("LED");
      digitalWrite(RLED_PIN, HIGH);
    }
    else {
      digitalWrite(RLED_PIN, LOW);
    }
  }
  else if (String(topic) == "servo") {
    int degree = data.toInt(); // Convert the received data to an integer
    Serial.print("Moving servo to degree: ");
    Serial.println(degree);
    servo.write(degree); // Move the servo to the specified degree
  }
  else if (String(topic) == "lights/neopixel") {
    int red, green, blue;
    sscanf(data.c_str(), "%d,%d,%d", &red, &green, &blue); // Parse the received data into RGB values
    Serial.print("Setting NeoPixel color to (R,G,B): ");
    Serial.print(red);
    Serial.print(",");
    Serial.print(green);
    Serial.print(",");
    Serial.println(blue);
    fill_solid(leds, NUM_LEDS, CRGB(red, green, blue)); // Set all LEDs in the strip to the specified color
    FastLED.show(); // Update the LED strip with the new color
     fill_solid(leds, NUM_LEDS, CRGB(red, green, blue));
     FastLED.show();
  }
}

//debounce Button
void checkBTN() {
 
  int KEY = !digitalRead(SW_PIN);

  if((KEY != oldKey) && KEY){ // if KEY pressed
    __asm volatile("NOP"); //__NOP;
    ledState = !ledState;
    digitalWrite(GLED_PIN, ledState);   // 스위치 버튼 1이 눌렸을 때 LED1 켜기
    //Blynk.virtualWrite(V0, ledState);       // 가상 핀 V0에 1(켜짐)을 보냄
  }
  oldKey = KEY;

  delay(10);
}

void setup() {
  Serial.begin(115200);
  // Initialize device.
  dht.begin();
  // Get temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);
  pinMode(RLED_PIN, OUTPUT);
  digitalWrite(RLED_PIN, LOW);
  pinMode(GLED_PIN, OUTPUT);
  digitalWrite(GLED_PIN, LOW);
  pinMode(SW_PIN, INPUT_PULLUP);
  // Setup servo
  servo.attach(SERVO_PIN, 500, 2400);
  servo.write(0);

  // Setup WS2812 LED strip
  FastLED.addLeds<WS2812, NEO_PIN, GRB>(leds, NUM_LEDS);

  setup_wifi();
  client.setServer(mqttServer, 1883); // Setting MQTT server
  client.setCallback(callback); // Define function which will be called when a message is received.
}

void loop() {
  if (!client.connected()) { // If client is not connected
    reconnect(); // Try to reconnect
  }
  client.loop();
  unsigned long currentMillis = millis(); // Read current time
  if (currentMillis - previousMillis >= interval) { // If current time - last time > 5 sec
    previousMillis = currentMillis;
    // Read temperature and humidity
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature)) {
      Serial.println(F("Error reading temperature!"));
    }
    else {
      Serial.print(F("Temperature: "));
      temp = event.temperature;
      Serial.print(temp);
      Serial.println(F("°C"));
    }
    // Get humidity event and print its value
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) {
      Serial.println(F("Error reading humidity!"));
    }
    else {
      Serial.print(F("Humidity: "));
      humi = event.relative_humidity;
      Serial.print(humi);
      Serial.println(F("%"));
    }
    msgStr = String(temp) + "," + String(humi) + ",";
    byte arrSize = msgStr.length() + 1;
    char msg[arrSize];
    Serial.print("PUBLISH DATA: ");
    Serial.println(msgStr);
    msgStr.toCharArray(msg, arrSize);
    client.publish(topic, msg);
    msgStr = "";
    delay(1);
  }
  //checkSwitch();
  checkBTN();
}
