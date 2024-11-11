// Include necessary libraries
#include <Wire.h>
#include <U8g2lib.h>
#include <Servo.h>

// Function prototypes
void NinjaHome();
void displayStatus(const char *message);
void NinjaRollForward();
void NinjaRollBackward();
void NinjaRollStop();
void NinjaTurnRight();
void NinjaTurnLeft();
void NinjaWalk();
void clearBluetoothBuffer();
void sendDistance();
void sendWalkingSpeed();

// OLED Display Initialization
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);

// Servo Definitions
Servo myservoLeftFoot, myservoRightFoot, myservoLeftLeg, myservoRightLeg;
Servo myservoHead;

// Ultrasonic Sensor Pins
#define echoPin 5  // Echo pin of ultrasonic sensor (D5)
#define trigPin 4  // Trig pin of ultrasonic sensor (D4)
long duration;
int distance;

// Servo Pin Mappings
const uint8_t servo_right_wheel_pin = A1;
const uint8_t servo_right_ankle_pin = A0;
const uint8_t servo_head_pin = 8;
const uint8_t servo_left_wheel_pin = 10;
const uint8_t servo_left_ankle_pin = 11;

// Global Variables
bool isWalking = false;         // Toggle between walking and rolling
float walking_speed = 0.75;     // Default walking speed
String commandBuffer = "";      // Buffer to store incoming commands

void setup() {
  // Initialize Serial, OLED, and servos
  Serial.begin(9600);
  u8g2.begin();
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  NinjaHome();  // Set initial positions for servos
}

void loop() {
  // Check for Bluetooth commands
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');  // Read until newline character
    command.trim();  // Remove any leading or trailing whitespace
    command.toLowerCase();  // Make command case-insensitive
    BluetoothCommandHandler(command);  // Process command
  }

  // If in walking mode, the robot should walk autonomously
  if (isWalking) {
    NinjaWalk();
  }
  
  // Send distance data periodically
  sendDistance();
  sendWalkingSpeed();
  delay(500);  // Adjust delay as needed
}

// Bluetooth command handler for direct rolling commands
void BluetoothCommandHandler(String command) {
    
  if (command == "home") {
    NinjaHome();
  } else if (command == "d") {
    sendDistance();
  } else if (command == "start walking") {
    isWalking = true;
    NinjaWalk();
  } else if(isWalking && command == "walk fast") {
    walking_speed = 0.3;
    NinjaWalk();
  } else if(isWalking && command == "walk slow") {
    walking_speed = 1.5;
    NinjaWalk();
  } else if (command == "stop walking") {
    isWalking = false;
    NinjaRollStop();
  } else if (command == "forward") {
    displayStatus("Rolling Forward");
    NinjaRollForward();
  } else if (command == "backward") {
    displayStatus("Rolling Backward");
    NinjaRollBackward();
  } else if (command == "turn left") {
    displayStatus("Turning Left");
    NinjaTurnLeft();
  } else if (command == "turn right") {
    displayStatus("Turning Right");
    NinjaTurnRight();
  } else if (command == "stop") {
    displayStatus("Stopped");
    NinjaRollStop();
  } else {
    displayStatus("Unknown Command");
  }
}

// Function to clear the Bluetooth buffer
void clearBluetoothBuffer() {
  while (Serial.available()) {
    Serial.read();  // Clear all remaining data in the Serial buffer
  }
}

// Function to initialize all servos to their home positions
void NinjaHome() {
  displayStatus("Hi, I'm Otto!");
  myservoLeftLeg.attach(servo_left_ankle_pin);
  myservoRightLeg.attach(servo_right_ankle_pin);
  myservoLeftFoot.attach(servo_left_wheel_pin);
  myservoRightFoot.attach(servo_right_wheel_pin);
  myservoHead.attach(servo_head_pin);

  myservoLeftLeg.write(80);
  myservoRightLeg.write(150);
  myservoLeftFoot.write(90);
  myservoRightFoot.write(90);
  myservoHead.write(90);

  delay(1000);  // Stabilize
}

// Function for walking
void NinjaWalk() {
  displayStatus("Getting my steps in!");
  myservoLeftLeg.write(50);   
  myservoRightLeg.write(150); 
  delay(200 * walking_speed);

  myservoLeftFoot.write(150);
  delay(150 * walking_speed);
  myservoLeftFoot.write(90);
  delay(150 * walking_speed);

  myservoLeftLeg.write(80);
  delay(200 * walking_speed);

  myservoRightLeg.write(180);
  myservoLeftLeg.write(80);
  delay(200 * walking_speed);

  myservoRightFoot.write(30);
  delay(150 * walking_speed);
  myservoRightFoot.write(90);
  delay(150 * walking_speed);

  myservoRightLeg.write(150);
  delay(200 * walking_speed);
}

// Function to roll forward
void NinjaRollForward() {
  myservoLeftLeg.write(180);
  myservoRightLeg.write(40);
  delay(300); // to stablise
  myservoLeftFoot.write(0);
  myservoRightFoot.write(180);
}

// Function to roll backward
void NinjaRollBackward() {
  myservoLeftLeg.write(180);
  myservoRightLeg.write(40);
  delay(300); // to stablise
  myservoLeftFoot.write(180);
  myservoRightFoot.write(0);
}

// Function to stop rolling
void NinjaRollStop() {
  myservoLeftFoot.write(90);
  myservoRightFoot.write(90);
}

// Function to turn left
void NinjaTurnLeft() {
  myservoLeftLeg.write(180);
  myservoRightLeg.write(40);
  delay(300); // to stablise
  myservoLeftFoot.write(90);
  myservoRightFoot.write(180);
}

// Function to turn right
void NinjaTurnRight() {
  myservoLeftLeg.write(180);
  myservoRightLeg.write(40);
  delay(300); // to stablise
  myservoRightFoot.write(90);
  myservoLeftFoot.write(0);
}

// Function to display text on OLED
void displayStatus(const char *message) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  int x_pos = (128 - u8g2.getStrWidth(message)) / 2;
  u8g2.drawStr(x_pos, 32, message);
  u8g2.sendBuffer();
}

// Function to read ultrasonic distance
int readUltrasonic() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  return duration * 0.034 / 2;
}

// Function to send distance data to Bluetooth device
void sendDistance() {
  int distance = readUltrasonic();
  Serial.print("*D");
  Serial.print(distance);
  Serial.println("*");
}

// Function to send walking speed data to Bluetooth device
void sendWalkingSpeed() {
  Serial.print("*W");
  Serial.print(walking_speed);
  Serial.println("*");
}
