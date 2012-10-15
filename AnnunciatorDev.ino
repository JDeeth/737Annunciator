#include <Bounce.h>

enum {
  PIN_SW1 = 0,
  PIN_SW2 = 4,
  PIN_SW3 = 8,
  PIN_SW4 = 12,
  PIN_SW5 = 16,
  PIN_COUNT = 5};

const int InPin[] = {
  PIN_SW1, PIN_SW2, PIN_SW3, PIN_SW4, PIN_SW5};

Bounce * sw[5];

void setup() {
  for (int i = 0; i < PIN_COUNT; ++i) {
    sw[i] = new Bounce (InPin[i], 5);
    pinMode(InPin[i], INPUT_PULLUP );
  }

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  for (int i = 0; i < PIN_COUNT; ++i) {
    sw[i]->update();
  }

  if(sw[1]->read() == LOW
    || sw[2]->read() == LOW
    || sw[3]->read() == LOW
    || sw[4]->read() == LOW
    || sw[5]->read() == LOW) {
    digitalWrite(LED_BUILTIN, HIGH);
  } 
  else {
    digitalWrite(LED_BUILTIN, LOW);
  }
}

