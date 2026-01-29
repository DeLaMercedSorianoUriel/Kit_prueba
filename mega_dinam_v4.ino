// --- CÓDIGO MEGA V24: VISUALIZADOR DE IP Y PUERTO ---

#include <DHT.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define PIN_SUELO A0       
#define PIN_DHT 2          
#define DHTTYPE DHT11      

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(PIN_DHT, DHTTYPE);

const int INTERVALO_ENVIO = 2000; 
unsigned long tiempoAnterior = 0;
String ipServidor = "---";
bool sistemaOnline = false;

// Variables Globales
String redGuardada = "";
String bufferBT = ""; 

void setup() {
  Serial.begin(9600);   
  Serial1.begin(9600);  // ESP8266
  Serial2.begin(9600);  // Bluetooth
  
  lcd.init(); lcd.backlight();
  pinMode(PIN_SUELO, INPUT); dht.begin();               

  Serial.println("--- SISTEMA V24: MOSTRAR IP ---");
  lcd.setCursor(0, 0); lcd.print("Esperando Web...");
}

void loop() {
  // A. LEER BLUETOOTH (Byte a Byte)
  while (Serial2.available() > 0) {
    char c = (char)Serial2.read();
    if (c == '\n') {
      bufferBT.trim();
      if (bufferBT.length() > 0) procesarComando(bufferBT);
      bufferBT = "";
    } else {
      bufferBT += c;
    }
  }

  // B. LEER ESP8266 (AQUI OCURRE LA MAGIA DE LA IP)
  if (Serial1.available()) {
    String resp = Serial1.readStringUntil('\n');
    resp.trim();
    
    if(resp.length() > 0) {
       // Debug en terminal
       // Serial.print("[ESP RAW]: "); Serial.println(resp);

       // Detectamos si el mensaje contiene la dirección web
       if (resp.indexOf("http") >= 0) {
          sistemaOnline = true;
          
          // PARSEO DE LA IP:
          // El ESP manda: "[INFO] http://192.168.1.65:3000"
          int inicioIP = resp.indexOf("http://") + 7; // Saltamos el http://
          int finIP = resp.indexOf(":3000");          // Buscamos donde empieza el puerto
          
          // Extraemos solo los números de la IP (ej: 192.168.1.65)
          String soloIP = resp.substring(inicioIP, finIP);
          ipServidor = soloIP;

          // 1. MOSTRAR EN LCD (Claramente)
          lcd.clear(); 
          lcd.setCursor(0,0); 
          lcd.print("IP:" + soloIP);      // Línea 1: IP:192.168.1.65
          lcd.setCursor(0,1); 
          lcd.print("Port:3000");         // Línea 2: Port:3000
          
          // 2. MOSTRAR EN TERMINAL ARDUINO
          Serial.println("\n==================================");
          Serial.println("✅ ¡SERVIDOR WEB ACTIVO!");
          Serial.println("----------------------------------");
          Serial.print("🌐 IP:     "); Serial.println(soloIP);
          Serial.println("🔌 PUERTO: 3000");
          Serial.println("🔗 URL:    http://" + soloIP + ":3000");
          Serial.println("==================================\n");

          // Avisamos al Bluetooth
          Serial2.println("OK_WIFI"); 
       }
       else if (resp.indexOf("ERROR") >= 0 || resp.indexOf("Fallo") >= 0) {
          lcd.clear(); lcd.print("Error Conexion");
          Serial.println("❌ ERROR: Contraseña incorrecta o fallo de red.");
          Serial2.println("ERR_WIFI");
       }
    }
  }

  // C. SENSORES (Solo enviamos si ya hay conexión)
  if (millis() - tiempoAnterior > INTERVALO_ENVIO) {
    tiempoAnterior = millis();
    if (sistemaOnline) {
       int s = map(analogRead(PIN_SUELO), 0, 1023, 100, 0);
       // Enviar JSON al ESP
       Serial1.println("{\"suelo\":" + String(s) + "}");
    }
  }
}

void procesarComando(String cmd) {
  Serial.println("[BT Recibido]: " + cmd);

  if (cmd.startsWith("S:")) {
    redGuardada = cmd.substring(2);
    Serial.println(" -> Red recibida: " + redGuardada);
    
    lcd.clear(); lcd.print("Red OK!");
    lcd.setCursor(0,1); lcd.print("Ponga Clave...");
    
    delay(100); 
    Serial2.println("OK_RED"); // Confirmar a la Web
  }
  else if (cmd.startsWith("P:")) {
    String clave = cmd.substring(2);
    Serial.println(" -> Clave recibida. Conectando...");

    if (redGuardada.length() > 0) {
      Serial2.println("OK_PASS");
      lcd.clear(); lcd.print("Conectando...");
      lcd.setCursor(0,1); lcd.print("Espere IP...");
      
      // Enviamos al ESP
      Serial1.println("CONECTAR:" + redGuardada + "," + clave);
    } else {
      Serial2.println("ERR_NO_RED");
    }
  }
}