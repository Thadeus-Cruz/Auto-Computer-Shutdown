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
const char* pcShutdownURL = "http://192.168.1.150:8000/shutdown";
// ---------------------

// --- GPIO Pin Definitions ---
#define PC_ON_PIN 13          // D13, used for both "On" and "Hard Shutdown"
#define PC_RESET_PIN 12       // D12, used for "Reset"
#define UPS_BUTTON_PIN 14     // D14, used for the UPS button
#define MAIN_POWER_SENSE_PIN 2 // D2, detects if main power is ON (HIGH) or OFF (LOW)

// --- Global Variables for Auto Power Management ---
int lastMainPowerState = LOW; // Stores the last known state of the main power supply

// --- HELPER FUNCTIONS for controlling devices ---

// Function to perform a soft shutdown via network
void softShutdownPC() {
  Serial.println("Action: Shutting down PC via network...");
  Serial.println("Contacting PC server...");

  HTTPClient http;

  // The http.begin() call is slightly different for ESP32
  if (http.begin(pcShutdownURL)) {
    // Send the GET request to your PC
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

// Function to press the PC's power button
void pressPC_ON_Button() {
    Serial.println("Action: Pressing PC Power Button...");
    digitalWrite(PC_ON_PIN, HIGH);
    delay(500); // Hold the pin high for 0.5 seconds
    digitalWrite(PC_ON_PIN, LOW);
    Serial.println("PC Power signal sent.");
}

// Function to press the UPS's power button
void pressUPS_Button() {
    Serial.println("Action: Pressing UPS button...");
    digitalWrite(UPS_BUTTON_PIN, HIGH);
    delay(1000); // Hold the pin high for 1 second
    digitalWrite(UPS_BUTTON_PIN, LOW);
    Serial.println("UPS button signal sent.");
}


// --- AUTOMATION SEQUENCES ---

// Sequence to run when main power is first detected or restored
void startupSequence() {
  Serial.println("AUTO: Main power detected. Initiating startup sequence...");
  
  Serial.println("AUTO: Waiting 5 seconds before starting UPS...");
  delay(5000);
  
  pressUPS_Button(); // Turn on the UPS
  
  Serial.println("AUTO: Waiting 5 seconds for UPS to stabilize before starting PC...");
  delay(5000);
  
  pressPC_ON_Button(); // Turn on the PC
  Serial.println("AUTO: Startup sequence complete.");
}

// Sequence to run when main power is lost
void shutdownSequence() {
  Serial.println("AUTO: Main power failure detected! Initiating shutdown sequence...");
  
  softShutdownPC(); // Trigger PC soft shutdown
  
  Serial.println("AUTO: Waiting 10 seconds for PC to shut down...");
  delay(10000);
  
  pressUPS_Button(); // Turn off the UPS
  
  // NOTE: If Mains power is off, the ESP32 will lose all power at this point.
  // Any code after this line will not execute in a power failure scenario until
  // main power is restored and the ESP32 reboots.
  Serial.println("AUTO: Shutdown sequence complete.");
}


// --- BLYNK VIRTUAL PIN HANDLERS (Manual Control) ---

// This function is for the SOFT SHUTDOWN (V1)
BLYNK_WRITE(V1)
{
  if (param.asInt() == 1) {
    softShutdownPC();
  }
}

// This function is for turning the PC ON (V3)
BLYNK_WRITE(V3)
{
  if (param.asInt() == 1) {
    pressPC_ON_Button();
  }
}

// This function is for RESETTING the PC (V2)
BLYNK_WRITE(V2)
{
  if (param.asInt() == 1) {
    Serial.println("Action: Resetting PC...");
    digitalWrite(PC_RESET_PIN, HIGH);
    delay(500);
    digitalWrite(PC_RESET_PIN, LOW);
    Serial.println("PC Reset signal sent.");
  }
}

// This function is for the HARD SHUTDOWN (V4)
BLYNK_WRITE(V4)
{
  if (param.asInt() == 1) {
    Serial.println("Action: Performing Hard Shutdown...");
    digitalWrite(PC_ON_PIN, HIGH);
    delay(10000); // Hold the pin high for 10 seconds
    digitalWrite(PC_ON_PIN, LOW);
    Serial.println("Hard Shutdown signal sent.");
  }
}

// This function is for pressing the UPS button (V5)
BLYNK_WRITE(V5)
{
  if (param.asInt() == 1) {
    pressUPS_Button();
  }
}

// --- MAIN SETUP AND LOOP ---

void setup()
{
  Serial.begin(115200);
  Serial.println("\nStarting Blynk PC Controller for ESP32...");

  // Set the GPIO output pins
  pinMode(PC_ON_PIN, OUTPUT);
  pinMode(PC_RESET_PIN, OUTPUT);
  pinMode(UPS_BUTTON_PIN, OUTPUT);

  // Set the GPIO input pin for power sensing
  pinMode(MAIN_POWER_SENSE_PIN, INPUT_PULLDOWN);

  // Ensure the output pins start in a LOW state
  digitalWrite(PC_ON_PIN, LOW);
  digitalWrite(PC_RESET_PIN, LOW);
  digitalWrite(UPS_BUTTON_PIN, LOW);
  
  // --- Non-blocking Wi-Fi and Blynk Configuration ---
  WiFi.begin(ssid, pass);
  Blynk.config(BLYNK_AUTH_TOKEN);
  
  // --- Auto-start logic ---
  // This block runs ONLY ONCE when the ESP32 boots up. This is critical because
  // after a full power loss (Mains + UPS off), this is the first code that runs
  // when Mains power is restored.
  delay(100); 
  int currentPowerState = digitalRead(MAIN_POWER_SENSE_PIN);
  lastMainPowerState = currentPowerState; // Initialize the state tracker for the loop()

  // If the ESP32 boots and finds that main power is already on,
  // it assumes a recovery from a power outage and starts everything up.
  if (currentPowerState == HIGH) {
    startupSequence();
  } else {
    // This case would happen if the ESP32 is powered via USB for programming
    // while the main power circuit is off.
    Serial.println("Main power is OFF on boot. Awaiting power restoration.");
  }
}

void loop()
{
  // Manages the Blynk connection in a non-blocking way.
  Blynk.run();

  // --- Core Automation Logic (runs continuously) ---
  // This logic handles state changes that happen *while the ESP32 remains powered*.
  int currentPowerState = digitalRead(MAIN_POWER_SENSE_PIN);

  // Scenario: The ESP32 was running on UPS power (Mains were off), and then Mains power was restored.
  // This triggers the startup sequence.
  if (currentPowerState == HIGH && lastMainPowerState == LOW) {
    startupSequence();
  }

  // Scenario: The ESP32 is running on Mains power, and then Mains power fails.
  // This triggers the shutdown sequence. The ESP32 will lose power at the end of this sequence.
  if (currentPowerState == LOW && lastMainPowerState == HIGH) {
    shutdownSequence();
  }

  // Update the state for the next loop. This is crucial for detecting a CHANGE in power state.
  lastMainPowerState = currentPowerState;
}

