# ROS2-ESP32-Servo-Control
Controlling the servo motor remotely by using with ros2_control framework  

# PIKES Smart Servo Project

##  Overview
The **PIKES Smart Servo Project** integrates an **ESP32-based servo system** with **ROS 2 Control** to simulate and manage a door-opening mechanism.  
It consists of two main parts:
1. **ESP32 Wokwi Sub-task** → Simulates servo + gear mechanism with WiFi, MQTT, torque detection, and OLED visualization.
2. **ROS 2 Package (`pikes_servo_ros_control`)** → Provides hardware interface and controller integration with ROS 2 Control.

Together, they demonstrate how embedded hardware (ESP32) can be bridged with ROS 2 for robotic applications.

---

##  Features
- **ESP32 Wokwi Simulation**
  - WiFi connectivity
  - MQTT communication
  - Randomized door endpoint generation
  - Torque simulation for endpoint detection
  - OLED display visualization
- **ROS 2 Control Package**
  - Custom hardware interface plugin
  - Controller manager integration
  - URDF/Xacro support for servo joints
  - Launch files and YAML configs
  - Bridge to ESP32 via MQTT/serial

---

## Project Structure
```text
PIKES-Servo-Project/
├── esp32_wokwi/
│   └── sketch.ino
│   └── README.md   # ESP32 sub-task documentation
├── pikes_servo_ros_control/
│   ├── CMakeLists.txt
│   ├── package.xml
│   ├── include/pikes_servo_ros_control/
│   │   └── esp32_servo_hardware.hpp
│   ├── src/
│   │   └── esp32_servo_hardware.cpp
│   ├── launch/
│   │   └── servo_control.launch.py
│   ├── urdf/
│   │   └── pikes_servo.urdf.xacro
│   └── config/
│       └── controllers.yaml
|   └── README.md   # ROS package sub-task documentation
└── README.md       # Complete project documentation
