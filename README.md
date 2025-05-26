## ESP32 Wireless Relay Control System

This guide helps you set up a wireless control system using two ESP32s. One ESP32 (Master) acts as a transmitter controlled by a switch. The other ESP32 (Slave) receives the signal and controls a relay module and an onboard LED.

---

### What You Need

* Two ESP32 development boards
* One switch (e.g., push button or toggle)
* One Relay Module (e.g., SRD-05VDC-SL-C, typically active-LOW)
* Connecting wires
* Arduino IDE or similar ESP32 programming environment

---

### Getting Started

1.  **Download Code:** You'll need two sketches: `master.ino` and `slave.ino`.
2.  **Upload to Master ESP32:** Open `master.ino` and upload it to your first ESP32.
3.  **Upload to Slave ESP32:** Open `slave.ino` and upload it to your second ESP32.

    * **Troubleshooting Tip:** If you see a "Failed to connect to ESP32: Timed out waiting for packet header" or "failed to detect chipset" error during upload, try holding down the **BOOT** button on the ESP32 when the IDE starts trying to connect, then release it.

---

### Master ESP32: Setup & Wiring

The Master ESP32 creates a Wi-Fi network and sends a signal when the switch is activated.

* **Onboard Blue LED (GPIO2):** Lights up when broadcasting.
* **Wi-Fi Network:**
    * SSID: `Gatik_bhai_ke_esp32`
    * Password: `longpassword`
* **Broadcast Signal:** Sends `"MASTER_SIGNAL"` on UDP port `4444`.

**Wiring Diagram (Master):**

```
          +---------------------+
          |      Master ESP32   |
          |                     |
  GND ----| GPIO4         (SW)  |----(Switch)---- GND
          |                     |
          | GPIO2 (Onboard Blue LED) |
          |                     |
          +---------------------+
```
* Connect one terminal of your switch to **GPIO4** and the other terminal to **GND**. The code uses an internal pull-up resistor.

---

### Slave ESP32: Setup & Wiring

The Slave ESP32 connects to the Master's Wi-Fi and listens for the signal to control a relay.

* **Onboard Blue LED (GPIO2):** Lights up when a signal is received from the Master.
* **Relay Control (GPIO5):** Controls the relay module. The code is set for an **active-LOW** relay module.
    * `relayActiveState = LOW;` (Relay activates when GPIO5 is LOW)
    * `relayInactiveState = HIGH;` (Relay deactivates when GPIO5 is HIGH)
* **Signal Timeout:** If no signal for 1 second, the relay activates.
* **Wi-Fi Reconnect:** Tries to reconnect every 10 seconds if Wi-Fi drops.

**Wiring Diagram (Slave to Relay Module):**

```
          +---------------------+
          |      Slave ESP32    |
          |                     |
          | GPIO2 (Onboard Blue LED) |
          |                     |
          | GPIO5   ------------|-----> To Relay Module IN pin
          |                     |
          +---------------------+
```
* Connect **GPIO5** on the Slave ESP32 to the **IN (Input)** pin of your relay module.
* Connect the relay module's **VCC** to a 5V source (can be from the ESP32 if the relay current draw is low, otherwise use a separate power supply).
* Connect the relay module's **GND** to the ESP32's GND.
* Connect your device/load to the **COM (Common)** and **NO (Normally Open)** or **NC (Normally Closed)** terminals of the relay, depending on whether you want the load powered when the relay is active or inactive. For an active-LOW relay, `relayActiveState = LOW` means the relay is energized (e.g., NO contact closes) when GPIO5 is LOW.

---

### How It Works

1.  **Power Up:** Turn on both ESP32s.
2.  **Master:**
    * Creates the `Gatik_bhai_ke_esp32` Wi-Fi network.
    * If the switch is ON (connecting GPIO4 to GND) at startup or when flipped ON:
        * Master's onboard blue LED turns ON.
        * Starts broadcasting the `"MASTER_SIGNAL"`.
    * If the switch is OFF:
        * Master's onboard blue LED turns OFF.
        * Stops broadcasting.
3.  **Slave:**
    * Connects to `Gatik_bhai_ke_esp32`. While trying to connect, its onboard blue LED is OFF, and the relay is kept in its **active state** (e.g., energized for an active-LOW relay).
    * **Signal Received:** When it gets the `"MASTER_SIGNAL"`:
        * Slave's onboard blue LED turns ON.
        * Relay is set to its **inactive state** (e.g., de-energized).
    * **Signal Lost / Timeout:** If the signal isn't received for 1 second (configurable by `signalTimeoutDuration`):
        * Slave's onboard blue LED turns OFF.
        * Relay is set to its **active state**.
    * **Wi-Fi Disconnected:** If Wi-Fi drops, the onboard blue LED turns OFF, the relay is set to its **active state**, and the Slave tries to reconnect.

---