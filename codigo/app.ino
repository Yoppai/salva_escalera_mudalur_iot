#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include "fauxmoESP.h"

#define SERIAL_BAUDRATE 115200

/* Network credenciales */
#define WIFI_SSID "RUT950_476A"
#define WIFI_PASS "Nq5c6Y3F"

/* IDs */
#define ID_SUBIR "arriba"
#define ID_BAJAR "abajo"
#define ID_PARAR "alto"


/* Paso Completo 2 Bobinas*/
int paso[4][4] = {
  { 1, 1, 0, 0 },
  { 0, 1, 1, 0 },
  { 0, 0, 1, 1 },
  { 1, 0, 0, 1 }
};
// demora en ms
int demora = 10;
// pines
int IN1 = 27;
int IN2 = 14;
int IN3 = 12;
int IN4 = 13;
int PULSADOR = 33;

bool girarDelante = false;
bool girarAtras = false;

fauxmoESP fauxmo;


void wifiSetup() {

  // Set WIFI module to STA mode
  WiFi.mode(WIFI_STA);

  // Connect
  Serial.printf("[WIFI] Connecting to %s ", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // Wait
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  // Connected!
  Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
}

void isr(){
  girarDelante = false;
  girarAtras = false;
  digitalWrite(IN1, LOW);  // detiene.
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void setup() {

  // Init serial port and clean garbage
  Serial.begin(SERIAL_BAUDRATE);

  // Inicialisar los pines
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(PULSADOR, INPUT_PULLDOWN);
  attachInterrupt(PULSADOR, isr, RISING);

  // Wifi
  wifiSetup();

  fauxmo.createServer(true);  // not needed, this is the default value
  fauxmo.setPort(80);         // This is required for gen3 devices
  fauxmo.enable(true);
  // Add virtual devices
  fauxmo.addDevice(ID_SUBIR);
  fauxmo.addDevice(ID_BAJAR);
  fauxmo.addDevice(ID_PARAR);
  fauxmo.onSetState([](unsigned char device_id, const char* device_name, bool state, unsigned char value) {
    Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);

    if (strcmp(device_name, ID_SUBIR) == 0) {
      girarDelante = true;
      girarAtras = false;
    } else if (strcmp(device_name, ID_BAJAR) == 0) {
      girarAtras = true;
      girarDelante = false;
    } else if (strcmp(device_name, ID_PARAR) == 0) {
      girarDelante = false;
      girarAtras = false;
      digitalWrite(IN1, LOW);  // detiene.
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);;
    }
  });
}

void loop() {

  fauxmo.handle();

  while (girarDelante) {
      for (int i = 0; i < 4; i++)       // bucle recorre la matriz de a una fila por vez
      {                                 // para obtener los valores logicos a aplicar
        digitalWrite(IN1, paso[i][0]);  // a IN1, IN2, IN3 e IN4
        digitalWrite(IN2, paso[i][1]);
        digitalWrite(IN3, paso[i][2]);
        digitalWrite(IN4, paso[i][3]);
        delay(demora);
      };
    };

    while (girarAtras) {
      for (int i = 3; i >= 0; i--)      // bucle recorre la matriz de a una fila por vez
      {                                 // para obtener los valores logicos a aplicar
        digitalWrite(IN4, paso[i][3]);  // a IN1, IN2, IN3 e IN4
        digitalWrite(IN3, paso[i][2]);
        digitalWrite(IN2, paso[i][1]);
        digitalWrite(IN1, paso[i][0]);
        delay(demora);
      };
    };
 
}