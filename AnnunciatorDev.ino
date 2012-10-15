#include <Bounce.h>

///////////////// Datarefs (faked with switches)
//
// Datarefs are grouped into 12 systems
//
// FC: Flight Controls
// IRS
// FUEL
// ELEC
// APU
// OD: OVHT/DET
// AI: Anti-Ice
// HYD
// DOOR
// ENG
// OH: Overhead
// AC: Air Conditioning

// "Datarefs" for individual overhead-panel annunciators
enum { // all values after an intermediate _COUNT must
  FC1 = 0,  // equal the preceding value
  FC2,
  FC_COUNT,
  IRS1 = FC_COUNT,
  IRS_COUNT,
  DR_COUNT = IRS_COUNT};

const int DRPin[DR_COUNT] = {0, 4, 8};

Bounce * dr[DR_COUNT];

////////////////// Switch inputs
enum {
  SW_MASTER = 0,
  SW_SIXPACK,
  SW_COUNT};

const int SwPin[SW_COUNT] = {12, 16};

Bounce * sw[SW_COUNT];

///////////////////////// LED outputs
enum {
  LED_FC,
  LED_IRS,
  LED_MC,
  LED_COUNT};

const int LedPin[LED_COUNT] = {
  44, 45, LED_BUILTIN };


void setup() {
  // "datarefs"
  for (int i = 0; i < DR_COUNT; ++i) {
    dr[i] = new Bounce (DRPin[i], 5);
    pinMode(DRPin[i], INPUT_PULLUP );
  }

  // switch input
  for (int i = 0; i < SW_COUNT; ++i) {
    sw[i] = new Bounce (SwPin[i], 5);
    pinMode(SwPin[i], INPUT_PULLUP );
  }
  // LED output
  for (int i = 0; i < LED_COUNT; ++i) {
    pinMode(LedPin[i], OUTPUT);
  }
}

void loop() {
  // "Datarefs"
  for (int i = 0; i < DR_COUNT; ++i) {
    dr[i]->update();
  }
  // Switches
  for (int i = 0; i < SW_COUNT; ++i) {
    sw[i]->update();
  }

  ////////////////////////////////////
  // System Annunciators
  //
  // Flight Control datarefs
  bool saOn[12] = {false};

  for (int i = FC1; i < FC_COUNT; ++i) {
    if(dr[i]->read() == LOW)
      saOn[0] = true;
  }
  digitalWrite(LedPin[LED_FC], saOn[0]);

  // IRS datarefs
  for (int i = IRS1; i < IRS_COUNT; ++i) {
    if(dr[i]->read() == LOW)
      saOn[1] = true;
  }
  digitalWrite(LedPin[LED_IRS], saOn[1]);

  //etc

  bool tmp = false;
  for (int i = 0; i < 12; ++i) {
    if(saOn[i])
      tmp = true;
  }
  digitalWrite(LedPin[LED_MC], tmp);
}

