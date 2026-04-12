#include "AttackAnimation.h"

// ==========================================
// BASE CLASS LOGIC
// ==========================================

AttackAnimation::AttackAnimation(Adafruit_SSD1306& display, String name, int speed)
  : _display(display), _name(name), _speed(speed), _state(ATTACK_READY) {}

void AttackAnimation::start(int startY) {
    _xPos = 0; 
    _yPos = startY;
    _impactTimer = 15; // Impact lasts 15 ticks
    _state = ATTACK_MOVING;
}

AttackState AttackAnimation::update() {
    if (_state == ATTACK_MOVING) {
        _xPos += _speed;
        // Trigger impact when we reach the center
        if (_xPos >= _display.width() ) { 
            _state = ATTACK_IMPACTING;
        }
    } else if (_state == ATTACK_IMPACTING) {
        if (_impactTimer > 0) {
            _impactTimer--;
        } else {
            _state = ATTACK_DONE;
        }
    }
    return _state;
}

// Helper method to draw the text box so we don't repeat this code
void AttackAnimation::drawTextBox() {
    int boxWidth = (_name.length() * 6) + 4; 
    int boxHeight = 14;
    int boxX = (_display.width() - boxWidth) / 2;
    int boxY = (_display.height() - boxHeight) / 2;
    
    _display.fillRect(boxX, boxY, boxWidth, boxHeight,BLACK);
    _display.drawRect(boxX, boxY, boxWidth, boxHeight, WHITE);
    
    _display.setTextSize(1);
    _display.setTextColor(WHITE);
    _display.setCursor(boxX + 3, boxY + 3);
    _display.print(_name);
}

// ==========================================
// SUBCLASS 1: FREEZE ATTACK
// ==========================================

FreezeAttack::FreezeAttack(Adafruit_SSD1306& display, int speed) 
  : AttackAnimation(display, "FREEZE", speed) {}

void FreezeAttack::drawProjectile() {
    _display.drawCircle(_xPos, _yPos, 4, WHITE); // Hollow circle
}

void FreezeAttack::drawImpact() {
    // Glacy random dots
    for (int i = 0; i < 60; i++) {
        _display.drawPixel(random(0, _display.width()), random(0, _display.height()), WHITE);
    }
    drawTextBox();
}


// ==========================================
// SUBCLASS 2: LIGHTNING ATTACK
// ==========================================

LightAttack::LightAttack(Adafruit_SSD1306& display, int speed) 
  : AttackAnimation(display, "ZAP", speed) {}

void LightAttack::drawProjectile() {
    // Fire projectile is a triangle
    _display.fillTriangle(_xPos, _yPos, _xPos - 6, _yPos - 4, _xPos - 6, _yPos + 4, WHITE);
}

void LightAttack::drawImpact() {
// 1. DYNAMIC FIRE PARTICLES (Moving upwards)
    // We draw random vertical "heat" lines that change every frame
    for (int i = 0; i < 40; i++) {
        int x = random(0, _display.width());
        int yStart = random(0, _display.height());
        int length = random(5, 15);
        
        // Draw vertical sparks
        _display.drawFastVLine(x, yStart, length, WHITE);
    }

    // 2. FLASH EFFECT
    // Occasionally fill the screen for 1 frame to simulate a "burst"
    if (random(0, 5) == 0) {
        for(int i=0; i<_display.width(); i+=2) {
             _display.drawFastVLine(i, 0, _display.height(),WHITE);
        }
    }
    
    drawTextBox();
}