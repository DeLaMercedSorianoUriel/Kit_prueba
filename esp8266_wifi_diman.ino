#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(3000);
String jsonActual = "{\"estado\":\"Esperando sensores...\"}";
bool servidorActivo = false;

void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); 
  delay(500);
  Serial.println();
  Serial.println("[ESP] LISTO. Esperando comando CONECTAR:...");
}

void loop() {
  // 1. Atender servidor web (si ya estamos conectados)
  if (servidorActivo) {
    server.handleClient();
  }

  // 2. Escuchar al Arduino Mega
  if (Serial.available() > 0) {
    String entrada = Serial.readStringUntil('\n');
    entrada.trim();
    
    // CASO A: Recibir datos de sensores para la web
    if (entrada.startsWith("{")) {
      jsonActual = entrada;
    }
    // CASO B: Recibir orden de conexión
    else if (entrada.startsWith("CONECTAR:")) {
      procesarConexion(entrada);
    }
  }
}

void procesarConexion(String cmd) {
  // Formato recibido: CONECTAR:NombreRed,Contrasena
  String datos = cmd.substring(9); // Quita "CONECTAR:"
  int coma = datos.indexOf(',');
  
  if (coma > 0) {
    String ssid = datos.substring(0, coma);
    String pass = datos.substring(coma + 1);
    
    // Convertir a char array para la librería WiFi
    char s[ssid.length() + 1]; ssid.toCharArray(s, ssid.length()+1);
    char p[pass.length() + 1]; pass.toCharArray(p, pass.length()+1);
    
    WiFi.begin(s, p);
    
    // Intentar conectar (timeout 15 seg)
    int intentos = 0;
    while (WiFi.status() != WL_CONNECTED && intentos < 30) {
      delay(500);
      intentos++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      server.on("/", [](){ server.send(200, "application/json", jsonActual); });
      server.begin();
      servidorActivo = true;
      
      // RESPUESTA DE ÉXITO AL MEGA
      Serial.println();
      Serial.print("[INFO] http://");
      Serial.print(WiFi.localIP());
      Serial.println(":3000");
    } else {
      // RESPUESTA DE ERROR AL MEGA
      Serial.println("[ERROR] Fallo conexion");
    }
  }
}