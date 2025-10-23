/*
  Arduino-Controlled Servo Arm
  --------------------------------------
  Description:
    This program controls a simple robotic arm using a joystick.
    The arm has multiple servo motors (base, shoulder, elbow, gripper)
    which move smoothly based on analog joystick input.

  Components:
    - Arduino Uno
    - 4x Servo Motors (Base, Shoulder, Elbow, Gripper)
    - 2-Axis Joystick Module (X, Y, SW)
    - Optional: External 5V power supply for servos (with common GND)
*/

#include <Servo.h>

// === Servo Objects ===
Servo baseServo;
Servo shoulderServo;
Servo elbowServo;
Servo gripperServo;

// === Pin Definitions ===
#define JOY_X A0       // Joystick X-axis controls base rotation
#define JOY_Y A1       // Joystick Y-axis controls shoulder
#define JOY_SW 2       // Joystick button (for gripper)
#define ELBOW_POT A2   // Optional potentiometer for elbow control

#define BASE_PIN 9
#define SHOULDER_PIN 10
#define ELBOW_PIN 11
#define GRIPPER_PIN 6

// === Servo Position Variables ===
int basePos = 90;        // Default center
int shoulderPos = 90;
int elbowPos = 90;
int gripperPos = 90;

// === Configuration ===
int servoSpeed = 3;       // Controls servo smoothness (higher = slower)
int deadZone = 50;        // Joystick tolerance around center

void setup() {
  Serial.begin(9600);

  baseServo.attach(BASE_PIN);
  shoulderServo.attach(SHOULDER_PIN);
  elbowServo.attach(ELBOW_PIN);
  gripperServo.attach(GRIPPER_PIN);

  pinMode(JOY_SW, INPUT_PULLUP);

  // Initialize arm to neutral positions
  baseServo.write(basePos);
  shoulderServo.write(shoulderPos);
  elbowServo.write(elbowPos);
  gripperServo.write(gripperPos);

  Serial.println("Servo Arm Control Initialized!");
  Serial.println("Move joystick to control arm; press button to toggle gripper.");
}

void loop() {
  int joyX = analogRead(JOY_X);
  int joyY = analogRead(JOY_Y);
  int elbowInput = analogRead(ELBOW_POT);
  int buttonState = digitalRead(JOY_SW);

  // Map joystick to servo motion
  // Center joystick is ~512, map left/right or up/down to ±motion
  if (joyX < (512 - deadZone)) {
    basePos = constrain(basePos - 1, 0, 180);
  } else if (joyX > (512 + deadZone)) {
    basePos = constrain(basePos + 1, 0, 180);
  }

  if (joyY < (512 - deadZone)) {
    shoulderPos = constrain(shoulderPos + 1, 0, 180);
  } else if (joyY > (512 + deadZone)) {
    shoulderPos = constrain(shoulderPos - 1, 0, 180);
  }

  // Elbow controlled by potentiometer (maps 0–1023 → 0–180)
  elbowPos = map(elbowInput, 0, 1023, 0, 180);
  elbowPos = constrain(elbowPos, 0, 180);

  // Gripper toggle (open/close) when joystick button pressed
  static bool gripperClosed = false;
  static unsigned long lastPress = 0;
  if (buttonState == LOW && millis() - lastPress > 500) {
    gripperClosed = !gripperClosed;
    gripperPos = gripperClosed ? 30 : 90; // adjust as needed for servo angle
    lastPress = millis();
  }

  // Move servos smoothly
  baseServo.write(basePos);
  shoulderServo.write(shoulderPos);
  elbowServo.write(elbowPos);
  gripperServo.write(gripperPos);

  // Debug info
  Serial.print("Base: "); Serial.print(basePos);
  Serial.print(" | Shoulder: "); Serial.print(shoulderPos);
  Serial.print(" | Elbow: "); Serial.print(elbowPos);
  Serial.print(" | Gripper: "); Serial.print(gripperPos);
  Serial.println();

  delay(servoSpeed);
}
