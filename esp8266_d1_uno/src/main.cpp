#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>

// Ustawienia Access Point
const char *ssid = "Czterej_Pancerni_i_Piec";
const char *password = "wielki_piec";

// WebServer i WebSocket
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Definicje pinów
int dir1PinA = D5;  // kierunek A1
int dir2PinA = D6;  // kierunek A2
int speedPinA = D7; // PWM A

int dir1PinB = D8;   // kierunek B1
int dir2PinB = D9;   // kierunek B2
int speedPinB = D10; // PWM B

// Funkcja pomocnicza: dzieli string po przecinku
std::vector<int> parseMessage(String message)
{
  std::vector<int> values;
  int lastIndex = 0;
  int index = message.indexOf(',');

  while (index >= 0)
  {
    values.push_back(message.substring(lastIndex, index).toInt());
    lastIndex = index + 1;
    index = message.indexOf(',', lastIndex);
  }
  values.push_back(message.substring(lastIndex).toInt()); // ostatnia wartość
  return values;
}

void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  if (type == WS_EVT_DATA)
  {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len)
    {
      String message = "";
      for (size_t i = 0; i < len; i++)
      {
        message += (char)data[i];
      }
      Serial.println("Received message: " + message);

      auto values = parseMessage(message);

      if (values.size() == 6)
      {
        int l_pwm = values[0];
        int in1 = values[1];
        int in2 = values[2];
        int r_pwm = values[3];
        int in3 = values[4];
        int in4 = values[5];

        analogWrite(speedPinA, l_pwm);
        digitalWrite(dir1PinA, in1);
        digitalWrite(dir2PinA, in2);

        analogWrite(speedPinB, r_pwm);
        digitalWrite(dir1PinB, in3);
        digitalWrite(dir2PinB, in4);

        Serial.println("Motors updated");
      }
      else
      {
        Serial.println("Invalid message format!");
      }
    }
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(dir1PinA, OUTPUT);
  pinMode(dir2PinA, OUTPUT);
  pinMode(speedPinA, OUTPUT);

  pinMode(dir1PinB, OUTPUT);
  pinMode(dir2PinB, OUTPUT);
  pinMode(speedPinB, OUTPUT);

  analogWriteFreq(500); // ustawienie częstotliwości PWM

  // Uruchomienie Access Point
  WiFi.softAP(ssid, password);
  Serial.println("Access Point uruchomiony!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Konfiguracja WebSocket
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);

  // Start serwera
  server.begin();
}

void loop()
{
  // WebSocket działa asynchronicznie – pusta pętla
}
