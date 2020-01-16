/*
 * SingleElectrodeRead example for QuickMpr121
 * ===========================================
 * 
 * Sometimes you don't need to read multiple inputs and just one is enough.
 * Reads electrode 0 of each MPR121 (digital, not analog).
 */

#include <QuickMpr121.h>


#define NUM_MPRS 2 // must be using sequential addresses starting at 0x5a, max 4 MPR121s


// create the mpr121 instances
// these will have addresses set automatically
mpr121 mprs[NUM_MPRS];

void setup() {
  for (int i = 0; i < NUM_MPRS; i++) {
    // this special line makes `mpr` the same as typing `mprs[i]`
    mpr121 &mpr = mprs[i];
    
    // `mpr.begin()` sets up the Wire library
    // mpr121 can run in 400kHz mode; if you have issues with it or want 100kHz speed, use `mpr121.begin(100000)`
    // (or use `Wire.begin` and/or `Wire.setClock` directly instead of this)
    mpr.begin();

    // set autoconfig charge level based on 3.2V
    // without this, it will assume 1.8V (a safe default, but not always ideal)
    mpr.autoConfigUSL = 256L * (3200 - 700) / 3200;

    // optional: change touch/release thresholds
    // note: these are only used for digital touch state reads
    mpr.setAllThresholds(15, 10, false);

    // start sensing (for 1 electrode)
    mpr.start(1);
  }

  // open the serial port
  Serial.begin(115200);
  while(!Serial) {} // wait for serial to be ready on USB boards
}

void loop() {  
  for (int i = 0; i < NUM_MPRS; i++) {
    mpr121 &mpr = mprs[i];
    
    // readTouchState with a numeric parameter will read just that electrode as a bool
    Serial.print(mpr.readTouchState(0));
    Serial.print(" ");
  }
  Serial.println();
  delay(10);
}
