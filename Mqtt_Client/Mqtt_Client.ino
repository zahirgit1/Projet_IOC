#include <WiFi.h>
#include <PubSubClient.h>
#define buzzer 17
// Update these with your settings
const char* ssid = "Zahir Sami's S23 Ultra";
const char* password = "Zahir2003";
const char* mqtt_server = "10.92.174.67"; // A free public broker
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "AttackAnimation.h"


AttackAnimation* activeAttack;
AttackAnimation* Attacked;
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET 16
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
volatile byte active, button_pressed;

// Create instances of your subclasses!
FreezeAttack freezeMagic(display, 8);
LightAttack lightMagic(display, 12);
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int current =0 ;
int value = 0;

struct ctx_led_t {
  int timer;                                              // n° du timer pour cette tâche utilisé par WaitFor
  unsigned long period;                                   // periode de clignotement
  int pin;                                                // numéro de la broche sur laquelle est la LED
  int etat;  // etat interne de la led
}  ;

#define MAX_WAIT_FOR_TIMER 2
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
void  init_button(struct ctx_led_t * button, int timer, unsigned long period, byte pin)
{
  button_pressed = 0;//variable pour traiter un appui une seule fois
  pinMode(pin, INPUT_PULLUP);//init de la resistance pull up
  button->timer = timer;
  button->period = period ;
  button->pin = pin;
  button->etat = digitalRead(button->pin);
}
void step_button(struct ctx_led_t * button) {
  if (!waitFor(button->timer, button->period)  ) return;// sort s'il y a moins d'une période écoulée
  button->etat = digitalRead(button->pin);
  if ((button->etat == HIGH) ) {button_pressed = 0;return;  }  // flag pour verifier qu'on traite l'appui qu'une seule fois   
  if(!(button->etat == HIGH) and !button_pressed) 
    {   
       
        if (current ==0)
        {
          activeAttack = &freezeMagic;
        }
        else { 
          activeAttack = &lightMagic;}

        activeAttack->start(SCREEN_HEIGHT / 2);
            // 1. Calculate the math
           display.clearDisplay();
        AttackState currentState = activeAttack->update();

        // 2. You control the drawing based on the state!
        while(currentState != ATTACK_DONE){
          
            if (currentState == ATTACK_MOVING) {
                activeAttack->drawProjectile();
                currentState = activeAttack->update();
            } 
            else if (currentState == ATTACK_IMPACTING) {
                
                currentState = activeAttack->update();
          // You can add screen shakes or delay tricks here since YOU control it!
            } 
            
            display.display();
            //delay(15);
            display.clearDisplay();
            

        }
            if (currentState == ATTACK_DONE ) {

              if (activeAttack == &lightMagic) {
                  client.publish("Attack_zap_1","1");
                  Serial.println("zap !!!");
              } else {
                 client.publish("Attack_freeze_1","1");
                  Serial.println("freeze !!!");
              }
            }
    button_pressed = 1;                    
    Serial.println("+1");// validation de l'appui sur le bouton
    }                             
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
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
    if (String(topic) == "ESP32/freeze"){

      if ((char)payload[0] == '1') {
        current = 1; // Set the flag to indicate the freeze attack is active
      } else {
        current = 0;  // Reset the flag to indicate the freeze attack is not active
      }
  }
  if (String(topic) == "Attack_freeze_2"){
    Serial.print("froze !!!!");
    Attacked = &freezeMagic;
      if ((char)payload[0] == '1') {
          for(int i=0;i<15;i++){   
          Attacked->drawImpact();
          display.display();
          display.clearDisplay();}
      }

  }
  if (String(topic) == "Attack_zap_2"){
    Serial.print("zapped !!");
    Attacked = &lightMagic;
      if ((char)payload[0] == '1') {
          for(int i=0;i<15;i++){   
          Attacked->drawImpact();
          display.display();
          display.clearDisplay();}

      }
  }


  
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

      client.subscribe("inTopic");
      client.subscribe("Attack_log");
      client.subscribe("Attack_zap_1");
      client.subscribe("Attack_freeze_1");
      client.subscribe("Attack_zap_2");
      client.subscribe("Attack_freeze_2");
      client.subscribe("ESP32/freeze");
      client.subscribe("ESP32/zap");
      client.subscribe("ESP32/5");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
ctx_led_t button;
void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
    init_button(&button,3,500,23);
  Wire.begin(4, 15);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    for(;;);
  }

  

}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); // This processes incoming messages and maintains the connection
  display.clearDisplay();
  step_button(&button);

}
