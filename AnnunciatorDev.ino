#include <Bounce.h>

///////////////// Datarefs (faked with switches)
//
// Datarefs are grouped into 12 systems
//
enum {
  FC, //Flight Controls
  IRS,
  //  FUEL,
  //  ELEC,
  //  APU,
  //  OD, // OVHT/DET
  //  AI, // Anti-Ice
  //  HYD,
  //  DOOR,
  //  ENG,
  //  OH, // Overhead
  //  AC, // Air Conditioning
  SA_COUNT
};

enum { // index for dataref array
  FC1 = 0,  // all values after an intermediate _MAX must
  FC2,      // equal the preceding value
  FC_MAX,
  IRS1 = FC_MAX,
  IRS_MAX,
  DR_COUNT = IRS_MAX};

const int DRPin[DR_COUNT] = {0, 4, 8};

Bounce * dr[DR_COUNT];

int dataref[DR_COUNT]; //fake dataref

// Element handling registers
bool drAck[DR_COUNT] = {false}; 
bool mcLit = false;
bool saLit[SA_COUNT] = {false};

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
  LED_SA_MAX,
  LED_MC = LED_SA_MAX,
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
    dataref[i] = !(dr[i]->read());
  }
  // Switches
  for (int i = 0; i < SW_COUNT; ++i) {
    sw[i]->update();
  }
  
  ////////////////////////////////////
  // System Annunciators
  //
  // Flight Control datarefs
  bool saOn[SA_COUNT] = {false}; // for lighting each of the twelve SAs
  bool mcOn = false; //for lighting Master Caution
  
  class SystemAnnc {
  public:
    bool lit();
  private:
    int lowIndex_;
    int highIndex_;
    bool ack_;
  };

  for (int i = FC1; i < FC_MAX; ++i) {
    if(dataref[i]) { //if overhead annunciator is lit
      mcOn = true;
      if(!drAck[i]) {  // if it has not yet caused its SA to light
        saOn[0] = true; // light the SA
        drAck[i] = true; // but only once
      }
    } else { // if overhead annunciator not lit
      drAck[i] = false; // reset acknowledgement
    }
  }
  
  // IRS datarefs
  for (int i = IRS1; i < IRS_MAX; ++i) {
    if(dataref[i]) { //if overhead annunciator is lit
      mcOn = true;
      if(!drAck[i]) {  // if it has not yet caused its SA to light
        saOn[1] = true; // light the SA
        drAck[i] = true; // but only once
      }
    } else { // if overhead annunciator not lit
      drAck[i] = false; // reset acknowledgement
    }
  }
  
  ///////////////////////////////////
  // Input handling
  //
  if(sw[SW_SIXPACK]->read() == LOW) { //if sixpack is pressed
    for (int i = 0; i < LED_SA_MAX; ++i) {
      digitalWrite(LedPin[i], HIGH); //light all the System Annunciators
    }
  }
  if(sw[SW_SIXPACK]->risingEdge()) { //when sixpack released
    for (int i = 0; i < DR_COUNT; ++i) {
      drAck[i] = false; // reset all acknowledgements
    }
    for (int i = 0; i < LED_SA_MAX; ++i) {
      digitalWrite(LedPin[i], LOW);
    }
  }
  
  
  if(sw[SW_MASTER]->read() == LOW) { //if Master Caution is pressed
    for (int i = 0; i < LED_SA_MAX; ++i) {
      saLit[i] = false;
    }
    for (int i = 0; i < SA_COUNT; ++i) {
      saOn[i] = false;
    }
    mcLit = true;
  }
  if (sw[SW_MASTER]->risingEdge()) {
    mcLit = false;
  }
  
  // light SA & MC
  for (int i = 0; i < SA_COUNT; ++i) {
    if (saOn[i])
      saLit[i] = true;
    digitalWrite(LedPin[i], saLit[i]);
  }
  if (mcOn)
    mcLit = true;
  digitalWrite(LedPin[LED_MC], mcLit);
}

