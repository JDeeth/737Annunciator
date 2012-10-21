///////////////////////////////////////////////////////////////////////////////
//
// 737 Annunciator (Hardware input demonstration)
//
// A simulation of the System Annunciators and Master Caution light for the
// Boeing 737-NG.
//
// This version does not interface with X-Plane at all. It is a completely
// self-contained demonstration of the behaviour of the the 737 warning system.
//
// The System Annunciators (SA) are located prominently on the glareshield, in two
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
#include "SystemAnnc.h"

//////////////////////
// System Annunciator index
//
// A System Annunciator represents a group of sub-annunciators from the
// overhead panel. The 737-NG has twelve.
//
// Enable as many entries as you have SA LEDs to illuminate.
//
// SA_COUNT is the number of System Annunciators in your system.
//
enum SYS_ANNC{
  FLT, //Flight Controls
  IRS,
  //FUEL,
  //ELEC,
  //APU,
  //OVHT, // OVHT/DET
  //AI, // Anti-Ice
  //HYD,
  //DOOR,
  //ENG,
  //OH, // Overhead
  //AC, // Air Conditioning
  SA_COUNT
};

////////////////////////////
// Sub-annunciator index
//
// This enum represents the number and categories of the sub-annunciators (the
// overhead panel annunciators)
//
// The _MAX values do not represent an individual annunciator. They are used
// for calculating the number of annunciators in an individual category.
//
// For instance, IRS_MAX - IRS1 is the number of IRS sub-annunciators.
//
// SUB_COUNT is the total number of sub-annunciators in your system.
//
// To make the maths work out correctly, every entry following a _MAX entry
// must be assigned to be equal to the _MAX entry.
// For example, IRS1 = FLT_MAX
// Be careful to keep this convention!!
//
enum SUB_ANNUNCIATOR_INDEX{
  FLT1 = 0,  // the value which follows any _MAX value
  FLT2,      // MUST equal the preceding value!!
  FLT_MAX,
  IRS1 = FLT_MAX,
  IRS_MAX,
  SUB_COUNT = IRS_MAX};

////////////////////
// Hardware for controlling sub-annunciators
//
const int DRPin[SUB_COUNT] = {0, 4, 8};
Bounce * dr[SUB_COUNT];
//
// Because I currently don't have a working X-Plane install,
// I cannot test this with actual datarefs. So I'm using a
// bunch of switches as a substitute.
//
////////////////////

////////////////////
// Sub-annunciator array
//
// This passes the state of the sub-annunciators to the SystemAnnc class
// I chose to use one long array to cover all SA sub-annunciators, so it could
// be entirely populated using one 'for' loop, but it would be equally
// appropriate to use multiple short arrays, with one for each SA.
//
// Write your own code in loop() to populate the arrays(s) with the state
// of the sub-annunciators. With this method, you have flexibility in choosing
// a data-source for the state of the sub-annunciators (Teensyduino dataref,
// SDK dataref, plugin internal data, hardware...)
//
bool anncArray[SUB_COUNT] = {0};

// System Annunciator array
SystemAnnc * sa[SUB_COUNT];

void setupSA() {
  sa[FLT] = new SystemAnnc(anncArray[0], FLT1, FLT_MAX);
  sa[IRS] = new SystemAnnc(anncArray[0], IRS1, IRS_MAX);
  //sa[FUEL] = new SystemAnnc(anncArray[0], FUEL1, FUEL_MAX);
  // etc
}

// Master Caution Lit flag
bool masterCautionLit = false;

///////////////////// Switch inputs ////////////////////////
//
// Index of hardware input switches.
//
// SW_COUNT is the number of input switches.
//
// Be sure to match each SW_PIN entry with a SwPin entry!!
//
enum SW_PIN{
  SW_MASTER = 0,
  SW_SIXPACK,
  SW_COUNT};

const int SwPin[SW_COUNT] = {12, 16}; // switch hardware pins

Bounce * sw[SW_COUNT]; // will be populated with Bounce(SwPin) in setup()

///////////////////////// LED outputs /////////////////
//
// Index of output LEDs.
//
// LED_SA_MAX is the number of System Annunciator LEDs in the system.
//
// LED_COUNT is the number of LEDs in the system.
//
// Be sure to match each LED_PIN entry with a LedPin entry!!
//
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
  for (int i = 0; i < SUB_COUNT; ++i) {
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
  for (int i = 0; i < SUB_COUNT; ++i) {
    dr[i]->update();
    anncArray[i] = !(dr[i]->read());
  }

  ////////////////////////
  // Update input switches
  //
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
    masterCautionLit = false;        // Master Caution is extinguished while
                                     // reset button is pressed. It will 
                                     // relight if faults still exist when it
                                     // is released.
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

