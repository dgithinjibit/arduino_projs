/*
  Arduino-Based Smart Home Control
  -------------------------------------------
  Description:
    This program allows remote control of home appliances (e.g., lights, fans)
    using an Arduino Uno and an ESP8266 WiFi module (in AT command mode).

  Components:
    - Arduino Uno
    - ESP8266 WiFi module (ESP-01 or NodeMCU in serial mode)
    - 2-Channel Relay Module
    - Smartphone or PC (for web control)
    - 5V power supply

  Skills:
    - WiFi communication via AT commands
    - Web server design (simplified)
    - Relay control and safety isolation
*/

#include <SoftwareSerial.h>

#define ESP_RX 2      // Arduino RX (to ESP TX)
#define ESP_TX 3      // Arduino TX (to ESP RX)
SoftwareSerial esp(ESP_RX, ESP_TX);

#define RELAY1 7      // Relay 1 controls light
#define RELAY2 8      // Relay 2 controls fan or appliance

String request = "";

void setup() {
  Serial.begin(9600);
  esp.begin(115200);
  
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);

  digitalWrite(RELAY1, HIGH); // Relays are active LOW
  digitalWrite(RELAY2, HIGH);

  Serial.println("Smart Home System Starting...");
  sendCommand("AT", "OK", 2000);
  sendCommand("AT+CWMODE=1", "OK", 2000);
  sendCommand("AT+CWJAP=\"YourWiFiSSID\",\"YourWiFiPassword\"", "WIFI GOT IP", 10000);
  sendCommand("AT+CIPMUX=1", "OK", 2000);
  sendCommand("AT+CIPSERVER=1,80", "OK", 2000);

  Serial.println("WiFi Connected. Smart Home Server Ready!");
  Serial.println("Use smartphone browser: http://<your-module-IP>");
}

void loop() {
  if (esp.available()) {
    char c = esp.read();
    request += c;

    if (request.endsWith("\r\n\r\n")) {
      Serial.println(request);

      // Handle relay control commands
      if (request.indexOf("/light/on") != -1) {
        digitalWrite(RELAY1, LOW);
      } else if (request.indexOf("/light/off") != -1) {
        digitalWrite(RELAY1, HIGH);
      }

      if (request.indexOf("/fan/on") != -1) {
        digitalWrite(RELAY2, LOW);
      } else if (request.indexOf("/fan/off") != -1) {
        digitalWrite(RELAY2, HIGH);
      }

      // Send simple HTML page to user
      sendHTTPResponse();

      request = "";
    }
  }
}

// --- Helper Functions ---

void sendCommand(String cmd, String ack, int timeout) {
  esp.println(cmd);
  long int time = millis();
  while ((millis() - time) < timeout) {
    while (esp.available()) {
      String response = esp.readString();
      if (response.indexOf(ack) != -1) {
        return;
      }
    }
  }
  Serial.println("Command Timeout: " + cmd);
}

void sendHTTPResponse() {
  String html = 
  "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
  "<!DOCTYPE html><html><head><title>Smart Home Control</title>"
  "<style>body{font-family:sans-serif;text-align:center;}button{padding:10px 20px;margin:5px;font-size:16px;}</style>"
  "</head><body>"
  "<h1>Smart Home Control</h1>"
  "<p><b>Light</b></p>"
  "<button onclick=\"window.location.href='/light/on'\">ON</button>"
  "<button onclick=\"window.location.href='/light/off'\">OFF</button>"
  "<p><b>Fan</b></p>"
  "<button onclick=\"window.location.href='/fan/on'\">ON</button>"
  "<button onclick=\"window.location.href='/fan/off'\">OFF</button>"
  "<p>Status: OK</p>"
  "</body></html>";

  String sendCmd = "AT+CIPSEND=0," + String(html.length());
  sendCommand(sendCmd, ">", 2000);
  esp.print(html);
  delay(500);
  esp.println("AT+CIPCLOSE=0");
}
