/**
 * @file sketch.ino
 * @brief This file contains the main code for controlling an ESP32 servo and gear mechanism.
 *        It connects to a WiFi network, subscribes to MQTT topics, and controls the servo based on received messages. 
 *        The code also simulates torque and detects endpoints for the door mechanism.
 */

#include <WiFi.h>
#include <ESP32Servo.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/******************************** MACROS **************************************/

#define MQTT_PORT               1883U
#define DEFAULT_SPEED           30U
#define DEFAULT_SERVO_ANGLE     0U
#define DEFAULT_SERVO_ANGLE_MAX 360U

#define SERVO_PIN               4U
#define GEAR_SERVO_PIN          5U

#define SERVO_ANGLE_MAX_RIGHT   360U
#define SERVO_ANGLE_MAX_LEFT    -360U

#define DELAY_MS                1000U
#define DELAY_100MS             100U

#define DOOR_PHYSICAL_RANGE     60U

#define SCREEN_WIDTH            128 
#define SCREEN_HEIGHT           64 
/******************************** GLOBAL VARIABLES ******************************/
Servo ESP32Servo;
Servo ESP32ServoGear;

WiFiClient espClient;
PubSubClient mqtt(espClient);

const char* mqttServer = "broker.hivemq.com";

typedef enum {
  SERVO_LEFT,
  SERVO_RIGHT
}Servo_direction_t;

static uint8_t ESP32_servo_speed = DEFAULT_SPEED;
static bool ESP32_servo_direction = SERVO_RIGHT;
static int16_t ESP32_servo_angle = DEFAULT_SERVO_ANGLE; 

const float Door_angle_max = 180.0f;  // Maximum angle for the door
const float gear_ratio = 40.0/15.0;   // Gear ratio for the servo gear
const float Door_range_max = 60.0f;

const float TORQUE_THRESHOLD = 0.75f;

static float  random_min_door   = 0.0f;
static float  random_max_door   = 60.0f;

static int16_t  detected_min_door_angle = 0.0f;
static int16_t  detected_max_door_angle = 60.0f;

static int16_t detected_min_servo_angle = 0;
static int16_t detected_max_servo_angle = SERVO_ANGLE_MAX_RIGHT;

static bool door_angle_change_flag = true;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/****************************** FUNCTIONS ******************************/
/**
 * @brief Callback function for MQTT messages
 */
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  String subscribed_topic = String(topic);
  for(int i = 0; i < length; i++) 
  {
    message += (char)payload[i]; 
  }
  if(subscribed_topic == "ESP32Servo/theta")
  {
    Serial.println("ESP32 angle Received: " + message);
    ESP32_servo_angle = message.toInt();
  }
  else if(subscribed_topic == "ESP32Servo/speed")
  {
    Serial.println("ESP32 Speed Received: " + message);
    ESP32_servo_speed = message.toInt();
  }
  else if(subscribed_topic == "ESP32Servo/direction")
  {
    Serial.println("ESP32 Direction Received: " + message);
    if (message.toInt() == SERVO_LEFT){
      ESP32_servo_direction = SERVO_LEFT;
    }
  }
}

/**
 * @brief Setup function for MQTT connection
 */
void mqtt_setup() {
  mqtt.setServer(mqttServer, MQTT_PORT);
  mqtt.setCallback(mqtt_callback);

  while (!mqtt.connected()) {
    Serial.println("Connecting to MQTT...");

    String clientId = "ESP32Servo-PIKES";

    if (mqtt.connect(clientId.c_str())) {
      Serial.println("MQTT Connected!");
    } else {
      Serial.print("Failed with:");
      Serial.println(mqtt.state());
      delay(DELAY_MS);
    }
  }

  mqtt.subscribe("ESP32Servo/direction");
  mqtt.subscribe("ESP32Servo/speed");
  mqtt.subscribe("ESP32Servo/theta");
}

/**
 * @brief Setup function for WiFi connection
 */
void WiFi_setup() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin("Wokwi-GUEST", "", 6);

  while (WiFi.status() != WL_CONNECTED) {
    delay(DELAY_100MS);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");
}


/**
 * @brief Function to generate random end points for the door
 *        The end points are in the range and <= 60 degrees apart
 */
void generate_random_end_points(){
  float base = random(0, 20);
  random_min_door = base;
  random_max_door = base + 40.0f;
  Serial.println("Random Point Min: "); 
  Serial.println(random_min_door);
  Serial.println("Random Point Max: "); 
  Serial.println(random_max_door);
}

/*
 * @brief Function to simulate torque based on the door angle
 * @param door_angle The current angle of the door
 * @return The simulated torque value
*/
float simulate_torque(float door_angle)
{
    float torque = 0.1f;
    float d_min = fabs(door_angle - random_min_door);
    float d_max = fabs(door_angle - random_max_door);
    float torque_min = 1.0f - (d_min / 5.0f);
    float torque_max = 1.0f - (d_max / 5.0f);

    torque_min = constrain(torque_min, 0.0f, 1.0f);
    torque_max = constrain(torque_max, 0.0f, 1.0f);

    return max(torque_min, torque_max);
}



/**
 * @brief Function to detect the minimum and maximum endpoints for the door
 *        based on the simulated torque values
 */
void detect_endpoints(){
  Serial.println("Starting Min end point detection...");

  bool min_endpoint_found = false;
  bool max_endpoint_found = false;

  for(int16_t servo_angle = DEFAULT_SERVO_ANGLE; servo_angle <= DEFAULT_SERVO_ANGLE_MAX; servo_angle++)
  {
    float door_angle_raw = servo_angle * gear_ratio;
    float door_angle = constrain(door_angle_raw, 0.0f, Door_range_max);
    float torque = simulate_torque(door_angle);

    ESP32Servo.write(servo_angle);
    ESP32ServoGear.write(door_angle);

    if(torque >= TORQUE_THRESHOLD)
    {
      detected_min_door_angle = door_angle;
      detected_min_servo_angle = servo_angle;
      min_endpoint_found = true;
      break;
    }
  }
  Serial.println("Starting Max end point detection...");
  for(int16_t servo_angle = DEFAULT_SERVO_ANGLE_MAX; servo_angle >= DEFAULT_SERVO_ANGLE; servo_angle--){
    float door_angle_raw = servo_angle * gear_ratio;
    float door_angle = constrain(door_angle_raw, 0.0f, Door_range_max);
    float torque = simulate_torque(door_angle);

    ESP32Servo.write(servo_angle);
    ESP32ServoGear.write(door_angle);

    if(torque >= TORQUE_THRESHOLD)
    {
      detected_max_door_angle = door_angle;
      detected_max_servo_angle = servo_angle;
      max_endpoint_found = true;
      break;
    }
  }

  if(!min_endpoint_found){
    detected_min_door_angle = random_min_door;
  }
  if(!max_endpoint_found){
    detected_max_door_angle = random_max_door;
  }
  
  Serial.println("Detected endpoints Min: ");
  Serial.println(detected_min_door_angle); 
  Serial.println("Detected endpoints Max: ");
  Serial.println(detected_max_door_angle);

  delay(DELAY_100MS);
  
}

/**
 * @brief Function to run the ESP32 servo and gear mechanism
 */
void ESP32_servo_run(){
    char buffer[12];
    mqtt.loop();
    delay(ESP32_servo_speed);
    if(ESP32_servo_angle == SERVO_ANGLE_MAX_RIGHT || ESP32_servo_angle == SERVO_ANGLE_MAX_LEFT)
    {
        ESP32_servo_angle = DEFAULT_SERVO_ANGLE;
    }
    ESP32Servo.write(ESP32_servo_angle);

    float gear_angle_raw = gear_ratio * ESP32_servo_angle;
    float gear_angle = constrain(gear_angle_raw, 0.0f, Door_angle_max);

    if((gear_angle > detected_min_door_angle) && (gear_angle < detected_max_door_angle))
    {
      ESP32ServoGear.write(gear_angle);
      sprintf(buffer, "%.2f", gear_angle);
      mqtt.publish("ESP32ServoGear/gear_angle_back", buffer);
    }
    sprintf(buffer, "%d", ESP32_servo_angle);
    mqtt.publish("ESP32Servo/angle_back", buffer);

    sprintf(buffer, "%d", ESP32_servo_speed);
    mqtt.publish("ESP32Servo/speed_back", buffer);
}

/**
 * @brief Setup function for the OLED display
 */
void oled_setup(){
  Wire.begin(21, 22);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for(;;);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("PIKES Servo");
  display.display();
  delay(1000);
}

/**
 * @brief Function to display detected endpoints on the OLED display
 */
void display_detected_endpoints(){
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("Min:");
  display.println(detected_min_door_angle);

  display.setCursor(0, 30);
  display.print("Max:");
  display.println(detected_max_door_angle);

  display.display();
}

/**
 * @brief Setup function for the ESP32 servo and gear mechanism
 */
void setup() {
  Serial.begin(115200);

  ESP32Servo.attach(SERVO_PIN);
  ESP32ServoGear.attach(GEAR_SERVO_PIN);
  Serial.println("Servo attached to pin 4 and Gear pin 5");

  oled_setup();
  WiFi_setup();
  delay(DELAY_MS);
  mqtt_setup();

  generate_random_end_points();
  delay(DELAY_MS);
  detect_endpoints();

  ESP32Servo.write(detected_min_servo_angle);
  ESP32ServoGear.write(detected_min_door_angle);
  display_detected_endpoints();
}

/**
 * @brief Main loop function for the ESP32 servo and gear mechanism
 */
void loop() {
  if (!mqtt.connected()) {
    mqtt_setup();
  }
  mqtt.loop();
  ESP32_servo_run();
}

