#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED Display Object
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// Button pins
const int buttonUpPin = 12;
const int buttonDownPin = 14;
int buttonUpState = 0;
int buttonDownState = 0;

// Put your WiFi Credentials here
const char* ssid = "Droid O'clock ";
const char* password = "2024_Kenya";

// URL Endpoint for the API
String URL = "https://bible-api.com/?random=verse";

String verseText;
String verseReference;
int scrollOffset = 0;
int maxScrollOffset = 0;

void setup() {
  Serial.begin(115200);

  // Setup the OLED Display and initialize: address => 0x3C or 0x3D
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  // Setup buttons
  pinMode(buttonUpPin, INPUT_PULLUP);
  pinMode(buttonDownPin, INPUT_PULLUP);

  // We start by connecting to a WiFi network
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  fetchVerse();
}

void loop() {
  // Read button states
  buttonUpState = digitalRead(buttonUpPin);
  buttonDownState = digitalRead(buttonDownPin);

  // Check if the up button is pressed
  if (buttonUpState == LOW) {
    if (scrollOffset > 0) {
      scrollOffset -= 8; // Adjust scroll speed as needed
    }
    displayVerse();
    delay(200); // Debounce delay
  }

  // Check if the down button is pressed
  if (buttonDownState == LOW) {
    if (scrollOffset < maxScrollOffset) {
      scrollOffset += 8; // Adjust scroll speed as needed
    }
    displayVerse();
    delay(200); // Debounce delay
  }

  delay(100);
}

void fetchVerse() {
  // Wait for WiFi connection
  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;

    // Set HTTP Request Final URL
    http.begin(URL);

    // Start connection and send HTTP Request
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {

      // Read Data as a JSON string
      String JSON_Data = http.getString();
      Serial.println(JSON_Data);

      // Parse the JSON data
      DynamicJsonDocument doc(2048);
      deserializeJson(doc, JSON_Data);
      JsonObject obj = doc.as<JsonObject>();

      verseReference = obj["reference"].as<String>();
      verseText = obj["text"].as<String>();

      // Calculate max scroll offset based on text length
      int textHeight = (verseText.length() / 21 + 1) * 8; // 21 chars per line, 8 pixels per line height
      maxScrollOffset = textHeight - 64; // 64 pixels display height

      scrollOffset = 0;
      displayVerse();

    } else {
      Serial.println("Error!");
      delay(2000);
    }
    http.end();
  }
}

void displayVerse() {
  display.clearDisplay();

  display.setTextSize(1);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(0, -scrollOffset);  // Start at top-left corner and apply scroll offset

  display.print("Reference: ");
  display.println(verseReference);
  display.println(verseText);

  display.display();
}
