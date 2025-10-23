#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define DHTPIN 2        // DHT22 data pin
#define DHTTYPE DHT22   // Sensor type
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP280 bmp; // I2C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(9600);
  dht.begin();

  if (!bmp.begin(0x76)) {
    Serial.println("Could not find BMP280 sensor!");
    while (1);
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 OLED not found!");
    while (1);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
}

void loop() {
  float tempDHT = dht.readTemperature();
  float humidity = dht.readHumidity();
  float tempBMP = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0; // in hPa

  if (isnan(tempDHT) || isnan(humidity)) {
    Serial.println("DHT22 read error!");
    return;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Weather Station");

  display.setCursor(0, 15);
  display.print("Temp (DHT22): ");
  display.print(tempDHT);
  display.println(" *C");

  display.setCursor(0, 30);
  display.print("Temp (BMP280): ");
  display.print(tempBMP);
  display.println(" *C");

  display.setCursor(0, 45);
  display.print("Humidity: ");
  display.print(humidity);
  display.println(" %");

  display.setCursor(0, 57);
  display.print("Pressure: ");
  display.print(pressure);
  display.println(" hPa");

  display.display();

  delay(2000);
}
