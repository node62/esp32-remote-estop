// --- ESP32 Master (Switch Controlled UDP Broadcaster) ---

#include <WiFi.h>
#include <WiFiUdp.h>

// --- WiFi Access Point Configuration ---
const char* ap_ssid = "ESP32_Broadcast_Net"; // SSID for the AP
const char* ap_password = "longpassword";  // Password for the AP (min 8 characters)

// --- LED Configuration ---
const int onboardLedPin = 2; // GPIO pin for the onboard LED

// --- Switch Configuration ---
const int switchPin = 4; // GPIO pin connected to the switch

// --- UDP Broadcast Configuration ---
const unsigned int broadcastPort = 4444;         // Port to broadcast on
const char* broadcastMessage = "MASTER_SIGNAL"; // The message to broadcast
WiFiUDP Udp;

// Debounce variables for the switch (optional but good practice)
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50; // 50 milliseconds
int lastSwitchState = HIGH;       // Assuming switch is off initially (due to pull-up)
int currentSwitchState = HIGH;

bool isBroadcasting = false; // Tracks if we should be broadcasting

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for Serial Monitor to connect
  Serial.println("\nESP32 Master Switch-Controlled Broadcaster Sketch");

  // Configure the onboard LED pin as an output
  pinMode(onboardLedPin, OUTPUT);
  digitalWrite(onboardLedPin, LOW); // Ensure LED is off initially

  // Configure the switch pin as an input with an internal pull-up resistor
  pinMode(switchPin, INPUT_PULLUP);
  currentSwitchState = digitalRead(switchPin); // Read initial state
  lastSwitchState = currentSwitchState;
  if (currentSwitchState == LOW) { // Switch is ON at boot
      isBroadcasting = true;
  }


  // Configure ESP32 as a WiFi Access Point
  Serial.print("Setting up Access Point: ");
  Serial.println(ap_ssid);
  if (WiFi.softAP(ap_ssid, ap_password)) {
    Serial.println("Access Point CREATED");
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
  } else {
    Serial.println("Failed to create Access Point!");
    Serial.println("Restarting ESP32...");
    delay(1000);
    ESP.restart();
  }

  // Begin UDP.
  if (Udp.begin(broadcastPort)) {
    Serial.print("UDP Broadcaster initialized on port: ");
    Serial.println(broadcastPort);
  } else {
    Serial.println("Failed to start UDP Broadcaster!");
    // Handle error appropriately
  }

  // Initial status message based on switch state
  if (isBroadcasting) {
    digitalWrite(onboardLedPin, HIGH);
    Serial.println("Switch is ON. Broadcasting ENABLED. Master LED ON.");
  } else {
    digitalWrite(onboardLedPin, LOW);
    Serial.println("Switch is OFF. Broadcasting DISABLED. Master LED OFF.");
  }
}

void loop() {
  // Read the switch state with debounce
  int reading = digitalRead(switchPin);

  // If the switch changed, due to noise or pressing:
  if (reading != lastSwitchState) {
    lastDebounceTime = millis(); // Reset the debouncing timer
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // Whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:
    if (reading != currentSwitchState) {
      currentSwitchState = reading;

      if (currentSwitchState == LOW) { // Switch has been turned ON
        isBroadcasting = true;
        digitalWrite(onboardLedPin, HIGH);
        Serial.println("Switch ON -> Broadcasting ENABLED. Master LED ON.");
      } else { // Switch has been turned OFF (currentSwitchState == HIGH)
        isBroadcasting = false;
        digitalWrite(onboardLedPin, LOW);
        Serial.println("Switch OFF -> Broadcasting DISABLED. Master LED OFF.");
      }
    }
  }
  lastSwitchState = reading; // Save the reading for next time

  // --- Broadcasting Logic ---
  if (isBroadcasting) {
    // Determine the broadcast IP address for the AP's subnet.
    IPAddress broadcastIP = WiFi.softAPBroadcastIP();

    // Send the broadcast message
    Udp.beginPacket(broadcastIP, broadcastPort);
    Udp.write((const unsigned char*)broadcastMessage, strlen(broadcastMessage));
    Udp.endPacket();

    // Optional: Print confirmation of sending (can be very noisy)
    // Serial.print("Broadcast sent: "); Serial.println(broadcastMessage);

  } else {
    // Not broadcasting, LED should be off (already handled by switch logic)
  }

  // Delay slightly to prevent flooding and allow other tasks
  delay(100); // Adjust as needed. If broadcasting, this sets the broadcast interval.
}
