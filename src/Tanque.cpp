#include <ESP8266WiFi.h>

const char* ssid = "Personal-110";
const char* password = "AEKk6yXPtG";
const char* serverIP = "192.168.1.38"; // Cambia esto con la IP del ESP8266 que controla la bomba
const int serverPort = 80;

const int flotantePin = D5;  // Cambiar al pin correcto según la conexión física

void enviarSenalRemota(String comando) {
  WiFiClient client;

  Serial.print("Conectando a ");
  Serial.print(serverIP);
  Serial.print(":");
  Serial.println(serverPort);

  if (client.connect(serverIP, serverPort)) {
    Serial.println("Conectado");
    client.print("GET /");
    client.print(comando);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(serverIP);
    client.println("Connection: close");
    client.println();

    while (client.connected()) {
      if (client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);
      }
    }
    client.stop();
    Serial.println("Desconectado");
  } else {
    Serial.println("Error de conexión");
  }
}

void setup() {
  Serial.begin(9600);
  delay(10);

  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conectado a la red WiFi");
}

void loop() {
  // Puedes cambiar esto con tu propia lógica para encender y apagar la bomba
  boolean bombaEncendida = digitalRead(flotantePin) == HIGH;

  if (bombaEncendida) {
    // Envía la señal para encender la bomba
    enviarSenalRemota("encender");
  } else {
    // Envía la señal para apagar la bomba
    enviarSenalRemota("apagar");
  }

  // Considera usar un enfoque sin delay con millis() si es necesario
  delay(5000); // Espera 5 segundos antes de enviar la siguiente señal
}
