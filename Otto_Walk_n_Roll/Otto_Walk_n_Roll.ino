// Include necessary libraries
#include <Wire.h>
#include <U8g2lib.h>
#include <Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

// Function prototypes
void NinjaHome();
void displayStatus(const char *message);
int readUltrasonic();
void NinjaRollForward();
void NinjaRollBackward();
void NinjaRollStop();
void NinjaTurnLeft();
void NinjaTurnRight();
int scanLeft();
int scanRight();

// OLED Display Initialization
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);

// Create the matrix display object
Adafruit_8x16matrix matrix = Adafruit_8x16matrix();

// Servo Definitions
Servo myservoLeftFoot, myservoRightFoot, myservoLeftLeg, myservoRightLeg;
Servo myservoLeftHand, myservoRightHand, myservoHead;

// Ultrasonic Sensor Pins
#define echoPin 5  // Echo pin of ultrasonic sensor (D5)
#define trigPin 4  // Trig pin of ultrasonic sensor (D4)
long duration;
int distance;

// Walking speed multiplier (adjust to control speed)
float walking_speed = 0.75;  // Set this value between 0.1 (very fast) and 2 (very slow)

// Servo Pin Mappings
const uint8_t servo_right_wheel_pin = A1;  // Right Wheel (Foot) servo on A0
const uint8_t servo_right_ankle_pin = A0;  // Right Ankle servo on A1
const uint8_t servo_right_hand_pin = A2;   // Right Hand servo on A2
const uint8_t servo_head_pin = 8;          // Head servo on pin 8
const uint8_t servo_left_hand_pin = 9;     // Left Hand servo on pin 9
const uint8_t servo_left_wheel_pin = 10;   // Left Wheel (Foot) servo on pin 11
const uint8_t servo_left_ankle_pin = 11;   // Left Ankle servo on pin 10

void setup() {
  // Initialize Serial, OLED, and Ultrasonic Sensor
  Serial.begin(115200);
  u8g2.begin();
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Initialize servos to home position
  NinjaHome();
  delay(1000);
  // // Start moving forward
  // NinjaRollForward();
}

void loop() {
  NinjaWalk();

  // Read distance from ultrasonic sensor
  distance = readUltrasonic();
  
  // if (distance >= 15) {
  //   displayStatus("Moving");
  //   NinjaRollForward();  // Keep rolling forward if no obstacle
  // } else 
  if (distance < 15 && distance >= 2) {
    displayStatus("Obstacle detected!");
    NinjaRollStop();     // Stop rolling
    delay(300); // to stablise
    // Scan left and right to decide the direction
    int distanceLeft = scanLeft();
    int distanceRight = scanRight();
    
    // Compare distances and turn in the best direction
    if (distanceLeft > distanceRight) {
      delay(300); // to stablise
      NinjaTurnLeft();
      delay(300); // to stablise
    } else {
      delay(300); // to stablise
      NinjaTurnRight();
      delay(300); // to stablise
    }

  }
}

// Function to initialize all servos to their home positions
void NinjaHome() {
  displayStatus("Hi, I'm Otto!");

  // Attach all servos
  myservoLeftLeg.attach(servo_left_ankle_pin);    
  myservoRightLeg.attach(servo_right_ankle_pin);   
  myservoLeftFoot.attach(servo_left_wheel_pin);   
  myservoRightFoot.attach(servo_right_wheel_pin);  
  myservoLeftHand.attach(servo_left_hand_pin);    
  myservoRightHand.attach(servo_right_hand_pin);  
  myservoHead.attach(servo_head_pin);        

  // Set initial positions
  myservoLeftLeg.write(80);
  myservoRightLeg.write(150);

  myservoLeftFoot.write(90);
  myservoRightFoot.write(90);

  myservoLeftHand.write(90);
  myservoRightHand.write(90);
  
  myservoHead.write(90);

  delay(1000); // small pause to stabilise
}

// Function for walking
void NinjaWalk() {
  displayStatus("Getting my steps in!");

  // Step 1: Point Left toe - shift weight on right leg
  myservoLeftLeg.write(50);   // Left leg supports weight
  myservoRightLeg.write(150); // Right leg slightly lifted to step forward
  delay(200 * walking_speed); // Adjusted delay with walking speed multiplier

  // Step 2: Move the left foot forward
  myservoLeftFoot.write(150); // left foot rotates forward
  delay(150 * walking_speed); // Adjusted delay
  myservoLeftFoot.write(90);  // stop rotation
  delay(150 * walking_speed); // Adjusted delay

  // Step 3: Normalize left leg, point right toe
  myservoLeftLeg.write(80);    // left leg normalize
  delay(200 * walking_speed); // Adjusted delay

  // Step 4: Point right toe - shift weight on left leg
  myservoRightLeg.write(180);  // point right toe
  myservoLeftLeg.write(80);    // left leg supports weight
  delay(200 * walking_speed); // Adjusted delay

  // Step 5: Move the right foot forward
  myservoRightFoot.write(30); // right foot rotates forward
  delay(150 * walking_speed); // Adjusted delay
  myservoRightFoot.write(90);  // right foot back to neutral
  delay(150 * walking_speed); // Adjusted delay

  // Step 6: Normalize right leg
  myservoRightLeg.write(150);  // Right leg normalize
  delay(200 * walking_speed); // Adjusted delay
}

// Function to roll forward
void NinjaRollForward() {
  displayStatus("Roll forward!");
  myservoLeftLeg.write(180);
  myservoRightLeg.write(40);
  myservoLeftFoot.write(0);
  myservoRightFoot.write(180);
}

// Function to roll backward
void NinjaRollBackward() {
  displayStatus("Fall back!");
  myservoLeftLeg.write(180);
  myservoRightLeg.write(40);
  myservoLeftFoot.write(180);
  myservoRightFoot.write(0);
}

// Function to stop rolling
void NinjaRollStop() {
  displayStatus("Halt!");
  myservoLeftLeg.write(80);
  myservoRightLeg.write(150);
  myservoLeftFoot.write(90);
  myservoRightFoot.write(90);
}

// Function to turn left
void NinjaTurnLeft() {
  displayStatus("Turning Left");
  myservoLeftLeg.write(180);
  myservoRightLeg.write(40);
  delay(300); // to stablise
  myservoLeftFoot.write(90);  // Stop left foot
  myservoRightFoot.write(180); // Move right foot backwards
  delay(1000); // Adjust the time as per the turn
}

// Function to turn right
void NinjaTurnRight() {
  displayStatus("Turning Right");
  myservoLeftLeg.write(180);
  myservoRightLeg.write(40);
  delay(300); // to stablise
  myservoRightFoot.write(90);  // Stop right foot
  myservoLeftFoot.write(0);    // Move left foot backwards
  delay(1000); // Adjust the time as per the turn
}

// Function to scan left
int scanLeft() {
  myservoHead.write(135);  // Rotate head to left
  delay(500);
  int distanceLeft = readUltrasonic();  // Read distance on the left
  myservoHead.write(90);  // Return head to the center
  return distanceLeft;
}

// Function to scan right
int scanRight() {
  myservoHead.write(45);  // Rotate head to right
  delay(500);
  int distanceRight = readUltrasonic();  // Read distance on the right
  myservoHead.write(90);  // Return head to the center
  return distanceRight;
}

// Read distance using ultrasonic sensor
int readUltrasonic() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  return duration * 0.034 / 2;
}

// Function to display text on OLED
void displayStatus(const char *message) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  int x_pos = (128 - u8g2.getStrWidth(message)) / 2;
  u8g2.drawStr(x_pos, 32, message);
  u8g2.sendBuffer();
}

