#ifndef AttackAnimation_h
#define AttackAnimation_h

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

// This defines the current state of the attack
enum AttackState {
  ATTACK_READY,
  ATTACK_MOVING,
  ATTACK_IMPACTING,
  ATTACK_DONE
};

// --------------------------------------------------------
// BASE CLASS: Handles all the math, timers, and movement
// --------------------------------------------------------
class AttackAnimation {
  public:
    AttackAnimation(Adafruit_SSD1306& display, String name, int speed);
    
    void start(int startY); 
    AttackState update(); // Calculates math and returns the current state
    
    // Pure virtual functions (the '= 0' means subclasses MUST define these)
    virtual void drawProjectile() = 0; 
    virtual void drawImpact() = 0;     

  protected: // 'protected' means subclasses can use these variables, but the main sketch cannot
    Adafruit_SSD1306& _display;
    String _name;
    int _speed;
    int _xPos;
    int _yPos;
    AttackState _state;
    int _impactTimer;
    
    void drawTextBox(); // A helper function both subclasses can use
};

// --------------------------------------------------------
// SUBCLASS 1: The Freeze Attack
// --------------------------------------------------------
class FreezeAttack : public AttackAnimation {
  public:
    FreezeAttack(Adafruit_SSD1306& display, int speed);
    void drawProjectile() override;
    void drawImpact() override;
};

// --------------------------------------------------------
// SUBCLASS 2: The Lightning Attack
// --------------------------------------------------------
class LightAttack : public AttackAnimation {
  public:
    LightAttack(Adafruit_SSD1306& display, int speed);
    void drawProjectile() override;
    void drawImpact() override;
};

#endif