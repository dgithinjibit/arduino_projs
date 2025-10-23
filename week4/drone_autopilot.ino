/*
  Arduino-Based Drone Autopilot
  ----------------------------------------
  Description:
    A basic autopilot system for a quadcopter using MPU6050 (IMU) for stabilization
    and ESCs to control brushless DC motors. Optional GPS integration for position hold.

  Components:
    - Arduino Uno
    - MPU-6050 Accelerometer & Gyroscope
    - 4 x ESC + Brushless DC Motors
    - GPS Module (e.g., NEO-6M)
    - LiPo Battery, Frame, Propellers

  Skills:
    - Sensor fusion (gyro + accelerometer)
    - PID control
    - PWM motor control
    - GPS integration

  Note:
    This is an educational implementation – not intended for actual flight
    without failsafes, calibration, and real-time safety checks.
*/

#include <Wire.h>
#include <Servo.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

// === MPU6050 Setup ===
const int MPU = 0x68;
float AccX, AccY, AccZ, GyroX, GyroY, GyroZ;
float AccAngleX, AccAngleY, GyroAngleX, GyroAngleY, GyroAngleZ;
float roll, pitch, yaw;
float elapsedTime, currentTime, previousTime;

// === PID Constants ===
float kp = 2.5;     // Proportional gain
float ki = 0.05;    // Integral gain
float kd = 1.2;     // Derivative gain
float errorRoll, errorPitch, errorYaw;
float prevErrorRoll, prevErrorPitch, prevErrorYaw;
float pidRoll, pidPitch, pidYaw;
float totalRoll, totalPitch, totalYaw;

// === Motor ESC Outputs ===
Servo motor1; // Front Left
Servo motor2; // Front Right
Servo motor3; // Rear Left
Servo motor4; // Rear Right

// === GPS Setup (optional) ===
SoftwareSerial gpsSerial(10, 11); // RX, TX
TinyGPSPlus gps;

// === Throttle Control ===
int throttle = 1200;  // Base speed (1000-2000 µs range)

void setup() {
  Serial.begin(9600);
  Wire.begin();

  // === Initialize MPU6050 ===
  Wire.beginTransmission(MPU);
  Wire.write(0x6B); 
  Wire.write(0);
  Wire.endTransmission(true);
  delay(100);

  // === Attach Motors ===
  motor1.attach(3);
  motor2.attach(5);
  motor3.attach(6);
  motor4.attach(9);

  // === Initialize GPS ===
  gpsSerial.begin(9600);
  Serial.println("Drone Autopilot Initialized\n");

  // === Arm Motors (ESC calibration) ===
  for (int i = 1000; i <= 1200; i += 20) {
    setMotorSpeeds(i, i, i, i);
    delay(50);
  }
}

void loop() {
  // === IMU Readings ===
  readMPU();

  // === Angle Calculation (Sensor Fusion) ===
  AccAngleX = atan(AccY / sqrt(pow(AccX, 2) + pow(AccZ, 2))) * 180 / PI;
  AccAngleY = atan(-AccX / sqrt(pow(AccY, 2) + pow(AccZ, 2))) * 180 / PI;

  currentTime = millis();
  elapsedTime = (currentTime - previousTime) / 1000.0;
  previousTime = currentTime;

  GyroAngleX += GyroX * elapsedTime;
  GyroAngleY += GyroY * elapsedTime;
  GyroAngleZ += GyroZ * elapsedTime;

  // Complementary Filter
  roll = 0.96 * (GyroAngleX) + 0.04 * (AccAngleX);
  pitch = 0.96 * (GyroAngleY) + 0.04 * (AccAngleY);
  yaw = GyroAngleZ;

  // === PID Control ===
  stabilizeWithPID(0, 0, 0); // Target angles: roll=0, pitch=0, yaw=0 (level hover)

  // === Apply Motor Outputs ===
  int m1 = throttle + pidPitch - pidRoll - pidYaw;
  int m2 = throttle + pidPitch + pidRoll + pidYaw;
  int m3 = throttle - pidPitch - pidRoll + pidYaw;
  int m4 = throttle - pidPitch + pidRoll - pidYaw;

  m1 = constrain(m1, 1100, 2000);
  m2 = constrain(m2, 1100, 2000);
  m3 = constrain(m3, 1100, 2000);
  m4 = constrain(m4, 1100, 2000);

  setMotorSpeeds(m1, m2, m3, m4);

  // === GPS Data (optional) ===
  if (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
    if (gps.location.isUpdated()) {
      Serial.print("GPS Lat: ");
      Serial.print(gps.location.lat(), 6);
      Serial.print(" | Lng: ");
      Serial.println(gps.location.lng(), 6);
    }
  }

  // === Debug Info ===
  Serial.print("Roll: "); Serial.print(roll);
  Serial.print(" | Pitch: "); Serial.print(pitch);
  Serial.print(" | Yaw: "); Serial.println(yaw);
  delay(50);
}

// === Read MPU6050 Function ===
void readMPU() {
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14, true);

  AccX = (Wire.read() << 8 | Wire.read()) / 16384.0;
  AccY = (Wire.read() << 8 | Wire.read()) / 16384.0;
  AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0;
  GyroX = (Wire.read() << 8 | Wire.read()) / 131.0;
  GyroY = (Wire.read() << 8 | Wire.read()) / 131.0;
  GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0;
}

// === PID Stabilization Function ===
void stabilizeWithPID(float targetRoll, float targetPitch, float targetYaw) {
  errorRoll = roll - targetRoll;
  errorPitch = pitch - targetPitch;
  errorYaw = yaw - targetYaw;

  totalRoll += errorRoll * ki;
  totalPitch += errorPitch * ki;
  totalYaw += errorYaw * ki;

  pidRoll = kp * errorRoll + ki * totalRoll + kd * (errorRoll - prevErrorRoll);
  pidPitch = kp * errorPitch + ki * totalPitch + kd * (errorPitch - prevErrorPitch);
  pidYaw = kp * errorYaw + ki * totalYaw + kd * (errorYaw - prevErrorYaw);

  prevErrorRoll = errorRoll;
  prevErrorPitch = errorPitch;
  prevErrorYaw = errorYaw;
}

// === Set Motor Speeds Function ===
void setMotorSpeeds(int m1, int m2, int m3, int m4) {
  motor1.writeMicroseconds(m1);
  motor2.writeMicroseconds(m2);
  motor3.writeMicroseconds(m3);
  motor4.writeMicroseconds(m4);
}
