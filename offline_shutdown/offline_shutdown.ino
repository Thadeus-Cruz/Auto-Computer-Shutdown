// --- GPIO Pin Definitions ---
#define PC_ON_PIN 12          // Pin for the PC Power button relay
#define PC_RESET_PIN 13       // Pin for the PC Reset button relay
#define UPS_BUTTON_PIN 14     // Pin for the UPS button relay
#define MAIN_POWER_SENSE_PIN 2 // D2, detects if main power is ON (HIGH) or OFF (LOW)

// --- Global Variables for Auto Power Management ---
int lastMainPowerState = LOW; // Stores the last known state of the main power supply

// --- HELPER FUNCTIONS for controlling devices ---

// Function to press ONLY the PC's power button
// A short press will turn the PC on, or trigger a graceful OS shutdown if it's already on.
void pressPC_Power_Button() {
    Serial.println("Action: Pressing PC Power Button...");
    digitalWrite(PC_ON_PIN, HIGH);
    delay(500); // Hold for 0.5 seconds
    digitalWrite(PC_ON_PIN, LOW);
    Serial.println("PC Power signal sent.");
}

// Function to press ONLY the PC's reset button
void pressPC_Reset_Button() {
    Serial.println("Action: Pressing PC Reset Button...");
    digitalWrite(PC_RESET_PIN, HIGH);
    delay(500); // Hold for 0.5 seconds
    digitalWrite(PC_RESET_PIN, LOW);
    Serial.println("PC Reset signal sent.");
}

// Function to press the UPS's power button
void pressUPS_Button() {
    Serial.println("Action: Pressing UPS button...");
    digitalWrite(UPS_BUTTON_PIN, HIGH);
    delay(1000); // Hold for 1 second
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
  f
  Serial.println("AUTO: Waiting 10 seconds for UPS to stabilize...");
  delay(10000);
  
  // --- CUSTOM STARTUP LOGIC for quirky PC ---
  Serial.println("AUTO: Initiating custom PC start (Power then Reset)...");
  pressPC_Power_Button(); // First, press the power button
  
  delay(30000); // Wait 5 seconds
  
  pressPC_Reset_Button(); // Now, press the reset button to ensure it starts
  // --- END CUSTOM STARTUP LOGIC ---
  
  Serial.println("AUTO: Startup sequence complete.");
}

// Sequence to run when main power is lost
void shutdownSequence() {
  Serial.println("AUTO: Main power failure detected! Initiating shutdown sequence...");
  delay(5000);

  // CRITICAL CHANGE: We ONLY call the power button function here to prevent a reboot.
  Serial.println("AUTO: Sending graceful shutdown signal via PC power button...");
  pressPC_Power_Button();
  
  // Wait longer to give the OS time to shut down completely.
  Serial.println("AUTO: Waiting 20 seconds for PC to shut down...");
  delay(20000);
  
  // This is the final step. It will power off the UPS.
  Serial.println("AUTO: Turning off UPS...");
  pressUPS_Button(); 
  
  // NOTE: The ESP32 will lose all power at this point if it's only powered by the UPS.
  Serial.println("AUTO: Shutdown sequence should be complete.");
}

// --- MAIN SETUP AND LOOP ---

void setup()
{
  Serial.begin(115200);
  Serial.println("\nStarting Offline PC Controller for ESP32...");

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
  
  // --- Auto-start logic ---
  // This block runs ONLY ONCE when the ESP32 boots up.
  delay(100); 
  int currentPowerState = digitalRead(MAIN_POWER_SENSE_PIN);
  lastMainPowerState = currentPowerState;

  // If the ESP32 boots and finds that main power is already on,
  // it assumes a recovery from a power outage and starts everything up.
  if (currentPowerState == HIGH) {
    startupSequence();
  } else {
    Serial.println("Main power is OFF on boot. Awaiting power restoration.");
  }
}

void loop()
{
  // --- Core Automation Logic (runs continuously) ---
  int currentPowerState = digitalRead(MAIN_POWER_SENSE_PIN);

  // Scenario: Main power is restored.
  if (currentPowerState == HIGH && lastMainPowerState == LOW) {
    startupSequence();
  }

  // Scenario: Main power fails while the system is running.
  if (currentPowerState == LOW && lastMainPowerState == HIGH) {
    shutdownSequence();
  }

  // Update the state for the next loop. This is crucial for detecting a CHANGE in power state.
  lastMainPowerState = currentPowerState;

  // Add a small delay to prevent the loop from running too fast.
  delay(500);
}