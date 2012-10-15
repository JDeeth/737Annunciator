#include <Bounce.h>

enum inPins {
  PIN_SW1 = 0,
  PIN_SW2 = 4,
  PIN_SW3 = 8,
  PIN_SW4 = 12,
  PIN_SW5 = 16 };
  
Bounce sw1 = Bounce (PIN_SW1, 5);
Bounce sw2 = Bounce (PIN_SW2, 5);
Bounce sw3 = Bounce (PIN_SW3, 5);
Bounce sw4 = Bounce (PIN_SW4, 5);
Bounce sw5 = Bounce (PIN_SW5, 5);

void setup() {
  pinMode(PIN_SW1, INPUT_PULLUP);
  pinMode(PIN_SW2, INPUT_PULLUP);
  pinMode(PIN_SW3, INPUT_PULLUP);
  pinMode(PIN_SW4, INPUT_PULLUP);
  pinMode(PIN_SW5, INPUT_PULLUP);
  
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  sw1.update();
  sw2.update();
  sw3.update();
  sw4.update();
  sw5.update();
  
  if(sw1.read() == LOW
  || sw2.read() == LOW
  || sw3.read() == LOW
  || sw4.read() == LOW
  || sw5.read() == LOW) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
}
