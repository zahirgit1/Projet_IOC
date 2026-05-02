#include <WiFi.h>
#include <PubSubClient.h>
#define buzzer 17
// Update these with your settings
const char* ssid = "Zahir Sami's S23 Ultra";
const char* password = "Zahir2003";
const char* mqtt_server = "10.192.10.67"; // A free public broker
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "AttackAnimation.h"


AttackAnimation* activeAttack;
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET 16
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Create instances of your subclasses!
FreezeAttack freezeMagic(display, 8);
LightAttack lightMagic(display, 12);
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

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
      client.subscribe("Attack_log");
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
  //client.setCallback(callback);
  Wire.begin(4, 15);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    for(;;);
  }
  activeAttack = &lightMagic;
  activeAttack->start(SCREEN_HEIGHT / 2); 
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); // This processes incoming messages and maintains the connection
    display.clearDisplay();

  // 1. Calculate the math
  AttackState currentState = activeAttack->update();

  // 2. You control the drawing based on the state!
  if (currentState == ATTACK_MOVING) {
      activeAttack->drawProjectile();
  } 
  else if (currentState == ATTACK_IMPACTING) {
      
      
      
      // You can add screen shakes or delay tricks here since YOU control it!
  } 
  else if (currentState == ATTACK_DONE ) {
      // Switch attacks just to see both working!
             client.publish("Attack_log","1");
       Serial.println("attack");
      if (activeAttack == &lightMagic) {

          activeAttack = &freezeMagic;
      } else {
          activeAttack = &lightMagic;
      }
      delay(1500); // Pause before next attack
      activeAttack->start(SCREEN_HEIGHT / 2);
  }

  display.display();

}
