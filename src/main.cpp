#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>

#define ledWifi D4
Ticker tic_ledWifi;

byte cont = 0;
byte max_intentos = 50;

void parpadeoLed() {
  byte estado = digitalRead(ledWifi);
  digitalWrite(ledWifi, !estado);
}

int pinLed = 2;
boolean Estado = false;
boolean BombaRemotaEncendida = false;

const uint32_t TiempoEsperaWifi = 5000;

unsigned long TiempoActual = 0;
unsigned long TiempoAnterior = 0;
const long TiempoCancelacion = 500;

const char* ssid = "Personal-110";
const char* password = "AEKk6yXPtG";

WiFiServer server(80);

const String Pagina = R"====(
<!DOCTYPE HTML>
<html>

<head>
  <title>Control de Bomba</title>
  <style>
    button {
      font-size: 20px;
      margin: 10px;
    }
  </style>
  <script>
    function enviarComando(comando) {
      var xmlhttp = new XMLHttpRequest();
      xmlhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
          console.log(this.responseText);
          document.getElementById('estadoBomba').innerHTML = "Estado de la bomba: " + (comando === 'encender' ? 'Encendida' : 'Apagada');
        }
      };
      xmlhttp.open("GET", "/" + comando, true);
      xmlhttp.send();
    }
  </script>
</head>

<body>

  <h2>Control de Bomba</h2>

  <button onclick="enviarComando('encender')">Encender Bomba</button>
  <button onclick="enviarComando('apagar')">Apagar Bomba</button>

  <p id="estadoBomba">Estado de la bomba: Desconocido</p>

</body>

</html>
)====";

void VerificarMensaje(String Mensaje) {
  if (Mensaje.indexOf("GET /encender") >= 0) {
    Serial.println("Encender bomba");
    Estado = true;
  } else if (Mensaje.indexOf("GET /apagar") >= 0) {
    Serial.println("Apagar bomba");
    Estado = false;
  }
}

void ResponderCliente(WiFiClient& cliente) {
  cliente.println("HTTP/1.1 200 OK");
  cliente.println("Content-Type: text/html");
  cliente.println("Connection: close");

  cliente.println("");
  cliente.println(Pagina);
}

void setup() {
  pinMode(ledWifi, OUTPUT);

  Serial.begin(9600);
  Serial.println("\n");

  tic_ledWifi.attach(0.2, parpadeoLed);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED and cont < max_intentos) {
    cont++;
    delay(500);
    Serial.print(".");
  }

  Serial.println("");

  if (cont < max_intentos) {
    Serial.println("************");
    Serial.print("Conectado a la red WiFi: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("MacAdress: ");
    Serial.println(WiFi.macAddress());
    Serial.println("************");
    digitalWrite(ledWifi, LOW);
  } else {
    Serial.println("************");
    Serial.println("Error de conexión");
    Serial.println("************");
  }

  tic_ledWifi.detach();

  server.begin();
}

void loop() {
  WiFiClient cliente = server.accept();

  if (cliente) {
    Serial.println("Nuevo Cliente");
    TiempoActual = millis();
    TiempoAnterior = TiempoActual;
    String LineaActual = "";

    while (cliente.connected() && TiempoActual - TiempoAnterior <= TiempoCancelacion) {
      if (cliente.available()) {
        TiempoActual = millis();
        char Letra = cliente.read();
        if (Letra == '\n') {
          if (LineaActual.length() == 0) {
            digitalWrite(pinLed, Estado);
            // Responde al cliente
            ResponderCliente(cliente);
            break;
          } else {
            Serial.println(LineaActual);
            VerificarMensaje(LineaActual);
            LineaActual = "";
          }
        }  else if (Letra != '\r') {
          LineaActual += Letra;
        }
      }
    }

    cliente.stop();
    Serial.println("Cliente Desconectado");
    Serial.println();
  }

  // Verifica si la bomba remota está encendida y realiza la acción correspondiente
  if (BombaRemotaEncendida) {
    digitalWrite(pinLed, HIGH);
  } else {
    digitalWrite(pinLed, LOW);
  }
}

