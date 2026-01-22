/*
  Receiver Station: ESP8266
  Function: Receives water level data and controls visual indicators (LEDs).
*/

#include <ESP8266WiFi.h>
#include <espnow.h>

// Pins for Indicators 
#define GREEN_LED D1
#define RED_LED   D2
#define WATER_LEVEL_THRESHOLD 50 

typedef struct struct_message {
  int waterLevel;
} struct_message;

struct_message incomingData;

// Function called when data is received 
void OnDataRecv(uint8_t * mac, uint8_t *incoming, uint8_t len) {
  memcpy(&incomingData, incoming, sizeof(incomingData));
  Serial.print("Received Water Level: ");
  Serial.println(incomingData.waterLevel);

  if (incomingData.waterLevel >= WATER_LEVEL_THRESHOLD) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    Serial.println("DANGER: Water level high!");
  } else {
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    Serial.println("SAFE: Water level normal.");
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  
  // Default to Safe state
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE); 
  esp_now_register_recv_cb(OnDataRecv);     
}

void loop() {
  // Logic is handled in the OnDataRecv callback
}