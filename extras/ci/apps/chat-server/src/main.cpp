#include <WiFi.h>
#include "CIDemoLib.hpp"

#ifndef WIFI_SSID
#define WIFI_SSID "unknown-network"
#endif
#ifndef WIFI_PSK
#define WIFI_PSK "unknown-psk"
#endif

// Create the server on port 1337
ChatServer chatServer(1337);

void setup() {
  // Open serial connection
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PSK);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  // We use a single line of json as this event is caught by the unit
  // test and provides the event data for the test.
  Serial.print("\n{\"event\":\"connected\",\"ip\":\"");
  Serial.print(WiFi.localIP());
  Serial.print("\"}\n");

  // Run the chat server
  chatServer.start();
  Serial.println("Server started.");
}

void loop() {
  // If WiFi disconnects, connect again
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connection lost.");
    WiFi.disconnect();
    delay(500);
    WiFi.begin(WIFI_SSID, WIFI_PSK);
    delay(5000);
  }

  // Call chat's loop, so the chat runs
  chatServer.loop();
}
