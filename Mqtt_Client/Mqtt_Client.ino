#include <WiFi.h>
#include <PubSubClient.h>
#define buzzer 17
// Update these with your settings
const char* ssid = "Zahir Sami's S23 Ultra";
const char* password = "Zahir2003";
const char* mqtt_server = "broker.hivemq.com"; // A free public broker

WiFiClient espClient;
PubSubClient client(espClient);

volatile byte active;
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
// This function connects the ESP32 to your WiFi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
}

// This function handles reconnecting to the MQTT broker if you lose connection
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); // This processes incoming messages and maintains the connection
}
