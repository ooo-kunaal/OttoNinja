#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"
#include "HTTPClient.h"
#include "ArduinoJson.h"
#include "WebServer.h"
#include <ESP32Servo.h>

// Wi-Fi Credentials
const char* ssid = "SHELL-4474B8";
const char* password = "jdvPtbK8HBhp";

// Gemini API Configuration
const char* Gemini_Token = "AIzaSyB888hU8v-aiG5m8jzodHS7JZ0Dgn0DFpI";
const char* Gemini_Max_Tokens = "25";

// Define Servo Object
Servo rightFootServo;
const int rightFootPin = 13;  // Right Foot Servo

// I2S Speaker Pins
#define I2S_DOUT 25  // Data Out (DIN)
#define I2S_BCLK 26  // Bit Clock (BCLK)
#define I2S_LRC  27  // Word Select (LRC)

// Create an Audio object
Audio audio;

// Create a WebServer object on port 80
WebServer server(80);

// Current status message for app updates
String currentStatus = "Initializing...";

// Task handle for the think function
TaskHandle_t thinkTaskHandle = NULL;

void setup() {
  Serial.begin(9600);

  // Connect to Wi-Fi
  connectToWiFi();

  // Initialize audio output
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(18);

  // Set up HTTP server endpoints
  setupHTTPServer();

  // Attach servo
  rightFootServo.attach(rightFootPin);

  // Set servo to default position
  NinjaHome();

  Serial.println("Setup Complete!");
}

void loop() {
  server.handleClient();  // Handle incoming client requests
}

// Function to connect to Wi-Fi
void connectToWiFi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Connected!");
  Serial.println("IP Address: " + WiFi.localIP().toString());
  currentStatus = "Wi-Fi Connected! IP: " + WiFi.localIP().toString();
}

// Function to set up the HTTP server
void setupHTTPServer() {
  server.on("/speech", HTTP_POST, handleSpeechInput);
  server.on("/speak", HTTP_POST, handleSpeakInput);
  server.on("/status", HTTP_GET, handleStatusRequest);
  server.begin();
  currentStatus = "HTTP server started";
}

// Function to reset the servo position
void NinjaHome() {
  rightFootServo.write(0);  // Neutral position
}

// Thinking task that runs in parallel
void thinkTask(void* parameter) {
  while (true) {
    rightFootServo.write(30);  // Bend leg
    delay(300);
    rightFootServo.write(0);   // Back to neutral
    delay(300);
  }
}

// Start the leg-tapping action
void startThinking() {
  if (thinkTaskHandle == NULL) {
    xTaskCreatePinnedToCore(
      thinkTask,      // Task function
      "ThinkTask",    // Task name
      1000,           // Stack size
      NULL,           // Parameters
      1,              // Priority
      &thinkTaskHandle, // Task handle
      1               // Run on core 1
    );
  }
}

// Stop the leg-tapping action
void stopThinking() {
  if (thinkTaskHandle != NULL) {
    vTaskDelete(thinkTaskHandle);
    thinkTaskHandle = NULL;
    NinjaHome();  // Reset servo to default
  }
}

// Handle POST requests for speech input
void handleSpeechInput() {
  if (server.hasArg("plain")) {
    String receivedText = server.arg("plain");
    Serial.println("Received Text: " + receivedText);

    generateGeminiResponse(receivedText);

    server.send(200, "text/plain", "Text received and processed successfully!");
  } else {
    server.send(400, "text/plain", "Invalid request: No plain text provided.");
  }
}

// Handle POST requests for speaking text
void handleSpeakInput() {
  if (server.hasArg("plain")) {
    String receivedText = server.arg("plain");
    speakInChunks(receivedText, 80);

    server.send(200, "text/plain", "Text spoken successfully!");
  } else {
    server.send(400, "text/plain", "Invalid request: No plain text provided.");
  }
}

// Fetch and speak a response from Gemini API
void generateGeminiResponse(String userInput) {
  startThinking();  // Start leg tapping while thinking

  HTTPClient https;

  if (https.begin("https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=" + (String)Gemini_Token)) {
    https.addHeader("Content-Type", "application/json");
    
    String systemPrompt = "You are Otto, a witty and concise assistant. Keep responses dark & unapologetically savage.";
    String payload = "{\"contents\": [{\"parts\": [{\"text\": \"" + systemPrompt + "\"}, {\"text\": \"" + userInput + "\"}]}], \"generationConfig\": {\"maxOutputTokens\": " + (String)Gemini_Max_Tokens + "}}";

    int httpCode = https.POST(payload);

    if (httpCode == HTTP_CODE_OK) {
      String response = https.getString();
      DynamicJsonDocument doc(4096);
      if (deserializeJson(doc, response) == DeserializationError::Ok) {
        String answer = doc["candidates"][0]["content"]["parts"][0]["text"].as<String>();
        Serial.println("Answer: " + answer);

        stopThinking();  // Stop leg tapping
        speakInChunks(answer, 60);
      } else {
        stopThinking();
        audio.connecttospeech("Error processing response.", "en");
      }
    } else {
      stopThinking();
      audio.connecttospeech("Failed to fetch response.", "en");
    }

    https.end();
  } else {
    stopThinking();
    audio.connecttospeech("Unable to connect to server.", "en");
  }
}

// Handle GET requests for status
void handleStatusRequest() {
  server.send(200, "text/plain", currentStatus);
}

// Speak text in chunks for smooth playback
void speakInChunks(String text, int chunkSize) {
  size_t textLength = text.length();
  size_t start = 0;

  while (start < textLength) {
    size_t end = start + chunkSize;

    // Ensure we don't split words awkwardly
    if (end < textLength) {
      while (end > start && text[end] != ' ' && text[end] != '.' && text[end] != ',' && text[end] != '!' && text[end] != '?') {
        end--;
      }
    } else {
      end = textLength;  // Capture the final part of the text
    }

    String chunk = text.substring(start, end);
    chunk.trim();

    // Speak the current chunk
    audio.connecttospeech(chunk.c_str(), "en");

    // Ensure the audio finishes playing
    while (audio.isRunning()) {
      audio.loop();
      delay(10);  // Reduced delay for more responsive playback
    }

    start = end + 1;  // Move to the next chunk
    delay(100);       // Small pause between chunks
  }

  // Ensure any remaining text is spoken
  if (start < textLength) {
    String lastChunk = text.substring(start);
    lastChunk.trim();

    if (lastChunk.length() > 0) {
      audio.connecttospeech(lastChunk.c_str(), "en");

      while (audio.isRunning()) {
        audio.loop();
        delay(10);
      }
    }
  }
}

