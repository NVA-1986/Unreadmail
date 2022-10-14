#include <Adafruit_NeoPixel.h>
#include <ArduinoMqttClient.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

char input;
int input2;

#define PIN            3
#define PIXELS_PER_SEGMENT  3
#define DIGITS 1

    //**************** -- MQTT -- ********************//
        //MQTT
        #define MQTT_BROKER       "IP/FQDN"
        #define MQTT_BROKER_PORT  1883
        #define MQTT_TOPIC        "Topic"
        #define MQTT_USERNAME     ""
        #define MQTT_KEY          "" 

        WiFiClient wifiClient;
        MqttClient mqttClient(wifiClient);
    //**************** -- FIN MQTT -- ****************//

int i = 6;

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

void setup() {
  Serial.begin(9600);
  strip.begin();
  writestart(0, 0);
  strip.show();
  delay(1000);

      //**************** -- WIFI et MQTT -- ********************//
//WIFI
      // Détecte s'il connait un wifi ou crée un AP pour se connecter à un WiFi
      WiFiManager wm;
      bool res;
      res = wm.autoConnect("AutoConnectAP","password"); // password protected ap
      if(!res) {
          Serial.println("Failed to connect");
        } 
        else {  
            Serial.println("connected...yeey :)");
        }

//MQTT
      
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
        Serial.println();
          strip.begin();
          writeconected(0, 0);
          strip.show();
          delay(1000);
        
        // set the message receive callback
        Serial.print("Subscribing to topic: ");
        Serial.println(MQTT_TOPIC);
        Serial.println();
      
        // subscribe to a topic
        mqttClient.subscribe(MQTT_TOPIC);
      
        // topics can be unsubscribed using:
        // mqttClient.unsubscribe(topic);
      
        Serial.print("Topic: ");
        Serial.println(MQTT_TOPIC);
        Serial.println();
        
    //**************** -- FIN WIFI et MQTT -- ****************//

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
