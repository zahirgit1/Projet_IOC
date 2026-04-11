/*
 Basic ESP8266 MQTT example
 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off
 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/

#include <WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "Zahir Sami's S23 Ultra";
const char* password = "Zahir2003";
const char* mqtt_server = "10.255.70.67";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;


volatile byte active;
#define buzzer 17
struct ctx_led_t {
  int timer;                                              // n° du timer pour cette tâche utilisé par WaitFor
  unsigned long period;                                   // periode de clignotement
  int pin;                                                // numéro de la broche sur laquelle est la LED
  int etat;  // etat interne de la led
}  ;

#define MAX_WAIT_FOR_TIMER 1
unsigned long waitFor(int timer, unsigned long period) {
  static unsigned long last_period[MAX_WAIT_FOR_TIMER];  // il y a autant de timers que de tâches
  unsigned long current = micros() / period;             // numéro de période
  unsigned long delta   = current - last_period[timer];  // gère le wrap-around
  if (delta) last_period[timer] = current;               // mise à jour si déclenchement
  return delta;                                          // nombre de periode depuis le dernier appel
}
void init_buz(struct ctx_led_t * buz, int timer, unsigned long period, byte pin) {// meme principe que la la led vu qu'il faut un signal periodique
  buz->timer = timer;
  buz->period = period ;
  buz->pin = pin;
  buz->etat = 0;
  pinMode(pin, OUTPUT);
  digitalWrite(pin, buz->etat);
}
void step_buz(struct ctx_led_t * buz) {
  if (!waitFor(buz->timer, buz->period) ) return;// sort s'il y a moins d'une période écoulée
  if (!active) buz->etat =0;// mets le buzzer a zero tant que le flag active n'est pas a 1
         
  digitalWrite(buz->pin, buz->etat);                      // ecriture
  buz->etat = 1 - buz->etat;                              // changement d'état
}
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();


}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("outTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
ctx_led_t buz;
void setup() {
  pinMode(BUILTIN_LED, OUTPUT);// Initialize the BUILTIN_LED pin as an output
  init_buz(&buz,0,500,buzzer);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  step_buz(&buz);

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outTopic", msg);
  }
}
