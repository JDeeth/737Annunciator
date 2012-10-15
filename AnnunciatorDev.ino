#include <Bounce.h>

////////////////////////////SystemAnnc class
//
const int MAX_DATAREFS_PER_SYSTEM = 20;

class SystemAnnc {
public:
  SystemAnnc(const bool &dataref, const int& beginDataref, const int& endDataref);
  bool foo();
  bool lit();
  bool turnOn();
  void recall();
  void reset();
private:
  const bool * dataref_;
  const int beginDataref_; // start and end of dataref array
  const int endDataref_;   // for DRs belonging to this SA
  bool isLit_;
  bool turnOn_;
  bool ack_[MAX_DATAREFS_PER_SYSTEM];
};

//////// SystemAnnc (int& dataref, int begin, int end)
//
// Requires an int array where 0 = 
SystemAnnc::SystemAnnc (const bool& dataref, const int& beginDataref, const int& endDataref) :
  dataref_(&dataref),
  beginDataref_(beginDataref),
  endDataref_(endDataref)
{
  isLit_ = false;
  turnOn_ = false;
  memset(ack_, 0, sizeof(ack_));
}

bool SystemAnnc::turnOn() {
  if(turnOn_)
    isLit_ = true;
  return isLit_;
}

bool SystemAnnc::lit() {
  return isLit_;
}

bool SystemAnnc::foo() {
  bool mcOn = false;
  for (int i = beginDataref_, j = 0; i < endDataref_; ++i, ++j) {
    if(dataref_[i]) { //if overhead annunciator is lit
      if(!ack_[j]) {  // if it has not yet caused its SA to light
        turnOn_ = true; // light the SA
        ack_[j] = true; // but only once
      }
      mcOn += true;
    } else { // if overhead annunciator not lit
      ack_[j] = false; // reset acknowledgement
      mcOn += false;
    }
  }
  return mcOn;
}

void SystemAnnc::recall() {
  for (int i = 0; i < endDataref_ - beginDataref_; ++i) {
    ack_[i] = false;
  }
}

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

// Substitute my int array for a FlightSimInteger array when using real datarefs
//FlightSimInteger dataref[DR_COUNT];
bool dataref[DR_COUNT] = {0}; 

// System Annunciator array
SystemAnnc * sa[SA_COUNT];

void setupSA() {
  sa[FC] = new SystemAnnc(dataref[0], FC1, FC_MAX);
  sa[IRS] = new SystemAnnc(dataref[0], IRS1, IRS_MAX);
  //sa[FUEL] = new SystemAnnc(dataref[0], FUEL1, FUEL_MAX);
  // etc
  // Use Dataref[0] and values from DATAREF_INDEX for each entry!
}

// Master Caution Lit flag
bool mcLit = false;

///////////////////// Switch inputs ////////////////////////
//
// Match each SW_PIN entry with a SwPin entry!!
enum SW_PIN{
  SW_MASTER = 0,
  SW_SIXPACK,
  SW_COUNT};

const int SwPin[SW_COUNT] = {12, 16};

Bounce * sw[SW_COUNT];

///////////////////////// LED outputs /////////////////
enum LED_PIN{
  LED_FC,
  LED_IRS,
  // add more System Annunciator LEDs here
  LED_SA_MAX,
  LED_MC = LED_SA_MAX, // as before, each entry following a _MAX
  LED_COUNT};          // must be assigned the value of the _MAX

const int LedPin[LED_COUNT] = {
  44, 45, LED_BUILTIN };

// End of configuration.
//
// With luck, everything below this point will work without
// any alteration, no matter how much you add above.
//
/////////////////////////////////

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
  
  for (int i = 0; i < SA_COUNT; ++i) {
    mcLit += sa[i]->foo();
  }
  
  ///////////////////////////////////
  // Input handling
  //  
  if(sw[SW_MASTER]->read() == LOW) { //if Master Caution is pressed
    for (int i = 0; i < SA_COUNT; ++i) {
      sa[i]->reset();                // reset each System Annc
    }
    mcLit = true;                    // light the Master Caution
  }
  if (sw[SW_MASTER]->risingEdge()) { // when MC is released
    mcLit = false;                   // extinguish Master Caution
  }
  
  // Light/extinguish Master Caution
  digitalWrite(LedPin[LED_MC], mcLit);
  
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
      digitalWrite(LedPin[i], sa[i]->turnOn()); //light SAs if called for
    }
  }
  
  // voila!
}

