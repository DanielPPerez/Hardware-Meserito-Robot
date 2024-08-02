#include <WiFi.h>
#include <PubSubClient.h>

// Pines para la comunicación serial con el Arduino
const int RX_PIN = 16; // Pin RX del ESP32 (cualquiera que no esté en uso por default, ej. RX2)
const int TX_PIN = 17; // Pin TX del ESP32 (cualquiera que no esté en uso por default, ej. TX2)

// MQTT Configuración
WiFiClient esp32Client;
PubSubClient mqttClient(esp32Client);

// Sustituir por los datos de vuestro WiFi
const char* ssid = "MIMAR";
const char* password = "BEYMAR15";
const char* server = "broker.emqx.io"; // Reemplaza con tu servidor MQTT
int port = 1883;

void setup() {
  // Configurar comunicación serial
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN); // Inicializar UART2 para comunicación con el Arduino

  // Conectar a WiFi
  connectWiFi();

  // Configurar cliente MQTT
  mqttClient.setServer(server, port);
  mqttClient.setCallback(callback);

  // Conectar a MQTT
  reconnectMQTT();
}

void loop() {
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();
}

void connectWiFi() {
  Serial.print("Conectándose a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("Conectado a WiFi");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Intentando conectarse a MQTT...");

    if (mqttClient.connect("esp32Client")) {
      Serial.println("Conectado");
      // Suscribirse a los temas
      mqttClient.subscribe("robot/commands/move_forward");
      mqttClient.subscribe("robot/commands/stop");
      mqttClient.subscribe("robot/commands/rotate_left");
      mqttClient.subscribe("robot/commands/rotate_right");
      mqttClient.subscribe("robot/commands/search_color");
      mqttClient.subscribe("robot/commands/return");
      mqttClient.subscribe("robot/commands/check_distance");
    } else {
      Serial.print("Fallo, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" intentar de nuevo en 5 segundos");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("]: ");

  char payload_string[length + 1];
  memcpy(payload_string, payload, length);
  payload_string[length] = '\0';
  String command = String(payload_string);

  Serial.println(command); // Mostrar comando recibido

  // Enviar comando al Arduino a través de la comunicación serial
  Serial.print("Enviando comando al Arduino: ");
  Serial.println(command);
  sendCommandToArduino(command);
}

void sendCommandToArduino(String command) {
  Serial2.println(command); // Enviar comando al Arduino
}
