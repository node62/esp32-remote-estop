
## ESP32 Wireless Relay Control

A wireless system using two ESP32s: a Master (switch-controlled transmitter) and a Slave (signal receiver controlling a relay and LED).

---

### What You Need

* Two ESP32 development boards
* One switch (e.g., push button/toggle)
* One Relay Module (e.g., SRD-05VDC-SL-C, active-LOW)
* Connecting wires
* Arduino IDE (or similar)

---

### Getting Started

1.  **Code:** Use `master.ino` and `slave.ino` sketches.
2.  **Upload to Master:** Open and upload `master.ino` to the first ESP32.
3.  **Upload to Slave:** Open and upload `slave.ino` to the second ESP32.

    * **Troubleshooting:** If "failed to detect chipset" or similar errors occur, hold the ESP32's **BOOT** button during upload initiation, then release.

---

### Master ESP32: Setup & Connections

* **Function:** Creates Wi-Fi, sends signal via switch.
* **Onboard Blue LED (GPIO2):** ON when broadcasting.
* **Wi-Fi:** SSID `Gatik_bhai_ke_esp32`, Password `longpassword`.
* **Signal:** Broadcasts `"MASTER_SIGNAL"` on UDP port `4444`.

**⭐ Connections (Master):**
* Master ESP32 **GPIO4** -> Positive terminal of Switch
* Master ESP32 **GND** -> Another positive terminal (+N) of Switch
    * Forget about the negative terminal of the switch

---

### Slave ESP32 & Relay: Setup & Connections

* **Function:** Connects to Master's Wi-Fi, listens for signal, controls relay.
* **Onboard Blue LED (GPIO2):** ON when Master signal received.
* **Relay Control (GPIO5):** For active-LOW relay modules.
    * Relay activates (energizes) if GPIO5 is LOW.
    * Relay deactivates if GPIO5 is HIGH.
* **Signal Timeout:** Relay activates if no signal for 1 second.
* **Wi-Fi Reconnect:** Tries reconnecting every 10 seconds if Wi-Fi drops.

**⭐ Connections (Slave & Relay Module):**
* Slave ESP32 **GPIO5** -> Relay Module **IN** pin (signal input)
* Slave ESP32 **GND** -> Relay Module **GND** pin
* Slave ESP32 **5V (or VIN)** -> Relay Module **VCC** pin (confirm relay voltage, use external PSU if ESP32 cannot supply enough current/correct voltage)

**Relay Load Connections (Example: Device turns OFF when signal is RECEIVED):**
* Relay **COM** (Common) -> Device/Load (one power terminal)
* Relay **NO** (Normally Open) -> Power Source for Device/Load
    * *This setup ensures the device is powered OFF when the relay is de-energized (signal received) because the NO contact is open. The device will be powered ON when the relay is energized (signal lost) because the NO contact closes to COM.*
* *(Alternatively, to turn device ON if signal is received, use Relay **NC** (Normally Closed) instead of NO for connection to Power Source).*


---

### How It Works

1.  **Power Up:** Turn on both ESP32s.
2.  **Master:**
    * Creates `Gatik_bhai_ke_esp32` Wi-Fi.
    * Switch ON: Blue LED ON, broadcasts `"MASTER_SIGNAL"`.
    * Switch OFF: Blue LED OFF, stops broadcasting.
3.  **Slave:**
    * Connects to Master's Wi-Fi. (During connection: blue LED OFF, relay ACTIVE).
    * **Signal Received:** Blue LED ON, relay DEACTIVATES.
    * **Signal Lost/Timeout (1s):** Blue LED OFF, relay ACTIVATES.
    * **Wi-Fi Disconnected:** Blue LED OFF, relay ACTIVATES, tries to reconnect.

