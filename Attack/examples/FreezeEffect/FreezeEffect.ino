#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "AttackAnimation.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET 16
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Create instances of your subclasses!
FreezeAttack freezeMagic(display, 8);
LightAttack lightMagic(display, 12);

// We use a pointer to point to whichever attack is currently active
AttackAnimation* activeAttack;

void setup() {
Wire.begin(4, 15);
  Serial.begin(9600);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    for(;;);
  }
  
  // Set the active attack to Freeze, and start it
  activeAttack = &lightMagic;
  activeAttack->start(SCREEN_HEIGHT / 2); 
}

void loop() {
  display.clearDisplay();

  // 1. Calculate the math
  AttackState currentState = activeAttack->update();

  // 2. You control the drawing based on the state!
  if (currentState == ATTACK_MOVING) {
      activeAttack->drawProjectile();
  } 
  else if (currentState == ATTACK_IMPACTING) {
      activeAttack->drawImpact();
      // You can add screen shakes or delay tricks here since YOU control it!
  } 
  else if (currentState == ATTACK_DONE) {
      // Switch attacks just to see both working!
      if (activeAttack == &lightMagic) {
          activeAttack = &freezeMagic;
      } else {
          activeAttack = &lightMagic;
      }
      delay(1500); // Pause before next attack
      activeAttack->start(SCREEN_HEIGHT / 2);
  }

  display.display();
  delay(50); // Frame rate
}
