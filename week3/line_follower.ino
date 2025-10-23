// Pin assignments
#define LEFT_SENSOR 2
#define RIGHT_SENSOR 3
#define ENA 5           // PWM for left motor
#define ENB 6           // PWM for right motor
#define IN1 7           // Left motor direction
#define IN2 8
#define IN3 9           // Right motor direction
#define IN4 10

int leftSensor, rightSensor;

void setup() {
  pinMode(LEFT_SENSOR, INPUT);
  pinMode(RIGHT_SENSOR, INPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  leftSensor = digitalRead(LEFT_SENSOR);
  rightSensor = digitalRead(RIGHT_SENSOR);

  Serial.print("Left: ");
  Serial.print(leftSensor);
  Serial.print(" | Right: ");
  Serial.println(rightSensor);

  if (leftSensor == LOW && rightSensor == LOW) {
    // Both sensors see line → move forward
    moveForward();
  } 
  else if (leftSensor == LOW && rightSensor == HIGH) {
    // Left on line → turn left
    turnLeft();
  } 
  else if (leftSensor == HIGH && rightSensor == LOW) {
    // Right on line → turn right
    turnRight();
  } 
  else {
    // Both off line → stop
    stopMotors();
  }
}

void moveForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 150);
  analogWrite(ENB, 150);
}

void turnLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 100);
  analogWrite(ENB, 150);
}

void turnRight() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, 150);
  analogWrite(ENB, 100);
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}
