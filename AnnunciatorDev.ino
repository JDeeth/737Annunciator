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

////////////////////// Element Annunciator Datarefs ////////////
//
// Datarefs are grouped into these 12 systems.
// Each system has a System Annunciator.
//
// Enable as many entries as you have SA LEDs to illuminate.
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

/////////////////////////
// Dataref index
//
// The values FLT, FLT2 etc, represent datarefs for overhead FLT CTRL annunciators
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
// For example, IRS1 = FLT_MAX
// Be careful to keep this convention!!
//
enum SUB_ANNUNCIATOR_INDEX{
  FLT1 = 0,  // the value which follows any _MAX value
  FLT2,      // MUST equal the preceding value!!
  FLT_MAX,
  IRS1 = FLT_MAX,
  IRS2,
  IRS_MAX,
  FUEL1 = IRS_MAX,
  FUEL2,
  FUEL_MAX,
  ELEC1 = FUEL_MAX, // You can have as many or as few datarefs in each
  ELEC2,            // category as you choose. I've arbitrarily picked
  ELEC3,            // two in each category and four in ELEC.
  ELEC4,
  ELEC_MAX,
  APU1 = ELEC_MAX,
  APU2,
  APU_MAX,
  OVHT1 = APU_MAX,
  OVHT2,
  OVHT_MAX,
  SUB_COUNT = OVHT_MAX};

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
bool anncArray[SUB_COUNT] = {0};

FlightSimInteger dr[SUB_COUNT];
FlightSimFloat magHeading;

void setupDR() {
  // note: the following datarefs have been selected at semi-random.
  // It is not possible for me to check correct behaviour, or spelling
  // as, at the time of writing, I don't have a working X-Plane installation.
  dr[FLT1] = XPlaneRef("sim/cockpit/switches/yaw_damper_on");
  dr[FLT2] = XPlaneRef("sim/cockpit2/annunciators/autopilot_trim_fail");
  dr[IRS1] = XPlaneRef("sim/cockpit2/electrical/dc_voltmeter_selection2");
  dr[IRS2] = XPlaneRef("sim/operation/failures/rel_efis_1");
  dr[FUEL1] = XPlaneRef("sim/cockpit2/annunciators/fuel_pressure_low[0]");
  dr[FUEL2] = XPlaneRef("sim/cockpit2/annunciators/fuel_pressure_low[1]");
  dr[ELEC1] = XPlaneRef("sim/cockpit2/annunciators/low_voltage");
  dr[ELEC2] = XPlaneRef("sim/cockpit2/annunciators/generator_off[0]");
  dr[ELEC3] = XPlaneRef("sim/cockpit2/annunciators/generator_off[1]");
  dr[ELEC4] = XPlaneRef("sim/cockpit2/annunciators/inverter_off[0]");
  dr[APU1] = XPlaneRef("sim/cockpit2/electrical/APU_generator_on");
  dr[APU2] = XPlaneRef("sim/operation/failures/rel_APU_press");
  dr[OVHT1] = XPlaneRef("sim/cockpit2/annunciators/hvac");
  dr[OVHT2] = XPlaneRef("sim/"); //dummy dataref
  
  magHeading = XPlaneRef("sim/cockpit2/gauges/indicators/compass_heading_deg_mag");
  
  return;
}

// System Annunciator array
SystemAnnc * sa[SA_COUNT];

void setupSA() {
  sa[FLT] = new SystemAnnc(anncArray[0], FLT1, FLT_MAX);
  sa[IRS] = new SystemAnnc(anncArray[0], IRS1, IRS_MAX);
  sa[FUEL] = new SystemAnnc(anncArray[0], FUEL1, FUEL_MAX);
  sa[ELEC] = new SystemAnnc(anncArray[0], ELEC1, ELEC_MAX);
  sa[APU] = new SystemAnnc(anncArray[0], APU1, APU_MAX);
  sa[OVHT] = new SystemAnnc(anncArray[0], OVHT1, OVHT_MAX);
  // etc
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
  LED_FLT,
  LED_IRS,
  LED_FUEL,
  LED_ELEC,
  LED_APU,
  LED_OVHT, // OVHT/DET
  //LED_AI, // Anti-Ice
  //LED_HYD,
  //LED_DOOR,
  //LED_ENG,
  //LED_OH, // Overhead
  //LED_AC, // Air Conditioning
  LED_SA_COUNT};

// SystemAnnc LED hardware pins
const int LedPin[LED_SA_COUNT] = {
  5, //FLT CTRL
  7, //IRS
  6, //FUEL
  4, //ELEC
  2, //APU
  3};//OVHT

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
  for (int i = 0; i < LED_SA_COUNT; ++i) {
    pinMode(LedPin[i], OUTPUT);
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
  for (int i = 0; i < SUB_COUNT; ++i) {
    anncArray[i] = dr[i]; //very simple, eh?
  }
  
  // Demonstration of creating virtual sub-annunciator from an arbitrary
  // floating point dataref.
  // If the magnetic heading is between 90 and 180, the OVHT System Annunciator
  // will be triggered.
  if (magHeading > 90 && magHeading < 180) {
    dr[OVHT2] = true;
  } else {
    dr[OVHT2] = false;
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
  digitalWrite(mcPin, masterCautionLit);
  
  if(sw[SW_SIXPACK]->risingEdge()) { //when sixpack released
    for (int i = 0; i < SA_COUNT; ++i) {
      sa[i]->recall();              // reset all acknowledgements
    }
    for (int i = 0; i < LED_SA_COUNT; ++i) {
      digitalWrite(LedPin[i], LOW); // and extinguish all SAs.
    }                               // They will be relit next loop() if called for.
  }
  
  if(sw[SW_SIXPACK]->read() == LOW) { //if sixpack is pressed
    for (int i = 0; i < LED_SA_COUNT; ++i) {
      digitalWrite(LedPin[i], HIGH); //light all the System Annunciators
    }
  } else {                           // if sixpack not pressed
    for (int i = 0; i < SA_COUNT; ++i) {
      digitalWrite(LedPin[i], sa[i]->isLit()); //light SAs if called for
    }
  }
  
  // voila!
}

