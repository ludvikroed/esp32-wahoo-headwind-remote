# ESP32 Wahoo Headwind Button Controller

Made by me and ChatGPT❤️

This project allows you to control a **Wahoo Headwind smart fan** using an **ESP32** and two physical buttons.  
It connects directly to the fan via **Bluetooth Low Energy (BLE)** and switches the fan to **manual mode**, allowing speed control without using the Wahoo app.

With this setup you can increase or decrease fan speed using two buttons — useful for indoor cycling setups where you want quick physical control.

---

# Features

- Connects to **Wahoo Headwind** via BLE
- Automatically switches the fan to **Manual Mode**
- Two-button control
  - Increase fan speed
  - Decrease fan speed
- Adjustable speed in **10% increments**
- Automatic reconnect if the connection drops

---

# Hardware Requirements

- ESP32 development board
- Wahoo Headwind fan
- 2 momentary push buttons connected to the esp32

---

# Wiring

Buttons are connected using the ESP32's **internal pull-up resistors**.

| Button | ESP32 Pin |
|------|------|
| Increase speed | GPIO 13 |
| Decrease speed | GPIO 14 |

### Wiring diagram

Button 1 (Speed Up)

```
ESP32 GPIO13 ---- Button ---- GND
```

Button 2 (Speed Down)

```
ESP32 GPIO14 ---- Button ---- GND
```

When pressed, the pin reads **LOW**.

---

# Installation

## 1 Install Arduino IDE

Download and install the Arduino IDE:

https://www.arduino.cc/en/software

---

## 2 Install ESP32 Board Support

Open **Arduino IDE → Preferences**

Add this URL to **Additional Board Manager URLs**

```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

Then:

1. Open **Boards Manager**
2. Search for **ESP32**
3. Install **Espressif ESP32**

---

## 3 Install Required Library

This project uses the built-in ESP32 BLE libraries included with the ESP32 board package:

```
BLEDevice
BLEUtils
BLEClient
```

No additional libraries are required.

---

## 4 Upload the Code

1. Connect the ESP32 via USB
2. Select your board in **Tools → Board**
3. Select the correct **COM port**
4. Upload the sketch

---

# Finding the Headwind MAC Address

The ESP32 connects to the Headwind using its **Bluetooth MAC address**.

There are several ways to find it.

---

## Method 1 — Using a BLE Scanner App (Recommended)

Install a BLE scanner app on your phone:

- **nRF Connect (iOS / Android)**
- **LightBlue (iOS / Android)**

Steps:

1. Turn on the **Wahoo Headwind**
2. Open the scanner app
3. Start scanning for BLE devices
4. Look for a device named:

```
HEADWIND
```

5. Note the **MAC address**

Example:

```
d4:ca:6e:12:34:56
```

---

## Method 2 — Using an ESP32 Scanner

You can also run a simple ESP32 BLE scanner sketch to list nearby BLE devices and find the Headwind address.

---

# Configure the Code

Edit this line in the sketch:

```cpp
const char* headwindAddress = "Headwind mac address";
```

Replace it with your Headwind MAC address (lowercase recommended):

```cpp
const char* headwindAddress = "d4:ca:6e:12:34:56";
```

---

# How It Works

1. The ESP32 scans for the Headwind using BLE.
2. When the correct MAC address is found, it connects.
3. The fan is switched to **Manual Mode**.
4. Button presses send commands to change fan speed.

Commands sent to the Headwind:

| Command | Function |
|------|------|
| `0x04 0x04` | Set Manual Mode |
| `0x02 XX` | Set fan speed |

Where `XX` is the fan speed value (0–100).

---

# Fan Speed Control

Buttons adjust fan speed in **10% increments**.

| Button | Action |
|------|------|
| UP | Increase speed |
| DOWN | Decrease speed |

Speed limits:

```
Minimum: 0%
Maximum: 100%
```

---

# Serial Debug Output

Open the **Serial Monitor (115200 baud)** to see debug messages.

Example:

```
Scanning for Headwind...
Found Headwind!
Connecting to Headwind...
Connected!
Set Headwind to MANUAL mode
Button UP pressed
Fan speed: 30
```

---

# Notes

- The ESP32 will automatically reconnect if the connection drops.
- The fan must not already be actively controlled by another device.
- Using the fan in **Manual Mode** disables automatic speed control from cycling apps.

---

# Disclaimer

This is an unofficial project and is **not affiliated with Wahoo**.

Use at your own risk.

---

## ⭐ Support the Project

This project was originally built for my own indoor cycling setup so I could control the **Wahoo Headwind** with physical buttons instead of using the app.

If you find this project useful and decide to use it yourself, please consider giving the repository a **⭐ star** on GitHub. It helps others find the project and shows that people are actually using it.

The project was mainly made for personal use, but if it turns out to be helpful for others in the community that's awesome!

---

# License

MIT License
