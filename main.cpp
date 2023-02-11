#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoMqttClient.h>
// LED
#define PIN            3
#define PIXELS_PER_SEGMENT  3
#define DIGITS 1
//MQTT
#define MQTT_BROKER       "IP"
#define MQTT_BROKER_PORT  1883
#define MQTT_TOPIC        "Topic"
#define MQTT_USERNAME     ""
#define MQTT_KEY          "" 
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
// VAR
char input;
int input2;
int i = 6;
const char* ssid = "SSID";
const char* password = "PASS";
String hostname = "Afficheur";
AsyncWebServer server(80);
// ******************************************************************************************************************** // STRIP LED
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXELS_PER_SEGMENT * 7 * DIGITS, PIN, NEO_GRB + NEO_KHZ800);
byte segments[11] = {
  0b1111110, //0
  0b0011000, //1
  0b0110111, //2
  0b0111101, //3
  0b1011001, //4
  0b1101101, //5
  0b1101111, //6
  0b0111000, //7
  0b1111111, //8
  0b1111101, //9
  0b1100111 //E (10)
};
void writestart(int index, int value) {
  byte seg = segments[value];
  for (int i = 6; i >= 0; i--) {
    int offset = index * (PIXELS_PER_SEGMENT * 7) + i * PIXELS_PER_SEGMENT;
    uint32_t color = seg & 0x01 != 0 ? strip.Color(100, 0, 100) : strip.Color(0, 0, 0);
    for (int x = offset; x < offset + PIXELS_PER_SEGMENT; x++) {
      strip.setPixelColor(x, color);
    }
    seg = seg >> 1;
  }
}
void writeconected(int index, int value) {
  byte seg = segments[value];
  for (int i = 6; i >= 0; i--) {
    int offset = index * (PIXELS_PER_SEGMENT * 7) + i * PIXELS_PER_SEGMENT;
    uint32_t color = seg & 0x01 != 0 ? strip.Color(0, 0, 255) : strip.Color(0, 0, 0);
    for (int x = offset; x < offset + PIXELS_PER_SEGMENT; x++) {
      strip.setPixelColor(x, color);
    }
    seg = seg >> 1;
  }
}
void writeerror(int index, int value) {
  byte seg = segments[value];
  for (int i = 6; i >= 0; i--) {
    int offset = index * (PIXELS_PER_SEGMENT * 7) + i * PIXELS_PER_SEGMENT;
    uint32_t color = seg & 0x01 != 0 ? strip.Color(255, 0, 0) : strip.Color(0, 0, 0);
    for (int x = offset; x < offset + PIXELS_PER_SEGMENT; x++) {
      strip.setPixelColor(x, color);
    }
    seg = seg >> 1;
  }
}
void writeDigit(int index, int value) {
  byte seg = segments[value];
  for (int i = 6; i >= 0; i--) {
    int offset = index * (PIXELS_PER_SEGMENT * 7) + i * PIXELS_PER_SEGMENT;
    uint32_t color = seg & 0x01 != 0 ? strip.Color(0, 255, 0) : strip.Color(0, 0, 0);
    for (int x = offset; x < offset + PIXELS_PER_SEGMENT; x++) {
      strip.setPixelColor(x, color);
    }
    seg = seg >> 1;
  }
}
// ******************************************************************************************************************** // FIN STRIP LED

void setup() {
// ********************************************************** // INIT
  Serial.begin(9600);
	delay(1000);
	Serial.println("\n");
  Serial.println("*****************************************************************");
  Serial.println("Bienvenue sur cet ESP destiné à Plott Informatique");
  Serial.println("Le code se trouve sur: https://github.com/NVA-1986/Unreadmail/");
  Serial.println("*****************************************************************");
  strip.begin();
  writestart(0, 0);
  strip.show();
  delay(1000);
// ********************************************************** // WIFI
	WiFi.begin(ssid, password);
	Serial.print("Tentative de connexion...");
	while(WiFi.status() != WL_CONNECTED)
	{
		Serial.print(".");
		delay(500);
	}
	// Serial.println("\n");
	Serial.println("Connexion etablie!");
	Serial.print("Adresse IP: ");
	Serial.println(WiFi.localIP());
  Serial.println("*****************************************************************");
// ********************************************************** // LittleFS
  if (!LittleFS.begin()) {
    Serial.println("Error mounting LittleFS");
    return;
  }
  //Lister les fichiers trouvé
  Serial.println("LittleFS mounted");
  // Affichez la liste des fichiers
  Dir dir = LittleFS.openDir("/");
  while (dir.next()) {
      Serial.println(dir.fileName());
  }
  // Ouvrez le fichier HTML
  File file = LittleFS.open("/index.html", "r");
  if (!file) {
    Serial.println("Error opening HTML file");
    return;
  }
  // Lisez le contenu du fichier
  String html = file.readString();
  file.close();
  Serial.println("*****************************************************************");
// ********************************************************** // MQTT
  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(MQTT_BROKER);

  if (!mqttClient.connect(MQTT_BROKER, MQTT_BROKER_PORT)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
      strip.begin();
      writeerror(0, 10);
      strip.show();
          
    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");
  // Serial.println();
    strip.begin();
    writeconected(0, 0);
    strip.show();
    delay(1000);
  
  // set the message receive callback
  Serial.print("Subscribing to topic: ");
  Serial.println(MQTT_TOPIC);
  // Serial.println();

  // subscribe to a topic
  mqttClient.subscribe(MQTT_TOPIC);

  // topics can be unsubscribed using:
  // mqttClient.unsubscribe(topic);

  Serial.print("Topic: ");
  Serial.println(MQTT_TOPIC);
  // Serial.println();
  Serial.println("*****************************************************************");
// ********************************************************** // WEB
  server.begin();
  // Servez la page HTML
  server.on("/", HTTP_GET, [html](AsyncWebServerRequest *request){
    request->send(200, "text/html", html);
  });
  server.on("/w3.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/w3.css", "text/css");
  });
  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/favicon.ico", "image/ico");
  });
  server.on("/logo.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/logo.png", "image/png");
  });
  // Récupérer la valeur de input2
  server.on("/input2-value", HTTP_GET, [](AsyncWebServerRequest *request){
    int input2 = input-'0';
    String input2b = String(input2);
    // Serial.print("Valeur I2 :");
    // Serial.println(input2b);
    request->send(200, "text/plain", String(input2b));
  });
  // Mise à jour la valeur de input2
  
  // Ajoutez ici le code pour MqttClient, en vous assurant de bien inclure les bibliothèques nécessaires et de configurer correctement la variable mqttClient
}

int c = 0;
int b = 0;

void clearDisplay() {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
}

void loop() {
  clearDisplay();
  
  int messageSize = mqttClient.parseMessage();
  if (messageSize) {
    // we received a message, print out the topic and contents
    Serial.print("Received a message with topic '");
    Serial.print(mqttClient.messageTopic());
    Serial.print("', length ");
    Serial.print(messageSize);
    Serial.println(" bytes:");
//    pinMode(LED, OUTPUT);

    // use the Stream interface to print the contents
    while (mqttClient.available()) {
      
 input= (int)mqttClient.read() ;
int input2 = input-'0';
// int input2 = 11;
 
 // Serial.print((int)mqttClient.read());
     Serial.print(input);
     Serial.println();
     Serial.print(input2);
     Serial.println();

    if (input2<=9){
        writeDigit(0, input2);
        strip.show();
        delay(1000);
        }
    else{
        writeDigit(0, 9);
        strip.show();
        delay(1000);
        }

  strip.show();
  delay(1000);
}
  }
}