/**
 * Example for the simple demo application.
 * 
 * Provides a telnet tcp chat server for demonstration purposes.
 * 
 * If you want to connect, open multiple terminals and run
 *  telnet <ip of your ESP32> 1337
 * 
 * Don't forget to fill in your WiFi credentials
 */
#define WIFI_SSID "<your wifi ssid>"
#define WIFI_PSK  "<your psk>"

#include <WiFi.h>
#include "CIDemoLib.hpp"

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
  Serial.print("Connected. IP=");
  Serial.println(WiFi.localIP());

  // Run the chat server
  chatServer.start();
}

void loop() {
  // If WiFi disconnects, connect again
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting WiFi...");
    WiFi.disconnect();
    delay(500);
    WiFi.begin(WIFI_SSID, WIFI_PSK);
    delay(5000);
  }

  // Call chat's loop, so the chat runs
  chatServer.loop();
}
