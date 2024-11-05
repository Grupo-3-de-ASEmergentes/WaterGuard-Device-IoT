// ===== VARIABLES DE MEDICIONES ===== //
// Inicialización de valores base y variación
float baseVolume = 300; // Valor inicial aproximado para el volumen (cm)
float baseTemperature = 25.0; // Valor inicial aproximado para la temperatura (°C)
float baseQuality = 7.0; // Valor inicial aproximado para el pH
float volumeChangeRate = 0.5; // Cambio gradual en volumen
float temperatureChangeRate = 0.1; // Cambio gradual en temperatura
float qualityChangeRate = 0.05; // Cambio gradual en calidad (pH)

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