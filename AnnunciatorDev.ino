///////////////////////////////////////////////////////////////////////////////
//
// 737 Annunciator
//
// A simulation of the System Annunciators and Master Caution light for the
// Boeing 737-NG.
//
// The System Annunciators are located prominently on the glareshield, in two
// clusters of six, referred to as a 'six-pack'. They represent multiple sub-
// annunciators, all located less visibly on the overhead panel. When one of
// the sub-annunciators lights up, the associated System Annunciator lights up
// too, along with the Master Caution light. Pressing the Master Caution light
// extinguishes (resets) the SAs - they will light up if a new sub-annunciator
// becomes active. Pressing the six-pack lights all the SAs for bulb testing.
// When the six-pack is released, any SA with an active sub-annunciator will
// remain lit.
//
// This code is written for the PJRC Teensy board, v2.0 or higher, using the
// Arduino+Teensyduino framework and driven by X-Plane. I believe it could be
// adapted for use in a plugin - feel free to use it for this!
//
// With thanks to Anthony Musaluke and b737.org.uk for reference information.
//
// Copyright 2012 Jack Deeth
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// I would appreciate, but not insist, on attribution if this code is
// incorporated into other projects.
//
///////////////////////////////////////////////////////////////////////////////

#include <Bounce.h>

//////////////////////////// SystemAnnc class /////////////////////////////////
//
// This class represents an individual System Annunciator on the glareshield of
// the Boeing 737-NG.
//

// Maximum sub-annunciators to be grouped with any one System Annunciator
const int MAX_ANNCS_PER_SYSTEM = 20;

class SystemAnnc {
public:
  SystemAnnc(const bool &anncArray, const int& beginAnnc, const int& endAnnc);
  bool checkSubAnncs();
  bool isLit();
  void recall();
  void reset();
private:
  const bool * annc_;
  const int beginAnnc_; // start and end of dataref array
  const int endAnnc_;   // for DRs belonging to this SA
  bool isLit_;
  bool turnOn_;
  bool ack_[MAX_ANNCS_PER_SYSTEM]; // Acknowledgement. When the reset button is
                                   // pressed, this Acknowledges all the
                                   // presently active sub-annunciators,
                                   // preventing them from relighting the SA
                                   // until they extinguish and relight or the
                                   // recall button is pressed.
};

//////// SystemAnnc (bool& anncArray, int begin, int end)
//
// Takes an array of booleans representing the sub-annunciators
// for that system (the multitude of overhead-panel annunciators)
// along with the indexes of the first and last annunciators
// belonging to this System Annunciator
//
SystemAnnc::SystemAnnc (const bool &anncArray, const int& beginAnnc, const int& endAnnc) :
  annc_(&anncArray),
  beginAnnc_(beginAnnc),
  endAnnc_(endAnnc)
{
  isLit_ = false;
  turnOn_ = false;
}

///////// bool checkSubAnncs()
//
// Iterates through the sub-annunciators, checking to see if any are active.
// If active (boolean true) sub-annunciators are found, the SystemAnnc will be
// marked as lit, and the function will return true.
//
bool SystemAnnc::checkSubAnncs() {
  bool activeFound = false;
  for (int i = beginAnnc_, j = 0; i < endAnnc_; ++i, ++j) {
    if(annc_[i]) {      //if sub-annunciator is lit
      activeFound += true;
      if(!ack_[j]) {    // and it has not yet caused its SA to light
        turnOn_ = true; // flag the SA to be lit later
        ack_[j] = true; // latch to stop the SA being relit by this sub-annc
      }
    } else {            // if overhead annunciator not lit
      ack_[j] = false;  // reset acknowledgement
    }
  }
  return activeFound;
}

////////// bool isLit
//
// returns true if System Annunciator should be lit
// returns false otherwise
//
bool SystemAnnc::isLit() {
  if(turnOn_)
    isLit_ = true;
  return isLit_;
}

/////////// void recall
//
// Resets acknowledgements, so any active subannunciator will cause the System
// Annunciator to be lit when checkSubAnncs is run again.
//
void SystemAnnc::recall() {
  for (int i = 0; i < endAnnc_ - beginAnnc_; ++i) {
    ack_[i] = false;
  }
}

/////////// void reset()
//
// Extinguishes SA. Currently active subannunciators will NOT cause it to relight until
// recall() is run.
//
void SystemAnnc::reset() {
  isLit_ = false;
  turnOn_ = false;
}
/////////////////////////////////////

////////////////////// Element Annunciator Datarefs ////////////
//
// Datarefs are grouped into these 12 systems.
// Each system has a System Annunciator.
//
// Enable as many entries as you have SA LEDs to illuminate.
//
enum SYS_ANNC{
  FC, //Flight Controls
  IRS,
  //FUEL,
  //ELEC,
  //APU,
  //OD, // OVHT/DET
  //AI, // Anti-Ice
  //HYD,
  //DOOR,
  //ENG,
  //OH, // Overhead
  //AC, // Air Conditioning
  SA_COUNT
};

// Dataref index
//
// The values FC1, FC2 etc, represent datarefs for overhead FLT CTRL annunciators
// IRS1 etc represents IRS annunciators
// The _MAX values do not represent an individual dataref. They are used
// for calculating the number of datarefs in an individual category.
//
// For instance, IRS_MAX - IRS1 is the number of FLT CTRL datarefs.
//
// DR_COUNT is the number of datarefs overall.
//
// To make the maths work out correctly, every entry following a _MAX entry
// must be assigned to be equal to the _MAX entry.
// For example, IRS1 = FC_MAX
// Be careful to keep this convention!!
//
enum DATAREF_INDEX{
  FC1 = 0,  // the value which follows any _MAX value
  FC2,      // MUST equal the preceding value!!
  FC_MAX,
  IRS1 = FC_MAX,
  IRS_MAX,
  DR_COUNT = IRS_MAX};

////////////////////
//
// these are for my dataref-simulating input switches
const int DRPin[DR_COUNT] = {0, 4, 8};
Bounce * dr[DR_COUNT];
//
// Because I currently don't have a working X-Plane install,
// I cannot test this with actual datarefs. So I'm using a
// bunch of switches as a substitute.
//
////////////////////

////////////////////
// sub-annunciator array(s)
//
// This passes the state of the sub-annunciators to the SystemAnnc class
// I chose to use one long array to cover all SA sub-annunciators, so it could
// be entirely populated using one for loop, but it would be equally
// appropriate to use multiple short arrays, one per SA.
// 
// Write your own code in loop() to populate this/these array/s with the state
// of the sub-annunciators. This way you have flexibility to decide what can be
// used as a sub-annunciator.
//
bool anncArray[DR_COUNT] = {0};

// System Annunciator array
SystemAnnc * sa[SA_COUNT];

void setupSA() {
  sa[FC] = new SystemAnnc(anncArray[0], FC1, FC_MAX);
  sa[IRS] = new SystemAnnc(anncArray[0], IRS1, IRS_MAX);
  //sa[FUEL] = new SystemAnnc(dataref[0], FUEL1, FUEL_MAX);
  // etc
  // Use Dataref[0] and values from DATAREF_INDEX for each entry!
}

// Master Caution Lit flag
bool masterCautionLit = false;

///////////////////// Switch inputs ////////////////////////
//
// Match each SW_PIN entry with a SwPin entry!!
enum SW_PIN{
  SW_MASTER = 0,
  SW_SIXPACK,
  SW_COUNT};

const int SwPin[SW_COUNT] = {12, 16}; // switch hardware pins

Bounce * sw[SW_COUNT];

///////////////////////// LED outputs /////////////////
enum LED_PIN{
  LED_FC,
  LED_IRS,
  // add more System Annunciator LEDs here
  LED_SA_MAX,
  LED_MC = LED_SA_MAX, // as before, each entry following a _MAX
  LED_COUNT};          // must be assigned the value of the _MAX

const int LedPin[LED_COUNT] = { // LED hardware pins
  44, 45, LED_BUILTIN };

///////////////////////////// setup /////////////////////////
void setup() {
  // By putting initialisation into a function, we
  // can configure the System Annc array in the same
  // part of the source file as all the input stuff.
  setupSA();

  // My hardware-simulated "datarefs"
  for (int i = 0; i < DR_COUNT; ++i) {
    dr[i] = new Bounce (DRPin[i], 5);
    dr[i]->write(HIGH);
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

////////////////////////////////// loop ////////////////////
void loop() {
  
  /////////////////////////
  // Assigning values to sub-annc array[]
  //
  // Here is where anncArray is populated with the state of the 
  // sub-annunciators. In this offline testing code, the sub-annunciator
  // datarefs are represented by switches.
  //
  for (int i = 0; i < DR_COUNT; ++i) {
    dr[i]->update();
    anncArray[i] = !(dr[i]->read());
  }
  
  ////////////////////////
  // Update input switches
  for (int i = 0; i < SW_COUNT; ++i) {
    sw[i]->update();
  }

  ////////////////////////////////////
  // Update System Annunciators
  //
  for (int i = 0; i < SA_COUNT; ++i) {
    masterCautionLit += sa[i]->checkSubAnncs();
  }

  ///////////////////////////////////
  // Input handling
  //
  if(sw[SW_MASTER]->read() == LOW) { //if Master Caution is pressed
    for (int i = 0; i < SA_COUNT; ++i) {
      sa[i]->reset();                // reset each System Annc
    }
    masterCautionLit = true;                    // light the Master Caution
  }
  if (sw[SW_MASTER]->risingEdge()) { // when MC is released
    masterCautionLit = false;                   // extinguish Master Caution
  }

  // Light/extinguish Master Caution
  digitalWrite(LedPin[LED_MC], masterCautionLit);

  if(sw[SW_SIXPACK]->risingEdge()) { //when sixpack released
    for (int i = 0; i < SA_COUNT; ++i) {
      sa[i]->recall();              // reset all acknowledgements
    }
    for (int i = 0; i < LED_SA_MAX; ++i) {
      digitalWrite(LedPin[i], LOW); // and extinguish all SAs.
    }                               // They will be relit next loop() if called for.
  }

  if(sw[SW_SIXPACK]->read() == LOW) { //if sixpack is pressed
    for (int i = 0; i < LED_SA_MAX; ++i) {
      digitalWrite(LedPin[i], HIGH); //light all the System Annunciators
    }
  } else {                           // if sixpack not pressed
    for (int i = 0; i < SA_COUNT; ++i) {
      digitalWrite(LedPin[i], sa[i]->isLit()); //light SAs if called for
    }
  }

  // voila!
}

