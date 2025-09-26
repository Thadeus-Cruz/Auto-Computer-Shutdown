// --- These are from your Blynk Template ---
#define BLYNK_TEMPLATE_ID "Template ID"
#define BLYNK_TEMPLATE_NAME "Template name"
#define BLYNK_AUTH_TOKEN "Blynk Auth Token"

// --- Standard Includes ---
#define BLYNK_PRINT Serial
// Use ESP32 specific libraries
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>

// --- UPDATE THESE ---
char ssid[] = "CruzoeInnotek_5G_Ext";
char pass[] = "Cruzoetek";

// This is the URL of the server running on your PC for soft shutdown
const char* pcShutdownURL = "http://192.168.0.150:8000/shutdown";
// ---------------------

// --- GPIO Pin Definitions ---
#define PC_ON_PIN 13      // D13, used for "On" (V2) and "Hard Shutdown" (V4)
#define PC_RESET_PIN 12   // D12, used for "Reset" (V3)
#define EXTRA_PIN_1 14    // D14, controlled by V5
#define EXTRA_PIN_2 27    // D27, controlled by V6

// This function is for the SOFT SHUTDOWN (V1) - PUSH BUTTON
BLYNK_WRITE(V1)
{
  int pinValue = param.asInt(); // Get value from the app button

  if (pinValue == 1) { // If the button is pressed down
    Serial.println("Action: Shutting down PC via network...");
    Serial.println("Contacting PC server...");

    HTTPClient http;

    if (http.begin(pcShutdownURL)) {
      int httpCode = http.GET();
      if (httpCode > 0) {
        String payload = http.getString();
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        Serial.println(payload);
      } else {
        String errorMsg = http.errorToString(httpCode).c_str();
        Serial.printf("[HTTP] GET... failed, error: %s\n", errorMsg.c_str());
      }
      http.end();
    } else {
      Serial.printf("[HTTP] Unable to connect to PC\n");
    }
  }
}

// This function is for PC ON (V2) - SWITCH
BLYNK_WRITE(V2)
{
  int pinValue = param.asInt();
  
  // The pin state will directly follow the switch state in the app
  digitalWrite(PC_ON_PIN, pinValue); 
  
  if (pinValue == 1) {
    Serial.println("Action: Pin D13 is ON.");
  } else {
    Serial.println("Action: Pin D13 is OFF.");
  }
}

// This function is for PC RESET (V3) - SWITCH
BLYNK_WRITE(V3)
{
  int pinValue = param.asInt();

  // The pin state will directly follow the switch state in the app
  digitalWrite(PC_RESET_PIN, pinValue);
  
  if (pinValue == 1) {
    Serial.println("Action: Pin D12 is ON.");
  } else {
    Serial.println("Action: Pin D12 is OFF.");
  }
}

// This function is for the HARD SHUTDOWN (V4) - PUSH BUTTON
BLYNK_WRITE(V4)
{
  int pinValue = param.asInt();
  if (pinValue == 1) {
    Serial.println("Action: Performing Hard Shutdown...");
    digitalWrite(PC_ON_PIN, HIGH);
    delay(10000); // Hold the pin high for 10 seconds
    digitalWrite(PC_ON_PIN, LOW);
    Serial.println("Hard Shutdown signal sent.");
  }
}

// This function is for controlling D14 (V5) - SWITCH
BLYNK_WRITE(V5)
{
  int pinValue = param.asInt();

  // The pin state will directly follow the switch state in the app
  digitalWrite(EXTRA_PIN_1, pinValue);
  
  if (pinValue == 1) {
    Serial.println("Action: Pin D14 is ON.");
  } else {
    Serial.println("Action: Pin D14 is OFF.");
  }
}

// This function is for controlling D27 (V6) - SWITCH
BLYNK_WRITE(V6)
{
  int pinValue = param.asInt();

  // The pin state will directly follow the switch state in the app
  digitalWrite(EXTRA_PIN_2, pinValue);
  
  if (pinValue == 1) {
    Serial.println("Action: Pin D27 is ON.");
  } else {
    Serial.println("Action: Pin D27 is OFF.");
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("\nStarting Blynk PC Controller for ESP32...");

  // Set the GPIO pins to output mode
  pinMode(PC_ON_PIN, OUTPUT);
  pinMode(PC_RESET_PIN, OUTPUT);
  pinMode(EXTRA_PIN_1, OUTPUT);
  pinMode(EXTRA_PIN_2, OUTPUT);

  // Ensure the pins start in a LOW state
  digitalWrite(PC_ON_PIN, LOW);
  digitalWrite(PC_RESET_PIN, LOW);
  digitalWrite(EXTRA_PIN_1, LOW);
  digitalWrite(EXTRA_PIN_2, LOW);
  
  // Connect to Wi-Fi and Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Connected to Blynk!");
}

void loop()
{
  Blynk.run(); // This keeps the ESP connected to Blynk
}

