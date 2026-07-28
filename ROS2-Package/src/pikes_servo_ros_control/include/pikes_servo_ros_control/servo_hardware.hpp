#pragma once

#include <rclcpp/rclcpp.hpp>
#include <hardware_interface/system_interface.hpp>
#include <hardware_interface/types/hardware_interface_type_values.hpp>
#include <pluginlib/class_list_macros.hpp>

#include <std_msgs/msg/int32.hpp>
#include <std_msgs/msg/int8.hpp>
#include <rclcpp/timer.hpp>
#include <rclcpp/executors/single_threaded_executor.hpp>
#include <thread>
#include <mqtt/async_client.h>


namespace pikes_servo_ros_control
{

// MQTT Callback handler
class MQTTCallback;


/**
 * @brief ROS2 Control hardware interface for ESP32 based smart servo
 * 
 *        This class is the hardware layer inside ros2 control. It receives servo commands from ROS2 controller 
 *        and forward to ESP32-Wokwi via MQTT.
 */
class ESP32ServoHardware : public hardware_interface::SystemInterface
{
public:
  RCLCPP_SHARED_PTR_DEFINITIONS(ESP32ServoHardware)
  ~ESP32ServoHardware() override;

  //Initialize hardware interface 
  hardware_interface::CallbackReturn on_init(const hardware_interface::HardwareInfo & info) override;

  // Activate hardware
  hardware_interface::CallbackReturn on_activate(const rclcpp_lifecycle::State & previous_state) override;

  //Deactivate Hardware
  hardware_interface::CallbackReturn on_deactivate(const rclcpp_lifecycle::State & previous_state) override;

  //State interfaces to ROS2 control
  std::vector<hardware_interface::StateInterface> export_state_interfaces() override;

  //Command interfaces to ROS2 control
  std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;

  // Read from Hardware
  hardware_interface::return_type read(const rclcpp::Time & time, const rclcpp::Duration & period) override;

  // Write from Hardware
  hardware_interface::return_type write(const rclcpp::Time & time, const rclcpp::Duration & period) override;

  // Called by MQTTCallback when a message arrives on a subscribed topic
  void mqtt_message_handler(const std::string & topic, const std::string & payload);

private:
  rclcpp::executors::SingleThreadedExecutor::SharedPtr executor_;
  std::thread spin_thread_;

  // Commands from controller
  double theta_cmd_ = 0.0;
  double speed_cmd_ = 0.0;
  double direction_cmd_ = 0.0;

  double last_theta_cmd_ = 0.0;
  double last_speed_cmd_ = 0.0;
  double last_direction_cmd_ = 0.0;

  // Useful variables
  double current_angle_ = 0.0;
  double current_speed_ = 0.0;
  double current_gear_angle_ = 0.0;


  std::shared_ptr<mqtt::async_client> mqtt_client_;
  std::shared_ptr<MQTTCallback> mqtt_callback_;
  std::string mqtt_server_ = "broker.hivemq.com";
  int mqtt_port = 1883;

  // ROS node + subscriptions
  rclcpp::Node::SharedPtr node_;
  rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr theta_sub_;
  rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr speed_sub_;
  rclcpp::Subscription<std_msgs::msg::Int8>::SharedPtr direction_sub_;

  // Teleop command callbacks
  void theta_callback(const std_msgs::msg::Int32::SharedPtr msg);
  void speed_callback(const std_msgs::msg::Int32::SharedPtr msg);
  void direction_callback(const std_msgs::msg::Int8::SharedPtr msg);
};

/**
 * @brief MQTT callback class used by paho MQTT client
 *        This class receives MQTT messages and forward to hardware interface via mqtt_message_handler()
 */
class MQTTCallback : public virtual mqtt::callback
{
public:
  explicit MQTTCallback(ESP32ServoHardware* hw): hw_(hw)
  {

  }

  void message_arrived(mqtt::const_message_ptr msg) override;

private:
  ESP32ServoHardware* hw_;
};

} // namespace pikes_servo_ros_control
