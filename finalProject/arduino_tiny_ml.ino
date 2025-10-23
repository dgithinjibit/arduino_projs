/*
  

  Tiny-ML on Arduino Uno (logistic regression inference)

  - Assumes a model with 2 features: f1 and f2 (both float).
  - The Python script (train_model.py) prints MODEL_W, MODEL_B, SCALER_MEAN, SCALER_SCALE.
    Copy those constants into this sketch (replace the placeholder values below).
  - Feature extraction example: read analog A0 (raw sensor) and compute a short moving average on A0 as second feature.
  - Output: Serial log and an LED on D13 indicates class 1 (ON) vs 0 (OFF).

  Notes:
  - Uno can do simple floating-point inference; keep model tiny.
  - For true TinyML with TF Lite you need a device with more flash/RAM (e.g., Nano 33 BLE Sense).
*/

#include <Arduino.h>

// === MODEL PARAMETERS ===
// Replace these with the printed values from train_model.py
const float MODEL_W[2] = {0.01234567f, -0.00432100f};  // example placeholders
const float MODEL_B      = -0.12345678f;               // example placeholder

// Scaler parameters (mean and scale used during training)
const float SCALER_MEAN[2]  = {500.0f, 300.0f};  // replace with printed means
const float SCALER_SCALE[2] = {80.0f, 60.0f};    // replace with printed scales

// === Hardware pins ===
#define SENSOR_PIN A0    // analog sensor input (e.g., analog moisture sensor)
#define LED_PIN    13    // onboard LED to indicate class=1

// === Moving average buffer for second feature ===
const int MA_LEN = 8;
int ma_buf[MA_LEN];
int ma_idx = 0;
long ma_sum = 0;
bool ma_filled = false;

// === Timing ===
unsigned long lastMillis = 0;
const unsigned long SAMPLE_INTERVAL = 500; // ms

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  // initialize moving average buffer with first reading
  int v = analogRead(SENSOR_PIN);
  for (int i = 0; i < MA_LEN; ++i) {
    ma_buf[i] = v;
    ma_sum += v;
  }
  ma_idx = 0;
  ma_filled = true;
  Serial.println(F("Tiny-ML Arduino Inference Started"));
}

float sigmoidf(float x) {
  // simple numerically-stable sigmoid
  return 1.0f / (1.0f + expf(-x));
}

void update_moving_average(int value) {
  ma_sum -= ma_buf[ma_idx];
  ma_buf[ma_idx] = value;
  ma_sum += ma_buf[ma_idx];
  ma_idx = (ma_idx + 1) % MA_LEN;
}

float get_moving_average() {
  return (float)ma_sum / (float)MA_LEN;
}

void scale_features(float in[], float out[]) {
  // standard scaler: (x - mean) / scale
  for (int i = 0; i < 2; ++i) {
    out[i] = (in[i] - SCALER_MEAN[i]) / SCALER_SCALE[i];
  }
}

int model_predict(float features[2]) {
  // linear combination
  float z = MODEL_B;
  for (int i = 0; i < 2; ++i) {
    z += MODEL_W[i] * features[i];
  }
  float prob = sigmoidf(z);
  return (prob >= 0.5f) ? 1 : 0;
}

void loop() {
  unsigned long now = millis();
  if (now - lastMillis < SAMPLE_INTERVAL) return;
  lastMillis = now;

  // Read sensor
  int raw = analogRead(SENSOR_PIN); // 0-1023
  update_moving_average(raw);
  float ma = get_moving_average();

  // Construct raw features (example): f1 = raw, f2 = moving average
  float raw_features[2];
  raw_features[0] = (float)raw;
  raw_features[1] = ma;

  // Scale features using training scaler params
  float features[2];
  scale_features(raw_features, features);

  // Run tiny model
  int cls = model_predict(features);

  // Output
  digitalWrite(LED_PIN, cls ? HIGH : LOW);
  Serial.print("raw:");
  Serial.print(raw);
  Serial.print(" ma:");
  Serial.print(ma, 2);
  Serial.print(" -> cls:");
  Serial.println(cls);

  // Optionally print probability (for debug)
  // compute z and prob for printing
  float z = MODEL_B + MODEL_W[0]*features[0] + MODEL_W[1]*features[1];
  float prob = sigmoidf(z);
  Serial.print(" probability:");
  Serial.println(prob, 4);
}
