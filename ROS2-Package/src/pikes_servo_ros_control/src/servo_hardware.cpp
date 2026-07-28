#include "pikes_servo_ros_control/servo_hardware.hpp"


namespace pikes_servo_ros_control
{

/**
 * @brief MQTT callback entry point
 *        Paho MQTT invokes this, whnever a subscribed topic receives a message  
 */
void MQTTCallback::message_arrived(mqtt::const_message_ptr msg)
{
  hw_->mqtt_message_handler(msg->get_topic(), msg->to_string());
}

/**
 * @brief Init hardware interface
 */
hardware_interface::CallbackReturn ESP32ServoHardware::on_init(const hardware_interface::HardwareInfo& info)
{
  if(SystemInterface::on_init(info) != hardware_interface::CallbackReturn::SUCCESS)
  {
    return hardware_interface::CallbackReturn::ERROR;
  }

  // Create ROS node
  node_ = rclcpp::Node::make_shared("esp32_servo_hardware");

  // MQTT
  std::string mqtt_address = "tcp://" + mqtt_server_ + ":" + std::to_string(mqtt_port);
  mqtt_client_ = std::make_shared<mqtt::async_client>(mqtt_address, "ROS2 Servo Control");
  mqtt::connect_options connOpts;
  connOpts.set_clean_session(true);

  mqtt_callback_ = std::make_shared<MQTTCallback>(this);
  mqtt_client_->set_callback(*mqtt_callback_);

  try{
    mqtt_client_->connect(connOpts)->wait();
    mqtt_client_->subscribe("ESP32Servo/angle_back", 1);
    mqtt_client_->subscribe("ESP32Servo/speed_back", 1);
    RCLCPP_INFO(node_->get_logger(), "MQTT Connection Success");
  }catch(const mqtt::exception &exception){
    RCLCPP_ERROR(node_->get_logger(), "MQTT Connection Failed: %s", exception.what());
  }

  // Subscriptions for teleop commands
  theta_sub_ = node_->create_subscription<std_msgs::msg::Int32>("pikes_servo/theta_cmd", 10, std::bind(&ESP32ServoHardware::theta_callback, this, std::placeholders::_1));
  speed_sub_ = node_->create_subscription<std_msgs::msg::Int32>("pikes_servo/speed_cmd", 10, std::bind(&ESP32ServoHardware::speed_callback, this, std::placeholders::_1));
  direction_sub_ = node_->create_subscription<std_msgs::msg::Int8>("pikes_servo/direction_cmd", 10, std::bind(&ESP32ServoHardware::direction_callback, this, std::placeholders::_1));

  // Spin the internal node
  executor_ = std::make_shared<rclcpp::executors::SingleThreadedExecutor>();
  executor_->add_node(node_);
  spin_thread_ = std::thread([this]() { executor_->spin(); });

  RCLCPP_INFO(node_->get_logger(), "ESP32 Servo Hardware initialized");
  return hardware_interface::CallbackReturn::SUCCESS;
}

ESP32ServoHardware::~ESP32ServoHardware()
{
  if(executor_)
  {
    executor_->cancel();
  }
  if(spin_thread_.joinable())
  {
    spin_thread_.join();
  }
  if(mqtt_client_ && mqtt_client_->is_connected())
  {
    try{
      mqtt_client_->disconnect()->wait();
    }catch(const mqtt::exception &){
    }
  }
}

/**
 * @brief called by controller manager on activate
 */
hardware_interface::CallbackReturn ESP32ServoHardware::on_activate(const rclcpp_lifecycle::State &)
{
  RCLCPP_INFO(node_->get_logger(), "ESP32 Servo Hardware activated");
  return hardware_interface::CallbackReturn::SUCCESS;
}


/**
 * @brief called by controller manager on deactivate
 */
hardware_interface::CallbackReturn ESP32ServoHardware::on_deactivate(const rclcpp_lifecycle::State &)
{
  RCLCPP_INFO(node_->get_logger(), "ESP32 Servo Hardware deactivated");
  return hardware_interface::CallbackReturn::SUCCESS;
}


/**
 * @brief Export state interfaces to ros2 control
 */
std::vector<hardware_interface::StateInterface> ESP32ServoHardware::export_state_interfaces()
{
  std::vector<hardware_interface::StateInterface> interfaces;
  interfaces.emplace_back(hardware_interface::StateInterface("servo_joint", "angle", &current_angle_));
  interfaces.emplace_back(hardware_interface::StateInterface("servo_joint", "speed", &current_speed_));
  return interfaces;
}

/**
 * @brief Export command interfaces to ros2 control
 */
std::vector<hardware_interface::CommandInterface> ESP32ServoHardware::export_command_interfaces()
{
  std::vector<hardware_interface::CommandInterface> interfaces;
  interfaces.emplace_back(hardware_interface::CommandInterface("servo_joint", "theta_cmd", &theta_cmd_));
  interfaces.emplace_back(hardware_interface::CommandInterface("servo_joint", "speed_cmd", &speed_cmd_));
  interfaces.emplace_back(hardware_interface::CommandInterface("servo_joint", "direction_cmd", &direction_cmd_));
  return interfaces;
}

/**
 * @brief Read hardware state
 */
hardware_interface::return_type ESP32ServoHardware::read(const rclcpp::Time &, const rclcpp::Duration &)
{
  //RCLCPP_INFO(node_->get_logger(), "READ → angle=%f speed=%f", current_angle_, current_speed_);
  return hardware_interface::return_type::OK;
}

/**
 * @brief Write commands to MQTT, only when data change in commands
 */
hardware_interface::return_type ESP32ServoHardware::write(const rclcpp::Time &, const rclcpp::Duration &)
{
  if(theta_cmd_ != last_theta_cmd_ || speed_cmd_ != last_speed_cmd_ || direction_cmd_ != last_direction_cmd_)
  {
    RCLCPP_INFO(node_->get_logger(), "theta = %f    speed = %f    direction = %f", theta_cmd_, speed_cmd_, direction_cmd_);

    last_theta_cmd_ = theta_cmd_;
    last_speed_cmd_ = speed_cmd_;
    last_direction_cmd_ = direction_cmd_;

    if(mqtt_client_ && mqtt_client_->is_connected())
    {
      try{
        mqtt_client_->publish("ESP32Servo/theta", std::to_string((int)last_theta_cmd_), 1, false);
        mqtt_client_->publish("ESP32Servo/speed", std::to_string((int)last_speed_cmd_), 1, false);
        mqtt_client_->publish("ESP32Servo/direction", std::to_string((int)last_direction_cmd_), 1, false);
      }catch (const mqtt::exception & exception){
        RCLCPP_ERROR(node_->get_logger(), "MQTT publish failed: %s", exception.what());
      }
    }
  }
  return hardware_interface::return_type::OK;
}


// Teleop command callbacks
 
void ESP32ServoHardware::theta_callback(const std_msgs::msg::Int32::SharedPtr msg)
{
  theta_cmd_ = msg->data;
  RCLCPP_INFO(node_->get_logger(), "Received theta_cmd: %f", theta_cmd_);
}

void ESP32ServoHardware::speed_callback(const std_msgs::msg::Int32::SharedPtr msg)
{
  speed_cmd_ = msg->data;
  RCLCPP_INFO(node_->get_logger(), "Received speed_cmd: %f", speed_cmd_);
}

void ESP32ServoHardware::direction_callback(const std_msgs::msg::Int8::SharedPtr msg)
{
  direction_cmd_ = static_cast<double>(msg->data);
  RCLCPP_INFO(node_->get_logger(), "Received direction_cmd: %f", direction_cmd_);
}

/**
 * @brief Handle incoming MQTT messages from ESP32
 */
void ESP32ServoHardware::mqtt_message_handler(const std::string & topic, const std::string & payload)
{
  try{
    if(topic == "ESP32Servo/angle_back")
    {
      current_angle_ = std::stod(payload);
      RCLCPP_INFO(node_->get_logger(), "ESP32 Servo Angle : %f", current_angle_);
    }
    else if(topic == "ESP32Servo/speed_back")
    {
      current_speed_ = std::stod(payload);
      RCLCPP_INFO(node_->get_logger(), "ESP32 Servo Speed : %f", current_speed_);
    }
    else if(topic == "ESP32ServoGear/gear_angle_back")
    {
      current_gear_angle_ = std::stod(payload);
      RCLCPP_INFO(node_->get_logger(), "ESP32 Servo Gear Angle : %f", current_gear_angle_);
    }
  }catch (const std::exception & exception){
    RCLCPP_ERROR(node_->get_logger(), "Failed to parse MQTT payload on '%s': %s", topic.c_str(), exception.what());
  }
}

} // namespace pikes_servo_ros_control

// Register plugin with pluginlib
PLUGINLIB_EXPORT_CLASS(pikes_servo_ros_control::ESP32ServoHardware, hardware_interface::SystemInterface)
