///////////////////////////////////////////////////////////////////////////////
//
// 737 Annunciator (Dataref Input demonstration)
//
// A simulation of the System Annunciators and Master Caution light for the
// Boeing 737-NG.
//
// This is NOT TESTED with X-Plane, as I don't have a capable PC right now.
// It does compile though.
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
  FUEL,
  ELEC,
  APU,
  OVHT, // OVHT/DET
  //AI, // Anti-Ice
  //HYD,
  //DOOR,
  //ENG,
  //OH, // Overhead
  //AC, // Air Conditioning
  SA_COUNT
};

// SystemAnnc LED hardware pins
const int SALedPin[SA_COUNT] = {
  5, //FLT CTRL
  7, //IRS
  6, //FUEL
  4, //ELEC
  2, //APU
  3};//OVHT

////////////////////////////
// Sub-annunciator indexes
//
// These enums represent the number and categories of the sub-annunciators (the
// overhead panel annunciators)
//
// xxx_COUNT is the number of sub-annunciators in that System.
//
// DEBUG
enum FLT_ANNC {
  FLT1,
  //FLT2,
  // add more here
  FLT_COUNT };

enum IRS_ANNC {
  //IRS1,
  // add more here
  IRS_COUNT};

enum FUEL_ANNC {
  FUEL1,
  FUEL2,
  // add more here
  FUEL_COUNT };

enum ELEC_ANNC {
  //ELEC1,
  //ELEC2,
  //ELEC3,
  //ELEC4,
  // add more here
  ELEC_COUNT };

enum APU_ANNC {
  //APU1,
  //APU2,
  // add more here
  APU_COUNT};

enum OVHT_ANNC {
  //OVHT1,
  //OVHT2,
  // add more here
  OVHT_COUNT};

// Warning. Keep this updated if you add more SAs!
const int SUB_COUNT = FLT_COUNT + IRS_COUNT + FUEL_COUNT +
                      ELEC_COUNT + APU_COUNT + OVHT_COUNT;

////////////////////
// Sub-annunciator arrays
//
// These pass the state of one system's sub-annunciators to the SystemAnnc class
//
// DEBUG
bool fltAnnc[FLT_COUNT] = {false};
bool irsAnnc[IRS_COUNT];// = {false};
bool fuelAnnc[FUEL_COUNT] = {false};
bool elecAnnc[ELEC_COUNT];// = {false};
bool apuAnnc[APU_COUNT];// = {false};
bool ovhtAnnc[OVHT_COUNT];// = {false};

// System Annunciator array
SystemAnnc * sa[SA_COUNT];

void setupSA() {
  sa[FLT] = new SystemAnnc(fltAnnc[0], FLT1, FLT_COUNT);
  sa[IRS] = new SystemAnnc(irsAnnc[0], 0,0); //disabled //IRS1, IRS_COUNT);
  sa[FUEL] = new SystemAnnc(fuelAnnc[0], FUEL1, FUEL_COUNT);
  sa[ELEC] = new SystemAnnc(elecAnnc[0], 0,0); //disabled //ELEC1, ELEC_COUNT);
  sa[APU] = new SystemAnnc(apuAnnc[0], 0,0); //disabled //APU1, APU_COUNT);
  sa[OVHT] = new SystemAnnc(ovhtAnnc[0], 0,0); //disabled // OVHT1, OVHT_COUNT);
}

////////////////////
// Simulator input
//
// These datarefs will be used to determine which sub-annunciators are
// currently lit.
//
FlightSimInteger fltDr[FLT_COUNT];
FlightSimInteger irsDr[IRS_COUNT];
FlightSimInteger fuelDr[FUEL_COUNT];
FlightSimInteger elecDr[ELEC_COUNT];
FlightSimInteger apuDr[APU_COUNT];
FlightSimInteger ovhtDr[OVHT_COUNT];

void setupDR() {
  // note: the following datarefs have been selected at semi-random.
  // It is not possible for me to check correct behaviour, or spelling
  // as, at the time of writing, I don't have a working X-Plane installation.
  
  // DEBUG
  fltDr[FLT1] = XPlaneRef("sim/cockpit/switches/yaw_damper_on");
  //fltDr[FLT2] = XPlaneRef("sim/cockpit2/annunciators/autopilot_trim_fail");
  
  //irsDr[IRS1] = XPlaneRef("sim/cockpit2/electrical/dc_voltmeter_selection2");
  
  fuelDr[FUEL1] = XPlaneRef("sim/cockpit2/annunciators/fuel_pressure_low[0]");
  fuelDr[FUEL2] = XPlaneRef("sim/cockpit2/annunciators/fuel_pressure_low[1]");
  
  //elecDr[ELEC1] = XPlaneRef("sim/cockpit2/annunciators/low_voltage");
  //elecDr[ELEC2] = XPlaneRef("sim/cockpit2/annunciators/generator_off[0]");
  //elecDr[ELEC3] = XPlaneRef("sim/cockpit2/annunciators/generator_off[1]");
  //elecDr[ELEC4] = XPlaneRef("sim/cockpit2/annunciators/inverter_off[0]");
  
  //apuDr[APU1] = XPlaneRef("sim/cockpit2/electrical/APU_generator_on");
  //apuDr[APU2] = XPlaneRef("sim/operation/failures/rel_APU_press");
  
  //ovhtDr[OVHT1] = XPlaneRef("sim/cockpit2/annunciators/hvac");
  //ovhtDr[OVHT2] = XPlaneRef("sim/"); //dummy dataref
  
  return;
}

// Master Caution Lit flag
bool masterCautionLit = false;

///////////////////// Switch inputs ////////////////////////
//
// Match each SW_PIN entry with a SwPin entry!!
enum SW_NAMES{
  SW_MASTER = 0,
  SW_SIXPACK,
  SW_COUNT};

const int SwPin[SW_COUNT] = {20, 21}; // switch hardware pins

Bounce * sw[SW_COUNT];

void setupSW() {
  for (int i = 0; i < SW_COUNT; ++i) {
    sw[i] = new Bounce (SwPin[i], 5);
    pinMode(SwPin[i], INPUT_PULLUP );
  }
}

// other LED pins
const int mcPin = 0; //master caution

///////////////////////////// setup /////////////////////////
void setup() {
  // By putting initialisation into a function, we
  // can configure the System Annc array in the same
  // part of the source file as all the input stuff.
  setupSA();
  setupDR();
  
  // switch input
  for (int i = 0; i < SW_COUNT; ++i) {
    sw[i] = new Bounce (SwPin[i], 5);
    pinMode(SwPin[i], INPUT_PULLUP );
  }
  
  // LED output
  for (int i = 0; i < SA_COUNT; ++i) {
    pinMode(SALedPin[i], OUTPUT);
  }
  
  pinMode(mcPin, OUTPUT);
}

////////////////////////////////// loop ////////////////////
void loop() {
  FlightSim.update();
  
  /////////////////////////
  // Assigning values to sub-annc array
  //
  // Here is where anncArray is populated with the state of the
  // sub-annunciators.
  //
  for (int i = 0; i < FLT_COUNT; ++i) {
    fltAnnc[i] = fltDr[i];
  }
  for (int i = 0; i < IRS_COUNT; ++i) {
    irsAnnc[i] = irsDr[i];
  }
  for (int i = 0; i < FUEL_COUNT; ++i) {
    fuelAnnc[i] = fuelDr[i];
  }
  for (int i = 0; i < ELEC_COUNT; ++i) {
    elecAnnc[i] = elecDr[i];
  }
  for (int i = 0; i < APU_COUNT; ++i) {
    apuAnnc[i] = apuDr[i];
  }
  for (int i = 0; i < OVHT_COUNT; ++i) {
    ovhtAnnc[i] = ovhtDr[i];
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
    masterCautionLit = false;
  }
  
  // Light/extinguish Master Caution
  digitalWrite(mcPin, masterCautionLit);
  
  if(sw[SW_SIXPACK]->risingEdge()) { // when sixpack released
    for (int i = 0; i < SA_COUNT; ++i) {
      sa[i]->recall();              // reset all acknowledgements
      sa[i]->setOverride(false);     // clear override
    }
  }
  
  if(sw[SW_SIXPACK]->read() == LOW) { // if sixpack is pressed
    for (int i = 0; i < SA_COUNT; ++i) {
      sa[i]->setOverride(true);       // override each SA to light up
    }
  }
  
  // Light and extinguish SA LEDs based on the state of the SA classes.
  for (int i = 0; i < SA_COUNT; ++i) {
    digitalWrite(SALedPin[i], sa[i]->isLit());
  }
}

