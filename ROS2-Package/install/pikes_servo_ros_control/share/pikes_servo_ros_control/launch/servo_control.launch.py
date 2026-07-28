from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():

    pkg_share = get_package_share_directory("pikes_servo_ros_control")

    urdf_path = os.path.join(pkg_share, "urdf", "servo_robot.urdf")
    yaml_path = os.path.join(pkg_share, "config", "servo_controllers.yaml")

    with open(urdf_path, 'r') as file:
        robot_description = file.read()

    robot_state_pub = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        parameters=[{"robot_description": robot_description}],
        output="screen"
    )

    controller_manager = Node(
        package="controller_manager",
        executable="ros2_control_node",
        parameters=[
            {"robot_description": robot_description},
            yaml_path
        ],
        output="screen"
    )

    return LaunchDescription([
        robot_state_pub,
        controller_manager
    ])
