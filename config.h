// ===== WIFI AND AWS CONFIG ===== //
// const char WIFI_SSID[] = "Johan_PC";
// const char WIFI_PASSWORD[] = "@rduino4141!";
const char WIFI_SSID[] = "Wonderland";
const char WIFI_PASSWORD[] = "christian7795";
const char THINGNAME[] = "waterGuard-tank";
const char MQTT_HOST[] = "a31ru19gk0viac-ats.iot.us-east-1.amazonaws.com";
const char AWS_IOT_PUBLISH_TOPIC[] = "dt/device/tank_01";
const char AWS_IOT_SUBSCRIBE_TOPIC[] = "dt/device/tank_01";

// ===== ULTRASONIDO ===== //
#define TRIG_PIN 15
#define ECHO_PIN 13

// ===== DISPLAY ===== //
#define SSD1306_I2C
#define I2C_ADDR 0x3C
#define I2C_SDA 14
#define I2C_SCL 12

// ===== BUTTONS ===== //
#define BUTTON_UP 5  
#define BUTTON_DOWN 0
#define BUTTON_ENTER 4
