/*
 * BaselineRead example for QuickMpr121
 * ================================
 * 
 * Logs analog touch readings adjusted with baseline offsets to serial
 */

#include <QuickMpr121.h>


#define NUM_MPRS 2 // must be using sequential addresses starting at 0x5a, max 4 MPR121s
#define PROXIMITY_ENABLE false


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

    // enable proximity sensing if it's set to true
    if (PROXIMITY_ENABLE)
      mpr.proxEnable = MPR_ELEPROX_0_TO_11;
    else
      mpr.proxEnable = MPR_ELEPROX_DISABLED;

    // start sensing
    mpr.start(12);
  }

  // open the serial port
  Serial.begin(115200);
  while(!Serial) {} // wait for serial to be ready on USB boards
}

void loop() {
  int numElectrodes;
  if (PROXIMITY_ENABLE)
    numElectrodes = 13;
  else
    numElectrodes = 12;
  
  for (int i = 0; i < NUM_MPRS; i++) {
    mpr121 &mpr = mprs[i];
    
    // readElectrodeData is used to read analog values
    // it returns a pointer to a short
    // in this usage, a pointer works just like an array would and can be accessed like values[j]
    short* values = mpr.readElectrodeData(0, numElectrodes); // read all electrodes, starting from electrode 0

    // it's also possible to read a single value as a regular short like this:
    // `short value = mpr.readElectrodeData(0); // only read electrode 0`

    
    // readElectrodeBaseline works like readElectrodeData, but returns a pointer to a byte
    byte* baselines = mpr.readElectrodeBaseline(0, numElectrodes); // read all baselines, starting from electrode 0

    // it can also read a single value:
    // `byte baseline = mpr.readElectrodeBaseline(0); // only read baseline for electrode 0`

    // baselines are read as 8 bit numbers, but they must be compared to larger 10 bit numbers.
    // they can be converted by "bit shifting" them by two bits: `((short)baselines[j] << 2)`
    
    for (int j = 0; j < numElectrodes; j++) {
      // subtract the baseline from the raw value to center the untouched level at 0
      short value = values[j] - ((short)baselines[j] << 2);
      Serial.print(value);
      Serial.print(" ");
    }
    
    Serial.print(" ");
  }
  Serial.println();
  delay(10);
}
