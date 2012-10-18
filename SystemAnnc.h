#ifndef SystemAnnc_h
#define SystemAnnc_h

//////////////////////////// SystemAnnc class /////////////////////////////////
//
// This class represents an individual System Annunciator on the glareshield of
// the Boeing 737-NG.
//

// Maximum sub-annunciators to be grouped with any one System Annunciator
const int MAX_ANNCS_PER_SYSTEM = 12;

class SystemAnnc {
public:
  SystemAnnc(const bool &anncArray, const unsigned int& beginAnnc, const unsigned int& endAnnc);
  bool checkSubAnncs();
  bool isLit();
  void recall();
  void reset();
  bool setOverride(bool lit);
private:
  const bool * annc_;
  const unsigned int beginAnnc_; // start and end of dataref array
  const unsigned int endAnnc_;   // for DRs belonging to this SA
  bool isLit_;
  bool turnOn_;
  bool overrideLit_;
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
SystemAnnc::SystemAnnc (const bool &anncArray, const unsigned int& beginAnnc, const unsigned int& endAnnc) :
  annc_(&anncArray),
  beginAnnc_(beginAnnc),
  endAnnc_(endAnnc)
{
  isLit_ = false;
  turnOn_ = false;
  overrideLit_ = false;
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
  if (overrideLit_)
      return true;
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

/////////// bool setOverride(bool lit)
//
// Activates or deactivates the light-on override. For bulb testing.
//
bool SystemAnnc::setOverride(bool lit) {
  overrideLit_ = lit;
}

/////////////////////////////////////

#endif
