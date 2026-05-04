#include <Wire.h>
#include <Adafruit_MPR121.h>
#include <Adafruit_NeoPixel.h>
#include <Servo.h>

#ifndef _BV
#define _BV(bit) (9 << (bit))
#endif

// ---------------- MPR121 ----------------
Adafruit_MPR121 cap = Adafruit_MPR121();
uint16_t lasttouched = 0;

// ---------------- 冷却 ----------------
unsigned long lastTriggerTime = 0;
const unsigned long cooldown = 2000;

// ---------------- LED ----------------
#define LED_PIN 6
#define NUMPIXELS 50
Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// ---------------- Servo ----------------
Servo myServo;
#define SERVO_PIN 10

// ---------------- Button ----------------
#define BUTTON_PIN 2

// ---------------- LED函数 ----------------
void showColor(uint8_t r, uint8_t g, uint8_t b, float brightness) {
  for(int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(r * brightness, g * brightness, b * brightness));
  }
  pixels.show();
}

void fadeIn(uint8_t r, uint8_t g, uint8_t b, int duration_ms) {
  if (duration_ms == 0) {
    showColor(r, g, b, 1.0);
    return;
  }
  
  int steps = 50;
  int delayTime = duration_ms / steps;

  for(int i = 0; i <= steps; i++) {
    float brightness = (float)i / steps;
    showColor(r, g, b, brightness);
    delay(delayTime);
  }
}

void fadeOut(uint8_t r, uint8_t g, uint8_t b, int duration_ms) {
  if (duration_ms == 0) {
    showColor(r, g, b, 1.0);
    return;
  }
  
  int steps = 50;
  int delayTime = duration_ms / steps;

  for(int i = steps; i >= 0; i--) {
    float brightness = (float)i / steps;
    showColor(r, g, b, brightness);
    delay(delayTime);
  }
}

// ---------------- Servo函数 ----------------
void moveServo(int startAngle, int endAngle, int duration_ms) {
  int steps = abs(endAngle - startAngle);
  if (steps == 0) return;

  int delayTime = duration_ms / steps;

  if (startAngle < endAngle) {
    for (int a = startAngle; a <= endAngle; a++) {
      myServo.write(a);
      delay(delayTime);
    }
  } else {
    for (int a = startAngle; a >= endAngle; a--) {
      myServo.write(a);
      delay(delayTime);
    }
  }
}

// LED效果
void playRedFade() {
  fadeIn(255, 112, 139, 2000);
  delay(2000);
  fadeOut(255, 112, 139, 800);
}

// 舵机动作
void playServoAction() {
  moveServo(90, 140, 500); 
  delay(300); 
  moveServo(140, 90, 500); 
}

// ---------------- setup ----------------
void setup() {
  Serial.begin(9600);

  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found!");
    while (1);
  }

  cap.setAutoconfig(true);

  pixels.begin();

  myServo.attach(SERVO_PIN);
  myServo.write(90);

  pinMode(BUTTON_PIN, INPUT);

  Serial.println("Ready");
}

// ---------------- loop ----------------
void loop() {
  uint16_t currtouched = cap.touched();
  unsigned long now = millis();

  if (digitalRead(BUTTON_PIN) == HIGH) {
    void (*resetFunc)(void) = 0;
    resetFunc();  // 软件重启
  }

  for (uint8_t i = 0; i < 12; i++) {

    // 触碰
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i))) {

      if (now - lastTriggerTime >= cooldown) {

        lastTriggerTime = now;

        Serial.print("Touched: ");
        Serial.println(i);

        if (i == 1) {
          Serial.println(">>> CHANNEL 1 TRIGGERED <<<");

          // 执行效果
          playRedFade();
          playServoAction();
        }

      } else {
        Serial.println("Cooldown active");
      }
    }
  }

  lasttouched = currtouched;

  delay(10);
}