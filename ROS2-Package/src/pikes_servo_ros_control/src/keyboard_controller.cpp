#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/int32.hpp>
#include <std_msgs/msg/int8.hpp>
#include <termios.h>
#include <unistd.h>

#define DEFAULT_ANGLE     0U
#define DEFAULT_SPEED     0U
#define DEFAULT_DIRECTION 0U

/**
 * @brief Keyboard teleoperation node for controlling the ESP32 servo
 */
class KeyboardController : public rclcpp::Node
{
public:
  KeyboardController():Node("keyboard_controller"), current_angle_(0), current_speed_(0), current_direction_(0)
  {
    // Publishers for servo command topics
    theta_pub_ = create_publisher<std_msgs::msg::Int32>("pikes_servo/theta_cmd", 10);
    speed_pub_ = create_publisher<std_msgs::msg::Int32>("pikes_servo/speed_cmd", 10);
    direction_pub_ = create_publisher<std_msgs::msg::Int8>("pikes_servo/direction_cmd", 10);

    RCLCPP_INFO(get_logger(), "Keyboard controller started.");
    configureTerminal();
    timer_ = create_wall_timer(std::chrono::milliseconds(100), std::bind(&KeyboardController::loop, this));
  }

  ~KeyboardController()
  {
    restoreTerminal();
  }

private:

  // Internal state variables
  int current_angle_ = DEFAULT_ANGLE;
  int current_speed_ = DEFAULT_SPEED;
  int current_direction_= DEFAULT_DIRECTION;

  // ROS publishers
  rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr theta_pub_;
  rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr speed_pub_;
  rclcpp::Publisher<std_msgs::msg::Int8>::SharedPtr direction_pub_;
  rclcpp::TimerBase::SharedPtr timer_;

  struct termios old_tio_, new_tio_;

  /**
   * @brief Configure terminal for raw keyboard input
   */
  void configureTerminal()
  {
    tcgetattr(STDIN_FILENO, &old_tio_);
    new_tio_ = old_tio_;
    new_tio_.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio_);
  }

  // Restore terminal to normal mode
  void restoreTerminal()
  {
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio_);
  }

  /**
   * @brief Executed by timer
   */
  void loop()
  {
    char ch = 0;
    if(read(STDIN_FILENO, &ch, 1) > 0)
    {
      if(ch == '\033')  
      {
        char seq[2];
        if(read(STDIN_FILENO, &seq[0], 1) == 0) return;
        if(read(STDIN_FILENO, &seq[1], 1) == 0) return;

        if(seq[0] == '[')
        {
          switch (seq[1])
          {
            case 'C':  // RIGHT arrow
              current_angle_ += 1;
              current_direction_ = 1;
              publishAngleDirection();
              break;

            case 'D':  // LEFT arrow
              current_angle_ -= 1;
              current_direction_ = 0;
              publishAngleDirection();
              break;
          }
        }
      }
      else if(ch == '+')
      {
        current_speed_ += 1;
        publishSpeed();
      }
      else if(ch == '-')
      {
        current_speed_ -= 1;
        publishSpeed();
      }
      else if(ch == 'q')
      {
        rclcpp::shutdown();
      }
    }
  }

  /**
   * @brief Publish angle, direction commands
   */

  void publishAngleDirection()
  {
    std_msgs::msg::Int32 angle_msg;
    angle_msg.data = current_angle_;
    theta_pub_->publish(angle_msg);

    std_msgs::msg::Int8 dir_msg;
    dir_msg.data = current_direction_;
    direction_pub_->publish(dir_msg);

    RCLCPP_INFO(get_logger(), "Angle=%d Direction=%d", current_angle_, current_direction_);
  }

  /**
   * @brief Publish spped command
   */
  void publishSpeed()
  {
    std_msgs::msg::Int32 speed_msg;
    speed_msg.data = current_speed_;
    speed_pub_->publish(speed_msg);

    RCLCPP_INFO(get_logger(), "Speed=%d", current_speed_);
  }
};

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<KeyboardController>());
  rclcpp::shutdown();
  return 0;
}
