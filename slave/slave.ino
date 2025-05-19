// --- ESP32 Slave (UDP Receiver with Inverted External LED) ---

#include <WiFi.h>
#include <WiFiUdp.h>

// --- WiFi Configuration (must match Master's AP settings) ---
const char* ap_ssid = "ESP32_Broadcast_Net"; // SSID of the Master's AP
const char* ap_password = "masterpassword";  // Password of the Master's AP

// --- LED Configuration ---
const int onboardLedPin = 2;    // GPIO pin for the onboard LED (indicates signal presence)
const int externalLedPin = 5;   // GPIO pin for the external LED (indicates signal absence)

// --- UDP Configuration (must match Master's settings) ---
const unsigned int listenPort = 4444;           // Port to listen on for broadcasts
const char* expectedMessage = "MASTER_SIGNAL"; // The message to expect
WiFiUDP Udp;
char packetBuffer[255]; // Buffer to hold incoming packets

// --- Signal Timeout Configuration ---
unsigned long lastSignalTime = 0; // Stores the time of the last received valid signal
const unsigned long signalTimeoutDuration = 1000; // Time in ms to wait before turning onboard LED off / external LED on
const unsigned long wifiReconnectInterval = 10000; // How often to attempt Wi-Fi reconnect if disconnected (10 seconds)
unsigned long lastWifiReconnectAttempt = 0;


void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for Serial Monitor to connect
  Serial.println("\nESP32 Slave Receiver Sketch with Inverted External LED & Auto-Reconnect");

  // Configure LED pins as output
  pinMode(onboardLedPin, OUTPUT);
  pinMode(externalLedPin, OUTPUT);

  // Initial LED states: onboard OFF, external ON (indicates no signal yet)
  digitalWrite(onboardLedPin, LOW);
  digitalWrite(externalLedPin, HIGH);
  lastSignalTime = millis(); // Initialize last signal time

  // Set ESP32 to Wi-Fi Station mode
  WiFi.mode(WIFI_STA);
  connectToWiFi(); // Initial connection attempt
}

void connectToWiFi() {
  Serial.print("Connecting to AP: ");
  Serial.println(ap_ssid);
  
  digitalWrite(onboardLedPin, LOW);   // Onboard LED OFF during connection attempt
  digitalWrite(externalLedPin, HIGH); // External LED ON during connection attempt

  WiFi.disconnect(true); // Disconnect from any previous network gracefully
  delay(100);
  WiFi.begin(ap_ssid, ap_password);

  int connectTryCount = 0;
  // Wait for connection, with a timeout
  while (WiFi.status() != WL_CONNECTED && connectTryCount < 40) { // Approx 20 seconds timeout (40 * 500ms)
    delay(500);
    Serial.print(".");
    connectTryCount++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to AP!");
    Serial.print("Slave IP Address: ");
    Serial.println(WiFi.localIP());
    // Begin UDP listening on the specified port
    if (Udp.begin(listenPort)) {
      Serial.print("UDP Listener started on port: ");
      Serial.println(listenPort);
      // Successfully connected and listening, external LED can go off if signals start immediately
      // but we'll let the loop handle the first signal.
      // Onboard LED remains off until first signal.
    } else {
      Serial.println("Failed to start UDP Listener!");
      digitalWrite(onboardLedPin, LOW);   // Ensure onboard LED is OFF
      digitalWrite(externalLedPin, HIGH); // External LED ON (problem state)
    }
    lastSignalTime = millis(); // Reset after successful connection and UDP start
  } else {
    Serial.println("\nFailed to connect to AP in setup. Will retry in loop.");
    digitalWrite(onboardLedPin, LOW);   // Ensure onboard LED is OFF
    digitalWrite(externalLedPin, HIGH); // External LED ON (problem state)
  }
  lastWifiReconnectAttempt = millis(); // Set timestamp for next potential reconnect attempt
}

void loop() {
  // Check Wi-Fi connection status
  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(onboardLedPin, LOW);   // Onboard LED OFF as we can't receive signals
    digitalWrite(externalLedPin, HIGH); // External LED ON (Wi-Fi disconnected)
    // Try to reconnect periodically
    if (millis() - lastWifiReconnectAttempt > wifiReconnectInterval) {
      Serial.println("Wi-Fi disconnected. Attempting to reconnect...");
      connectToWiFi(); // Attempt to reconnect
    }
    return; // Skip the rest of the loop if not connected
  }

  // If connected, proceed with UDP packet checking
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    // Read the packet into the buffer
    int len = Udp.read(packetBuffer, sizeof(packetBuffer) - 1);
    if (len > 0) {
      packetBuffer[len] = 0; // Null-terminate the string
    }

    // Check if the received message matches the expected message
    if (strcmp(packetBuffer, expectedMessage) == 0) {
      // Serial.println("Correct Master signal received!"); // Can be noisy
      digitalWrite(onboardLedPin, HIGH);  // Turn ON onboard LED (signal present)
      digitalWrite(externalLedPin, LOW); // Turn OFF external LED (signal present)
      lastSignalTime = millis();         // Update the time of the last valid signal
    } else {
      // Optional: Handle incorrect messages
      // Serial.print("Received unexpected message: ");
      // Serial.println(packetBuffer);
    }
  }

  // Check if the signal has timed out (and we are connected to Wi-Fi)
  if (WiFi.status() == WL_CONNECTED && (millis() - lastSignalTime > signalTimeoutDuration)) {
    digitalWrite(onboardLedPin, LOW);   // Turn OFF onboard LED (signal timed out)
    digitalWrite(externalLedPin, HIGH); // Turn ON external LED (signal timed out / absent)
  }
}
