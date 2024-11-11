// Include necessary libraries
#include <Wire.h>
#include <Servo.h>
#include <U8g2lib.h>

// Function prototypes
void NinjaHome();
void NinjaWalk();
void NinjaRollStop();
void NinjaTurnLeft();
void NinjaTurnRight();
void NinjaRollForward();
void NinjaRollBackward();
void displayStatus(const char *message);

void Step_one();
void Step_two();
void TapRightLeg(int tempo);
void TapLeftLeg(int tempo);
void tapDance(int repeats, int tempo);
void Dance();

void sendDistance();
void clearBluetoothBuffer();

// OLED Display Initialization
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Servo Definitions
Servo myservoLeftFoot, myservoRightFoot, myservoLeftLeg, myservoRightLeg, myservoHead;

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

void setup() {
  Serial.begin(9600);
  // Initialize the OLED display
  u8g2.begin();

  // Display a welcome message
  u8g2.clearBuffer();                      // Clear internal buffer

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  NinjaHome();
}

void loop() {
  // Check for Bluetooth commands
  if (Serial.available()) {
    char command = Serial.read();
    BluetoothCommandHandler(command);
    clearBluetoothBuffer();
  }
  
  // If in walking mode, the robot should walk autonomously
  if (isWalking) {
    NinjaWalk();
  }
  
  // Send distance data periodically
  sendDistance();
  delay(100);
}

// Bluetooth command handler
void BluetoothCommandHandler(char command) {
  switch (command) {
    case 'H':  // Home position
      NinjaHome();
      break;
    case 'P':  // Dance!
      Dance();
      break;
    case 'p':  // Home position
      NinjaHome();
      break;
    case 'O':  // Stop
      NinjaRollStop();
      break;
    case 'D':  // Send Distance
      sendDistance();
      break;
    case 'A':  // Walk
      isWalking = true;
      NinjaWalk();
      break;
    case 'a':  // Stop Walking
      isWalking = false;
      NinjaHome();
      break;
    case 'F':  // Roll forward
      NinjaRollForward();
      break;
    case 'B':  // Roll backward
      NinjaRollBackward();
      break;
    case 'L':  // Turn left
      NinjaTurnLeft();
      break;
    case 'R':  // Turn right
      NinjaTurnRight();
      break;
    case 'S':  // Stop rolling
      NinjaRollStop();
      break;
    default:
      break;
  }
}

// Function to clear the Bluetooth buffer
void clearBluetoothBuffer() {
  while (Serial.available()) {
    Serial.read();
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
  displayStatus("Roll forward!");
  myservoLeftLeg.write(180);
  delay(300); // to stablise
  myservoRightLeg.write(40);
  delay(300); // to stablise
  myservoLeftFoot.write(0);
  myservoRightFoot.write(180);
}

// Function to roll backward
void NinjaRollBackward() {
  displayStatus("Fall back!");  
  myservoLeftLeg.write(180);
  delay(300); // to stablise
  myservoRightLeg.write(40);
  delay(300); // to stablise
  myservoLeftFoot.write(180);
  myservoRightFoot.write(0);
}

// Function to stop rolling
void NinjaRollStop() {
  displayStatus("Halt!");
  myservoLeftFoot.write(90);
  myservoRightFoot.write(90);
}

// Function to turn left
void NinjaTurnLeft() {
  displayStatus("Turning Left");
  myservoLeftLeg.write(180);
  delay(300); // to stablise
  myservoRightLeg.write(40);
  delay(300); // to stablise
  myservoLeftFoot.write(90);
  myservoRightFoot.write(180);
}

// Function to turn right
void NinjaTurnRight() {
  displayStatus("Turning Right");
  myservoLeftLeg.write(180);
  delay(300); // to stablise
  myservoRightLeg.write(40);
  delay(300); // to stablise
  myservoRightFoot.write(90);
  myservoLeftFoot.write(0);
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
  
  Serial.println(distance);
  Serial.print("*D");
  Serial.print(distance);
  Serial.println("*");
}

// Function for Moonwalk Left
void Step_one() {
  displayStatus("Let's tap some toes!");
  for (int i = 0; i < 4; i++) {
    // Step 1: Lift and shift weight to the right leg
    myservoLeftLeg.write(50);   // Point left toe down
    delay(100);
    myservoRightLeg.write(180); // Slightly lift right leg
    delay(100);
    
    // Step 2: Move left leg back (simulating glide) and bring right leg higher
    myservoLeftLeg.write(65);   // Partially raise left leg
    delay(100);
    myservoRightLeg.write(170); // Continue raising right leg
    delay(100);

    // Step 3: Rest left leg, point right toe fully
    myservoLeftLeg.write(80);   // Lower left leg (rest position)
    delay(100);
    myservoRightLeg.write(180); // Point right toe (glide position)
    delay(100);

    // Step 4: Glide left leg, rest right leg
    myservoLeftLeg.write(90);   // Move left leg forward (simulating glide)
    delay(100);
    myservoRightLeg.write(150); // Lower right leg to rest
    delay(100);
  }
  myservoLeftLeg.write(80);
  myservoRightLeg.write(150);
}

// spin around
void Step_two() {
  displayStatus("Spinnn!");
  myservoLeftLeg.write(180);
  delay(300); // to stablise
  myservoRightLeg.write(40);
  delay(300); // to stablise
  myservoLeftFoot.write(0);    // Spin left foot backwards
  myservoRightFoot.write(0);   // Spin right foot forward
  delay(1000); // Adjust the time as per the turn
  myservoLeftFoot.write(90);
  myservoRightFoot.write(90);
  myservoLeftLeg.write(40);   // Left leg supports weight
  myservoRightLeg.write(150); // Right leg slightly lifted to step forward
}

// Tap Right Leg function with rhythmic pause
void TapRightLeg(int tempo) {
  myservoRightLeg.write(100);    // Lower right leg for tap
  delay(tempo * 1.5);            // Slightly longer hold for emphasis
  myservoRightLeg.write(150);    // Return right leg to neutral
  delay(tempo / 3);              // Small pause after lifting
}

// Tap Left Leg function with rhythmic pause
void TapLeftLeg(int tempo) {
  myservoLeftLeg.write(120);     // Lower left leg for tap
  delay(tempo * 1.2);            // Hold the tap briefly
  myservoLeftLeg.write(80);      // Return left leg to neutral
  delay(tempo / 2);              // Small pause after lifting
}

// Function for Rhythmic Tap Dance with Varied Sequence
void tapDance(int repeats, int tempo) {
  displayStatus("Tap Tap Tap!");
  for (int i = 0; i < repeats; i++) {
    // Pattern 1: Single Left Tap with a Pause
    TapLeftLeg(tempo);
    delay(tempo / 2);  // Short pause for emphasis

    // Pattern 2: Single Right Tap, Quick Left Tap, Pause
    TapRightLeg(tempo);
    delay(tempo / 4);
    TapLeftLeg(tempo);
    delay(tempo);  // Longer pause for rhythm reset

    // Pattern 3: Double Right Tap for Shuffle Effect
    TapRightLeg(tempo / 2);
    delay(tempo / 4);  // Quick pause between double tap
    TapRightLeg(tempo / 2);
    delay(tempo / 2);

    // Pattern 4: Left Tap, Right Tap, Left Tap (syncopated beat)
    TapLeftLeg(tempo / 1.5);
    delay(tempo / 4);
    TapRightLeg(tempo / 1.5);
    delay(tempo / 4);
    TapLeftLeg(tempo / 1.5);
    delay(tempo * 0.75);  // Slightly longer pause for beat sync

    // Break after each cycle
    if (i % 2 == 1) {
      delay(tempo * 1.5);  // Extra pause every two cycles for rhythm variation
    }
  }

  // Reset legs to initial position after the dance
  myservoLeftLeg.write(80);
  myservoRightLeg.write(150);
}

void Dance() {
  displayStatus("Let's Dance!");
  delay(100);
  Step_one();
  delay(1000);
  Step_two();
  delay(1000);
  tapDance(3, 250);
  delay(1000);
}

// Function to display text on OLED
void displayStatus(const char *message) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  int x_pos = (128 - u8g2.getStrWidth(message)) / 2;
  u8g2.drawStr(x_pos, 32, message);
  u8g2.sendBuffer();
}