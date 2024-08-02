#include <ArduinoJson.h> // Asegúrate de tener instalada la librería ArduinoJson

// Pines para el sensor de color
const int S0 = 11;
const int S1 = 10;
const int S2 = 13;
const int S3 = 12;
const int OUT = 9;

// Pines para el seguimiento de línea
const int Left_Tra_Pin = 7;
const int Center_Tra_Pin = 8;
const int Right_Tra_Pin = A1;
const int Black_Line = 1; // Definir el valor de línea negra

// Pines para el sensor de ultrasonido
const int trigPin = 12; // Pin del trigger del sensor de ultrasonido
const int echoPin = 13; // Pin del echo del sensor de ultrasonido
const float obstacleDistanceThreshold = 20.0; // Distancia en cm para considerar un obstáculo

// Temporizadores y estados
const unsigned long colorDetectionTime = 2000; // 2 segundos
unsigned long lastMoveTime = 0;
bool isMoving = false;
String targetColor = "yellow";

// Umbrales para la detección de color
const int redThreshold = 3000; // Ajusta este valor según sea necesario
const int blueThreshold = 3000; // Ajusta este valor según sea necesario
const int greenThreshold = 3000; // Ajusta este valor según sea necesario
const int yellowThreshold = 3000; // Ajusta este valor según sea necesario

void setup() {
  // Configurar pines
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(OUT, INPUT);
  pinMode(Left_Tra_Pin, INPUT);
  pinMode(Center_Tra_Pin, INPUT);
  pinMode(Right_Tra_Pin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(2, OUTPUT); // Motor A
  pinMode(5, OUTPUT); // Motor A speed
  pinMode(4, OUTPUT); // Motor B
  pinMode(6, OUTPUT); // Motor B speed

  // Configurar el sensor de color para la escala de frecuencia máxima
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  Serial.begin(115200); // Inicializar comunicación serial con el ESP32
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    Serial.print("Comando recibido: ");
    Serial.println(command);

    // Crear un objeto JSON
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, command);

    if (error) {
      Serial.print("Error al analizar el JSON: ");
      Serial.println(error.c_str());
      return;
    }

    // Obtener la acción y el valor del JSON
    String action = doc["action"];
    int value = doc["value"];

    // Ejecutar el comando
    if (action == "stop") {
      Serial.println("Ejecutando comando: STOP");
      Stop();
    } else if (action == "move_forward") {
      Serial.print("Ejecutando comando: MOVE_FORWARD con valor ");
      Serial.println(value);
      Move_Forward(value);
      isMoving = true;
      lastMoveTime = millis();
    } else if (action == "rotate_left") {
      Serial.print("Ejecutando comando: ROTATE_LEFT con valor ");
      Serial.println(value);
      Rotate_Left(value);
    } else if (action == "rotate_right") {
      Serial.print("Ejecutando comando: ROTATE_RIGHT con valor ");
      Serial.println(value);
      Rotate_Right(value);
    } else if (action.startsWith("search_color")) {
      Serial.print("Ejecutando comando: SEARCH_COLOR con color ");
      targetColor = doc["color"].as<String>(); // Extrae el color del JSON
      Serial.println(targetColor);
      Move_Forward(70);
      isMoving = true;
      lastMoveTime = millis();
    } else if (action == "return") {
      Serial.println("Ejecutando comando: RETURN");
      Return_To_Start();
    } else if (action == "check_distance") {
      Serial.println("Ejecutando comando: CHECK_DISTANCE");
      float distance = checkDistance();
      Serial.print("Distance: ");
      Serial.println(distance);
    }
  }

  if (isMoving && millis() - lastMoveTime >= colorDetectionTime) {
    Serial.println("Deteniendo después de moverse para detección de color.");
    Stop();
    isMoving = false;
  }

  if (targetColor != "none") {
    String detectedColor = readColor();
    if (detectedColor == targetColor) {
      Serial.print("Color objetivo ");
      Serial.print(targetColor);
      Serial.println(" encontrado.");
      Stop();
      targetColor = "none";
    }
  }
}

void Move_Forward(int speed) {
  Serial.println("Moviendo hacia adelante");
  digitalWrite(2, HIGH); // Motor A hacia adelante
  analogWrite(5, speed); // Velocidad del Motor A
  digitalWrite(4, LOW); // Motor B hacia adelante
  analogWrite(6, speed); // Velocidad del Motor B
}

void Stop() {
  Serial.println("Deteniendo");
  digitalWrite(2, LOW); // Detener Motor A
  analogWrite(5, 0); // Velocidad del Motor A a 0
  digitalWrite(4, LOW); // Detener Motor B
  analogWrite(6, 0); // Velocidad del Motor B a 0
}

void Rotate_Right(int speed) {
  Serial.println("Girando a la derecha");
  digitalWrite(2, HIGH); // Motor A hacia adelante
  analogWrite(5, speed); // Velocidad del Motor A
  digitalWrite(4, HIGH); // Motor B hacia atrás
  analogWrite(6, speed); // Velocidad del Motor B
}

void Rotate_Left(int speed) {
  Serial.println("Girando a la izquierda");
  digitalWrite(2, LOW); // Motor A hacia atrás
  analogWrite(5, speed); // Velocidad del Motor A
  digitalWrite(4, LOW); // Motor B hacia adelante
  analogWrite(6, speed); // Velocidad del Motor B
}

void Return_To_Start() {
  Serial.println("Regresando al inicio...");
  Stop();
  delay(2000); // Espera para asegurar que el carrito está parado
  Rotate_Left(70); // Gira a la izquierda para regresar al inicio
  delay(2000); // Ajusta el tiempo según sea necesario
  Stop();
}

float checkDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  float distance = duration * 0.0344 / 2; // Convertir a distancia en cm

  return distance;
}

String readColor() {
  // Lee el color del sensor de color y devuelve el color detectado
  int redValue = analogRead(OUT); // Reemplaza con la lectura real del sensor
  if (redValue > redThreshold) {
    return "red";
  }
  return "none";
}
