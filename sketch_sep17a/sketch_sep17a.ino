
// --- These are from your Blynk Template ---
#define BLYNK_TEMPLATE_ID "Template ID"
#define BLYNK_TEMPLATE_NAME "Template name"
#define BLYNK_AUTH_TOKEN "Blynk Auth Token"

// --- Standard Includes ---
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266HTTPClient.h>

// --- UPDATE THESE ---
char ssid[] = "CruzoeInnotek_5G_Ext";
char pass[] = "Cruzoetek";

// This is the URL of the server running on your PC
const char* pcShutdownURL = "http://192.168.0.150:8000/shutdown";
// ---------------------

// This function is called every time you press the button (on V1) in the Blynk app
// This function is called every time you press the button (on V1) in the Blynk app
BLYNK_WRITE(V1)
{
  int pinValue = param.asInt(); // Get value from the app button

  if (pinValue == 1) { // If the button is pressed down
    Serial.println("Blynk button pressed. Contacting PC server...");

    WiFiClient client;
    HTTPClient http;

    if (http.begin(client, pcShutdownURL)) {
      // Send the GET request to your PC
      int httpCode = http.GET();
      if (httpCode > 0) {
        String payload = http.getString();
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        Serial.println(payload);
        
        // --- THIS LINE WAS REMOVED ---
        // Blynk.notify("Shutdown command sent to PC!"); 
        
      } else {
        String errorMsg = http.errorToString(httpCode).c_str();
        Serial.printf("[HTTP] GET... failed, error: %s\n", errorMsg.c_str());

        // --- THIS LINE WAS REMOVED ---
        // Blynk.notify("Failed to contact PC server!"); 
      }
      http.end();
    } else {
      Serial.printf("[HTTP] Unable to connect to PC\n");
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("\nStarting Blynk Shutdown Trigger...");
  
  // Connect to Wi-Fi and Blynk (uses the Auth Token from the #define above)
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Connected to Blynk!");
}

void loop()
{
  Blynk.run(); // This keeps the ESP connected to Blynk
}