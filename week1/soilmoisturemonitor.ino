#define SOIL_PIN A0     // Analog pin connected to soil sensor
#define LED_PIN 7       // LED indicator pin
#define BUZZER_PIN 8    // Buzzer pin

int soilValue = 0;      // Variable to store soil sensor value
int dryThreshold = 600; // Adjust this based on calibration (0–1023)

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(SOIL_PIN, INPUT);
}

void loop() {
  soilValue = analogRead(SOIL_PIN);
  Serial.print("Soil Moisture Value: ");
  Serial.println(soilValue);

  if (soilValue > dryThreshold) {
    // Soil is dry — turn on alert
    digitalWrite(LED_PIN, HIGH);
    tone(BUZZER_PIN, 1000); // Buzz at 1kHz
    Serial.println("Status: Dry soil! Water the plant!");
  } else {
    // Soil is moist — turn off alert
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN);
    Serial.println("Status: Soil moisture OK.");
  }

  delay(2000); // Update every 2 seconds
}
