/*
  Arduino and Cloud Integration
  ----------------------------------------
  Description:
    This project reads temperature and humidity from a DHT22 sensor
    and uploads the data to ThingSpeak via an ESP8266 WiFi module.

  Components:
    - Arduino Uno
    - DHT22 Sensor
    - ESP8266 WiFi module (ESP-01 or NodeMCU in serial mode)
    - Cloud platform: ThingSpeak (https://thingspeak.com)

  Skills:
    - Sensor data collection
    - HTTP communication via WiFi (AT commands)
    - IoT data publishing
*/

#include <SoftwareSerial.h>
#include <DHT.h>

// === Pin Definitions ===
#define DHTPIN 2
#define DHTTYPE DHT22
#define ESP_RX 3   // Arduino RX  <- ESP TX
#define ESP_TX 4   // Arduino TX  -> ESP RX (use voltage divider!)
#define LED_PIN 7  // Status LED

// === WiFi & ThingSpeak Settings ===
const char* WIFI_SSID = "YourWiFiSSID";
const char* WIFI_PASS = "YourWiFiPassword";
String API_KEY = "YOUR_THINGSPEAK_API_KEY";  // Replace with your ThingSpeak Write API Key
String HOST = "api.thingspeak.com";
String PORT = "80";

SoftwareSerial esp(ESP_RX, ESP_TX);
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  esp.begin(115200);
  dht.begin();
  pinMode(LED_PIN, OUTPUT);

  Serial.println("\n[ThingSpeak Cloud Uploader]");
  sendCommand("AT", "OK", 2000);
  sendCommand("AT+CWMODE=1", "OK", 2000);
  sendCommand("AT+CWJAP=\"" + String(WIFI_SSID) + "\",\"" + String(WIFI_PASS) + "\"", "WIFI GOT IP", 10000);

  Serial.println("WiFi Connected! Starting data upload loop...");
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Sensor read error!");
    delay(2000);
    return;
  }

  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print(" °C | Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  digitalWrite(LED_PIN, HIGH);

  // Prepare HTTP GET request
  String requestData = "GET /update?api_key=" + API_KEY +
                       "&field1=" + String(temperature) +
                       "&field2=" + String(humidity) + " HTTP/1.1\r\n" +
                       "Host: " + HOST + "\r\n" +
                       "Connection: close\r\n\r\n";

  // Send data to ThingSpeak
  sendCommand("AT+CIPSTART=\"TCP\",\"" + HOST + "\"," + PORT, "OK", 5000);
  sendCommand("AT+CIPSEND=" + String(requestData.length()), ">", 2000);
  esp.print(requestData);
  delay(5000);
  sendCommand("AT+CIPCLOSE", "OK", 2000);

  digitalWrite(LED_PIN, LOW);
  Serial.println("Data sent to ThingSpeak!\n");

  delay(20000); // ThingSpeak allows updates every 15+ seconds
}

// === Helper Function to Communicate with ESP ===
void sendCommand(String command, String ack, int timeout) {
  esp.println(command);
  long int time = millis();
  while ((millis() - time) < timeout) {
    while (esp.available()) {
      String response = esp.readString();
      if (response.indexOf(ack) != -1) {
        Serial.println("> " + command + " OK");
        return;
      }
    }
  }
  Serial.println("> " + command + " [Timeout]");
}
