#include <U8g2lib.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"  // Archivo de configuración para pines y credenciales
#include "env.h"     // Archivo que contiene los certificados y claves para AWS IoT

// ===== PANTALLA OLED ===== //
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oledDisplay(U8G2_R0, U8X8_PIN_NONE, I2C_SCL, I2C_SDA); 

// ===== VARIABLES DEL MENÚ ===== //
int menuIndex = 0;
const int menuLength = 3;
String menuItems[menuLength] = {"Ver Volumen", "Ver Temperatura", "Ver pH"};
String items[menuLength] = {"Volumen:", "Temperatura:", "Calidad (pH):"};
bool inMeasurementView = false;

// Intervalo de publicación
const long interval = 2000;

// Zona horaria
const int8_t TIME_ZONE = -5;

// Último tiempo de envío
unsigned long lastMillis = 0;

// Objetos para comunicación segura
WiFiClientSecure net;
BearSSL::X509List cert(cacert);          // Certificado CA
BearSSL::X509List client_crt(client_cert); // Certificado del cliente
BearSSL::PrivateKey key(privkey);          // Clave privada
PubSubClient client(net);

// ===== VARIABLES DE SENSORES ===== //
// Sensor de ultrasonido
long duration;
int volume;

// Valores simulados de temperatura y pH
float temperature;
float quality;

// ===== FUNCIONES DEL MENÚ ===== //
void menuView() {
  oledDisplay.clearBuffer();  
  for (int i = 0; i < menuLength; i++) {
    if (i == menuIndex) {
      oledDisplay.setDrawColor(1);
      oledDisplay.drawBox(0, i * 14, 128, 14);
      oledDisplay.setDrawColor(0);
    } else {
      oledDisplay.setDrawColor(1);
    }
    oledDisplay.setFont(u8g2_font_7x14B_tr);
    oledDisplay.setCursor(5, (i + 1) * 14);
    oledDisplay.print(menuItems[i]);
  }
  oledDisplay.sendBuffer();  
}

void menuUp() {
  if (menuIndex > 0) {
    menuIndex--;
  } else {
    menuIndex = menuLength - 1;
  }
  menuView();
}

void menuDown() {
  if (menuIndex < menuLength - 1) {
    menuIndex++;
  } else {
    menuIndex = 0;
  }
  menuView();
}

void measurementView() {
  oledDisplay.clearBuffer();
  oledDisplay.setDrawColor(1);
  oledDisplay.setFont(u8g2_font_7x14B_tr);
  oledDisplay.setCursor(5, 20);
  oledDisplay.print(items[menuIndex]);
  oledDisplay.setCursor(5, 40);
  if (menuIndex == 0) {
    oledDisplay.print(volume);
    oledDisplay.print(" cm");
  } else if (menuIndex == 1) {
    oledDisplay.print(temperature);
    oledDisplay.print(" °C");
  } else {
    oledDisplay.print(quality);
    oledDisplay.print(" pH");
  }
  oledDisplay.sendBuffer();
}

// ===== FUNCIONES DE SENSORES ===== //
// Generar valores aleatorios para temperatura y pH
void generateValues() {
  temperature = random(180, 350) / 10.0;  // De 18.0 a 35.0 °C
  quality = random(60, 90) / 10.0;             // De 6.0 a 9.0 pH
}

// Medir volumen usando el sensor de ultrasonido
void measureVolume() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  volume = duration * 0.034 / 2;
}

// ===== FUNCIONES DE AWS IoT ===== //
// Función de callback para mensajes recibidos
void messageReceived(char *topic, byte *payload, unsigned int length) {
  Serial.print("Recibido [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Conectar al servidor NTP y establecer la hora
void NTPConnect() {
  Serial.print("Estableciendo hora usando SNTP");
  configTime(TIME_ZONE * 3600, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  while (now < 1510592825) { // Fecha mínima: 13 de noviembre de 2017
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("¡Hecho!");
}

// Publicar datos a AWS IoT
void publishMessage() {
  StaticJsonDocument<200> doc;
  doc["volume"] = volume;
  doc["temperature"] = temperature;
  doc["quality"] = quality;
  doc["device_id"] = "tank_01";

  char jsonBuffer[200];
  serializeJson(doc, jsonBuffer);
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

// Conectar a AWS IoT Core
void connectAWS() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println(String("Intentando conectar a SSID: ") + String(WIFI_SSID));

  // Esperar conexión WiFi
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\n¡Conectado a WiFi!");

  // Conectar al servidor NTP para establecer la hora
  NTPConnect();

  // Configurar certificados y claves para comunicación segura
  net.setTrustAnchors(&cert);
  net.setClientRSACert(&client_crt, &key);

  // Configurar cliente MQTT
  client.setServer(MQTT_HOST, 8883);
  client.setCallback(messageReceived);

  Serial.println("Conectando a AWS IoT...");

  // Intentar conectar a AWS IoT Core
  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(500);
  }

  // Verificar si la conexión fue exitosa
  if (!client.connected()) {
    Serial.println("¡Tiempo de espera agotado al conectar a AWS IoT!");
    return;
  }

  // Suscribirse al tópico MQTT
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  Serial.println("\n¡Conectado a AWS IoT!");
}

void setup() {
  // Inicializar comunicación serial
  Serial.begin(9600);

  // Inicializar pantalla OLED
  oledDisplay.begin();
  oledDisplay.clearBuffer();
  menuView();
  
  // Configurar pines de botones
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_ENTER, INPUT_PULLUP);

  // Configurar pines del sensor de ultrasonido
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Conectar a AWS IoT
  connectAWS();
}

void loop() {
  measureVolume();
  generateValues();

  // Publicar datos a intervalos regulares
  if (millis() - lastMillis > interval) {
    lastMillis = millis();
    if (client.connected()) {
      // Publish message
      publishMessage();
      Serial.print("Volumen: ");
      Serial.print(volume);
      Serial.print(" cm, Temp: ");
      Serial.print(temperature);
      Serial.print(" °C, pH: ");
      Serial.println(quality);
    }else{
      Serial.print("Error de Conexion");
    }
  }

  // Control del menú
  if (digitalRead(BUTTON_UP) == LOW) {
    if(!inMeasurementView){
      menuUp();
    }
  } else if (digitalRead(BUTTON_DOWN) == LOW) {
    if(!inMeasurementView){
      menuDown();
    }
  } else if (digitalRead(BUTTON_ENTER) == LOW) {
    if(!inMeasurementView){
      inMeasurementView = true;
    }else{
      inMeasurementView = false;
      menuView();
    }
  }

  // Mostrar las medidas continuamente si estamos en una vista de medición
  if (inMeasurementView) {
    measurementView();
  }

  // Mantener conexión MQTT
  client.loop();
}


