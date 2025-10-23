#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Error reading from DHT sensor!");
    return;
  }

  // Send data to PC as CSV (comma-separated values)
  Serial.print(temperature);
  Serial.print(",");
  Serial.println(humidity);

  delay(2000); // Send data every 2 seconds
}
