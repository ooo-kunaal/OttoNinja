// Include necessary libraries
#include <Servo.h>
#include <SoftwareSerial.h>

// Servo Definitions
Servo myservoLeftFoot, myservoRightFoot, myservoLeftLeg, myservoRightLeg;
Servo myservoLeftHand, myservoRightHand, myservoHead;

SoftwareSerial espSerial(2, 3); // RX, TX

// Walking speed multiplier
float walking_speed = 0.75;  // Speed adjustment: 0.1 (fast) to 2 (slow)

// Walking state
bool isWalking = false;
bool isProcessing = false;

// Servo Pin Mappings
const uint8_t servo_right_wheel_pin = A0;
const uint8_t servo_right_ankle_pin = A1;
const uint8_t servo_right_hand_pin = A2;
const uint8_t servo_head_pin = A6;
const uint8_t servo_left_hand_pin = A5;
const uint8_t servo_left_wheel_pin = A3;
const uint8_t servo_left_ankle_pin = A4;

// Function prototypes
void NinjaHome();
void NinjaWalkStep();
void Spin();
void think();

void setup() {
  Serial.begin(9600);      // For debugging
  espSerial.begin(9600);   // For communication with ESP32
  Serial.println("Arduino Nano Ready!");

  // attach servos
  myservoLeftLeg.attach(servo_left_ankle_pin);
  myservoRightLeg.attach(servo_right_ankle_pin);
  myservoLeftFoot.attach(servo_left_wheel_pin);
  myservoRightFoot.attach(servo_right_wheel_pin);
  myservoLeftHand.attach(servo_left_hand_pin);
  myservoRightHand.attach(servo_right_hand_pin);
  myservoHead.attach(servo_head_pin);

  // Set servos to home position
  NinjaHome();
  delay(1000);
}

void loop() {
  if (espSerial.available()) {
    String command = espSerial.readStringUntil('\n');
    command.trim();  // Remove any trailing spaces or line breaks

    Serial.print("Received Command by Nano: [");
    Serial.print(command);
    Serial.println("]");

    if (command == "spin") {
      Serial.println("Command matched: spin");
      isWalking = false;
      Spin();
    } else if (command == "walk") {
      Serial.println("Command matched: walk");
      isWalking = true;
    } else if (command == "stop") {
      Serial.println("Command matched: stop");
      isWalking = false;
      NinjaHome();
    } else if (command == "processing") {
      Serial.println("Command matched: processing");
      isProcessing = true;
    } else if (command == "processing_done") {
      Serial.println("Command matched: processing done");
      isProcessing = false;
    }
  }

  // Perform continuous walking if in walking state
  if (isWalking) {
    NinjaWalkStep();
  }
  if (isProcessing) {
    think();
    delay(100);
  }
}

// Home all servos to initial positions
void NinjaHome() {
  myservoLeftLeg.write(80);
  myservoRightLeg.write(150);
  myservoLeftFoot.write(90);
  myservoRightFoot.write(90);
  myservoLeftHand.write(90);
  myservoRightHand.write(90);
  myservoHead.write(90);

  delay(1000);
}

// Perform a single step of walking
void NinjaWalkStep() {
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
  delay(200 * walking_speed);

  myservoRightFoot.write(30);
  delay(150 * walking_speed);
  myservoRightFoot.write(90);
  delay(150 * walking_speed);

  myservoRightLeg.write(150);
  delay(200 * walking_speed);
}

// Perform a spin motion
void Spin() {
  Serial.println("Spinning...");
  myservoLeftLeg.write(180);
  delay(300); // to stablise
  myservoRightLeg.write(40);
  delay(300); // to stablise
  myservoLeftFoot.write(0);    // Spin left foot backwards
  myservoRightFoot.write(0);   // Spin right foot forward
  delay(1000); // Adjust the time as per the turn
  myservoLeftFoot.write(90);
  myservoRightFoot.write(90);
  myservoLeftLeg.write(80);
  myservoRightLeg.write(150); // Right leg slightly lifted to step forward
  
  NinjaHome(); // Reset position after spin
}

// look sideways
void think() {
  myservoRightLeg.write(130);    // Lower right leg for tap
  delay(150);            // Slightly longer hold for emphasis
  myservoRightLeg.write(150);    // Return right leg to neutral
  delay(100);              // Small pause after lifting
}
