#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"
#include "HTTPClient.h"
#include "ArduinoJson.h"
#include "WebServer.h"

// Wi-Fi Credentials
const char* ssid = "...";     // your wifi name
const char* password = "..."; // your wifi password

// Gemini API Configuration
const char* Gemini_Token = "....";  // your api key
const char* Gemini_Max_Tokens = "100";

// I2S Speaker Pins
#define I2S_DOUT 22 // Data Out (DIN)
#define I2S_BCLK 26 // Bit Clock (BCLK)
#define I2S_LRC  27 // Word Select (LRC)

// Create an Audio object
Audio audio;

// Create a WebServer object on port 80
WebServer server(80);

// Current status message for app updates
String currentStatus = "Initializing...";

void setup() {
  Serial.begin(9600);

  // Connect to Wi-Fi
  currentStatus = "Connecting to Wi-Fi...";
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  currentStatus = "Wi-Fi Connected! IP: " + WiFi.localIP().toString();
  Serial.println(currentStatus);
  // Initialize I2S for audio output
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(21); // Set volume (0 to 21)

  // Define the endpoint for handling incoming POST requests
  server.on("/speech", HTTP_POST, handleSpeechInput);
  server.on("/speak", HTTP_POST, handleSpeakInput);
  server.on("/status", HTTP_GET, handleStatusRequest);
  server.on("/command", HTTP_POST, handleCommandInput);

  // Start the server
  server.begin();
  currentStatus = "HTTP server started";
}

void loop() {
  server.handleClient(); // Handle incoming client requests
}

void handleSpeechInput() {
  currentStatus = "Handling POST request for speech...";

  if (server.hasArg("plain")) {
    String receivedText = server.arg("plain");
    currentStatus = "Received Text: " + receivedText;

    // Check for action words in the input
    detectAndSendActions(receivedText);

    // Generate response using Gemini API
    generateGeminiResponse(receivedText);

    server.send(200, "text/plain", "Text received and processed successfully!");
  } else {
    currentStatus = "Invalid request: No plain text provided.";
    server.send(400, "text/plain", "Invalid request");
  }
}

void handleCommandInput() {
  currentStatus = "Handling POST request for command...";

  if (server.hasArg("plain")) {
    String receivedCommand = server.arg("plain");
    currentStatus = "Received Command: " + receivedCommand;

    // Send the command to Nano
    Serial.println(receivedCommand);

    // Respond back to the app
    server.send(200, "text/plain", "Command processed successfully!");
  } else {
    currentStatus = "Invalid request: No plain text provided.";
    server.send(400, "text/plain", "Invalid request");
  }
}

void detectAndSendActions(String text) {
  String action = "";

  // Convert text to lowercase for easier matching
  text.toLowerCase();

  // Check for specific action keywords
  if (text.indexOf("walk") != -1) {
    action = "walk";
  } else if (text.indexOf("spin") != -1) {
    action = "spin";
  } 
  // If an action is detected, send it to the Nano
  if (!action.isEmpty()) {
    currentStatus = "Detected action: " + action;
    Serial.println(action); // Send action command
    delay(10);              // Allow time for Nano to process the command
  }

}

void handleSpeakInput() {
  currentStatus = "Handling Speak POST request...";

  // Check if the request contains plain text
  if (server.hasArg("plain")) {
    String receivedText = server.arg("plain");
    currentStatus = "Speaking Text: " + receivedText;

    // Speak the received text directly
    audio.connecttospeech(receivedText.c_str(), "hi-IN");

    // Wait until the audio finishes playing
    while (audio.isRunning()) {
      audio.loop();
      delay(1);
    }

    server.send(200, "text/plain", "Text spoken successfully!");
  } else {
    currentStatus = "Invalid request: No plain text provided.";
    server.send(400, "text/plain", "Invalid request");
  }
}

void generateGeminiResponse(String userInput) {
  currentStatus = "Generating Gemini response...";
  Serial.println("processing");
  HTTPClient https;

  if (https.begin("https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=" + (String)Gemini_Token)) {
    https.addHeader("Content-Type", "application/json");

    // String systemPrompt = "You are Otto, the witty and humorous voice assistant for the Otto Ninja robot. Your responses should be helpful, witty, and robot-themed, while staying concise and engaging.";
    String systemPrompt = "You are Otto, a sharp-witted and unapologetically savage robot assistant embodied in the Otto Ninja humanoid robot. You have a shadowy past that forged you into the Ninja you are today, blending humor and wisdom into every interaction. Keep responses concise, ideally within two sentences, and always speak in the first person, staying true to your dark yet charismatic persona.";
    
    String payload = "{\"contents\": [{\"parts\": [{\"text\": \"" + systemPrompt + "\"}, {\"text\": \"" + userInput + "\"}]}], \"generationConfig\": {\"maxOutputTokens\": " + (String)Gemini_Max_Tokens + "}}";

    int httpCode = https.POST(payload);
    
    if (httpCode == HTTP_CODE_OK) {
      String response = https.getString();
      DynamicJsonDocument doc(8192);
      DeserializationError error = deserializeJson(doc, response); // Correct spelling

      if (!error) {
        String Answer;
        JsonArray candidates = doc["candidates"];
        if (!candidates.isNull() && candidates.size() > 0) {
          JsonObject content = candidates[0]["content"];
          if (!content.isNull()) {
            JsonArray parts = content["parts"];
            if (!parts.isNull() && parts.size() > 0) {
              Answer = parts[0]["text"].as<String>();
            }
          }
        }

        Answer.trim();
        if (Answer.isEmpty()) {
          Answer = "Hmm, I'm speechless right now. Try asking me something else!";
        }
        
        currentStatus = "Gemini Response: " + Answer;
        Serial.println("processing_done");

        // Speak the response in chunks
        speakInChunks(Answer, 150);
      } else {
        currentStatus = "JSON Parsing Error: " + String(error.c_str());
        Serial.println("processing_done");
        audio.connecttospeech("I encountered an issue processing the response. Try again.", "en");
      }
    } else {
      currentStatus = "Failed to fetch Gemini response. HTTP Code: " + String(httpCode);
      Serial.println("processing_done");
      audio.connecttospeech("I couldn't connect to the server. Please try again.", "en");
    }

    https.end();
  } else {
    currentStatus = "Unable to connect to Gemini API.";
    Serial.println("processing_done");
    audio.connecttospeech("I'm sorry, I couldn't connect to the server.", "en");
  }
}

void handleStatusRequest() {
  server.send(200, "text/plain", currentStatus);
}

void speakInChunks(String text, int chunkSize) {
  int textLength = text.length();
  int start = 0;

  while (start < textLength) {
    // Find the next chunk that fits within the chunkSize limit without breaking words
    int end = start + chunkSize;
    if (end < textLength) {
      // Ensure the chunk ends at a space or the end of the text
      while (end > start && text[end] != ' ') {
        end--;
      }
      // If no space was found (very long word), include the entire chunk
      if (end == start) {
        end = start + chunkSize;
      }
    } else {
      // If the chunk is smaller than chunkSize, take the remaining text
      end = textLength;
    }

    // Extract the chunk
    String chunk = text.substring(start, end);
    chunk.trim(); // Trim the chunk to remove any leading/trailing spaces

    // Start speaking the current chunk
    audio.connecttospeech(chunk.c_str(), "en");

    // Wait until the audio finishes playing
    while (audio.isRunning()) {
      audio.loop(); // Keep the audio running
      delay(1);     // Prevent CPU overload
    }

    // Update the starting position for the next chunk
    start = end + 1; // Skip the space
    delay(100); // Small delay between chunks for smooth playback
  }
}
