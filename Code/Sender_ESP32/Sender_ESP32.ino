/*
  Transmitter Station: ESP32 (Master)
  Function: Measures water level, sends local alerts via ESP-NOW, and updates Blynk Cloud.
*/

#define BLYNK_TEMPLATE_ID "Your_Template_ID"
#define BLYNK_TEMPLATE_NAME "waterlog"

#include <WiFi.h>
#include <esp_now.h>
#include <BlynkSimpleEsp32.h>
#include <NewPing.h>

// Credentials 
char auth[] = "Your_Blynk_Auth_Token"; 
char ssid[] = "Your_SSID";
char pass[] = "Your_PASSWORD";

// Hardware Pins 
const int trigPin = 5;
const int echoPin = 18;
const int relayPin = 23;
#define MAX_DISTANCE 400

// ESP-NOW Configuration 
typedef struct struct_message {
  int waterLevel;
} struct_message;

struct_message dataToSend;
uint8_t receiverMAC[] = {0x24, 0x6F, 0x28, 0xAA, 0xBB, 0xCC}; // Replace with actual Receiver MAC 

NewPing sonar(trigPin, echoPin, MAX_DISTANCE);
BlynkTimer timer;

// Callback when data is sent 
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void measureAndSend() {
  // 1. Measure Distance 
  int distance = sonar.ping_cm();
  dataToSend.waterLevel = distance;
  Serial.print("Current Water Distance: ");
  Serial.println(distance);

  // 2. Update Blynk Cloud 
  Blynk.virtualWrite(V0, distance);

  // 3. Send via ESP-NOW to Local Receivers 
  esp_now_send(receiverMAC, (uint8_t *) &dataToSend, sizeof(dataToSend));

  // 4. Local Relay Control 
  if (distance > 0 && distance < 20) { // Danger threshold
    digitalWrite(relayPin, HIGH);
    Blynk.virtualWrite(V1, 1);
  } else {
    digitalWrite(relayPin, LOW);
    Blynk.virtualWrite(V1, 0);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  // Initialize WiFi and Blynk
  WiFi.mode(WIFI_STA);
  Blynk.begin(auth, ssid, pass);

  // Initialize ESP-NOW 
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  timer.setInterval(1000L, measureAndSend); // Run every 1 second 
}

void loop() {
  Blynk.run(); 
  timer.run(); 
}