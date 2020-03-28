#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_SCL 0
#define OLED_SDA 2

#define OLED_ADDR     0x3C // OLED address
#define SCREEN_WIDTH  128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64   // OLED display height, in pixels
#define OLED_RESET    -1   // Reset pin

const char* ssid  = "";
const char* pass  = "";
const char* id    = "";

char* tTemp = "livingroom/temp";
char* tHum  = "livingroom/hum";

IPAddress broker(192,168,1,1);
WiFiClient wifiClient;
PubSubClient pubSubClient(wifiClient);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

String temperature = "";
String humidity = "";
  
void setup() {
  Serial.begin(9600);

  setup_wifi();
  pubSubClient.setServer(broker, 1883);
  pubSubClient.setCallback(callback);

  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  clearDisplay();
  display.print("Waiting for data");
  display.display();
}

void setup_wifi() {
  WiFi.begin(ssid, pass);
  Serial.print("Connection to WiFi network");
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connection established. IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String response;

  for (int i = 0; i < length; i++) {
    response += (char)payload[i];
  }

  if(strcmp(topic, tTemp)) {
    temperature = response;
  }
  
  if(strcmp(topic, tHum)) {
    humidity = response;
  }
}

void loop() {  
  if(!pubSubClient.connected()) {
    reconnect();
  }
  pubSubClient.loop();

  clearDisplay();
  printTemp(temperature);
  printHum(humidity);
  display.display();

  delay(100);
}

void reconnect() {
  Serial.print("Attempting to connect MQTT broker");
  while(!pubSubClient.connected()) {
    Serial.print(".");

    if(pubSubClient.connect(id)) {
      pubSubClient.subscribe(tTemp);
      pubSubClient.subscribe(tHum);
      Serial.println("\nConnected to MQTT broker");
    } else {
      delay(1000);
    }
  }
}

void clearDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
}

void printTemp(String payload) {
  display.setCursor(10, 10);
  display.print("Temperature: ");
  display.print(payload);
  display.print((char)247);
  display.print("C");
}

void printHum(String payload) {
  display.setCursor(10, 20);
  display.print("Humidity: ");
  display.print(payload);
  display.print("%");
}
