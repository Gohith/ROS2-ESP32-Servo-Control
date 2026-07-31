# PIKES Servo ROS2 Control Package

## Overview
The `pikes_servo_ros_control` package provides a **ROS 2 Control hardware interface** and controller setup for the **PIKES Servo System**.  
It bridges the ESP32-based servo hardware (simulated in Wokwi or running on real hardware) with the ROS 2 ecosystem, enabling position and velocity control through `ros2_control`.

---

## Features
- **Custom Hardware Interface Plugin** (`esp32_servo_hardware.cpp`)
- **Controller Manager Integration** with `ros2_control_node`
- **URDF/Xacro Support** for servo and gear joint definitions
- **Launch Files** to start control node and spawn controllers
- **YAML Configurations** for controller parameters
- Designed to connect with **ESP32 Wokwi simulation** via MQTT

---

## Package Structure
```text
pikes_servo_ros_control/
├── CMakeLists.txt
├── package.xml
├── include/pikes_servo_ros_control/
│   └── esp32_servo_hardware.hpp
├── src/
│   └── esp32_servo_hardware.cpp
├── launch/
│   └── servo_control.launch.py
├── urdf/
│   └── pikes_servo.urdf.xacro
└── config/
    └── controllers.yaml
```

## BUILD & RUN
-   From the root directory, build the package
```
colcon build
source install/setup.sh
```
-   Launch 
```
ros2 launch pikes_servo_ros_control servo_control.launch.py
```

-   Run the node
```
ros2 run pikes_servo_ros_control keyboard_teleop 
```
