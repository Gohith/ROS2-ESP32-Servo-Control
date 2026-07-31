# ESP32 Smart Servo (Wokwi Simulation)

## Overview
This project implements a **smart servo mechanism** on the **ESP32** using the **Wokwi simulator**.  
The servo is connected to a gear mechanism that simulates a **door opening/closing system**.  

Key features:
- WiFi connectivity
- MQTT communication
- Randomized door endpoint generation
- Torque simulation for endpoint detection
- OLED display integration for visualization

This sub-task is part of the larger **Robot Servo Control** project.

---

## Features
- **WiFi setup**: Connects ESP32 to Wokwi-GUEST network.
- **MQTT integration**: Publishes and subscribes to topics for servo control.
- **Servo + Gear mechanism**: Simulates door movement with gear ratio.
- **Torque simulation**: Detects door endpoints based on torque threshold.
- **OLED display**: Shows detected min/max door angles.

---


## System Architecture (Text Diagram)

```text
+-------------------------------------------------------------+
|                     ESP32 (Wokwi Simulation)                |
|-------------------------------------------------------------|
|  - Connects to WiFi (Wokwi-GUEST)                           |
|  - Publishes/Receives MQTT messages                         |
|  - Controls Servo and Gear mechanism                        |
|  - Displays data on OLED                                    |
+-------------------------------------------------------------+
                | Wi-Fi
                v
+-------------------------------------------------------------+
|                     MQTT Broker (HiveMQ)                    |
|-------------------------------------------------------------|
|  broker.hivemq.com                                          |
|  Topics:                                                    |
|   - ESP32Servo/theta                                        |
|   - ESP32Servo/speed                                        |
|   - ESP32Servo/direction                                    |
|   - ESP32ServoGear/gear_angle_back                          |
+-------------------------------------------------------------+
                | Control Commands / Telemetry
                v
+-------------------------------------------------------------+
|             Servo & Gear Mechanism + OLED Display           |
|-------------------------------------------------------------|
|  - Servo motor (pin 4)                                      |
|  - Gear servo motor (pin 5)                                 |
|  - OLED (I2C pins 21, 22)                                   |
|  - Displays Min/Max door angles                             |
+-------------------------------------------------------------+
```

## Hardware & Libraries
- **ESP32** (Wokwi simulation)
- **Servo motor** (pin 4)
- **Gear servo motor** (pin 5)
- **OLED Display (SSD1306)** via I2C (pins 21, 22)

Libraries used:
- `WiFi.h`
- `ESP32Servo.h`
- `PubSubClient.h`
- `Wire.h`
- `Adafruit_GFX.h`
- `Adafruit_SSD1306.h`

---

## MQTT Topics
**Subscribed topics:**
- `ESP32Servo/theta` → updates servo angle
- `ESP32Servo/speed` → updates servo speed
- `ESP32Servo/direction` → updates servo direction

**Published topics:**
- `ESP32Servo/angle_back` → current servo angle
- `ESP32Servo/speed_back` → current servo speed
- `ESP32ServoGear/gear_angle_back` → current gear angle

---

## How to Run (Wokwi)
1. Open [Wokwi](https://wokwi.com).
2. Create a new ESP32 project.
3. Copy the code from `sketch.ino` into the editor.
4. Add components:
   - ESP32 board
   - Servo motor (pin 4)
   - Servo motor (pin 5)
   - SSD1306 OLED display (I2C pins 21, 22)
5. Run the simulation.
6. Observe:
   - Servo + gear movement
   - MQTT messages in broker.hivemq.com
   - OLED display showing detected endpoints

---

## Example Output
**Serial Monitor:**
```
    Connecting to WiFi...
    WiFi Connected
    Connecting to MQTT...
    MQTT Connected!
    Random Point Min: 10
    Random Point Max: 50
    Detected endpoints Min: 12
    Detected endpoints Max: 48
```
**OLED Display:**
```
    Min: 12
    Max: 48
```


## Future Improvements
- Integrate with **ROS2 Control** for real-time robot control.
- Add **endpoint calibration** via MQTT commands.
- Extend torque simulation with real sensor feedback.
