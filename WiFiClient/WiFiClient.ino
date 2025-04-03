#include <WiFi.h>

const char* ssid = "Walter";          // Tu SSID WiFi
const char* password = "Saxomofon1";  // Tu contraseña WiFi

// Configuración del servidor TCP Python
const char* host = "192.168.199.175";  // IP de la PC con el servidor Python
const uint16_t tcpPort = 12345;             // Puerto del servidor Python

int counter = 0;  // Contador para demostración

void setup() {
  Serial.begin(115200);

  // Conectar a WiFi
  Serial.println("\nConectando a WiFi...");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi conectado");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  WiFiClient client;
  
  Serial.print("\nConectando al servidor...");
  
  if (!client.connect(host, tcpPort)) {
    Serial.println(" falló!");
    delay(2000);
    return;
  }
  
  Serial.println(" exitoso!");
  
  // Crear y enviar mensaje
  String mensaje = "Hola servidor desde ESP32 #" + String(counter);
  client.print(mensaje);
  Serial.println("Mensaje enviado: " + mensaje);
  
  // Esperar y recibir respuesta
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 2000) {
      Serial.println("Tiempo de espera agotado!");
      client.stop();
      return;
    }
  }
  
  // Leer respuesta
  String respuesta = client.readStringUntil('\r');
  Serial.print("Respuesta del servidor: ");
  Serial.println(respuesta);
  
  client.stop();
  counter++;
  delay(5000);
}