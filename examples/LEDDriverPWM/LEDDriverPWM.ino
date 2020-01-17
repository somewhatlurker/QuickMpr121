/*
 * LEDDriverPWM example for QuickMpr121
 * ====================================
 * (warning: untested)
 * 
 * The same as LEDDriver, but brightness fades in and out.
 * 
 * Displays a light pattern on the n (default 8) highest pins of each MPR121.
 * Connect LED anodes to MPR121 via a suitable current-limiting resistor (~100 Ohms should be fine for most LEDs).
 * Connect LED cathodes to ground.
 */

#include <QuickMpr121.h>


#define NUM_MPRS 2 // must be using sequential addresses starting at 0x5a, max 4 MPR121s
#define PINS_PER_MPR 8 // use all 8 GPIO-capable pins, max is 8


// don't change this
#define FIRST_PIN_PER_MPR (12 - PINS_PER_MPR) // subtract number of pins from 12 to find where to start on each mpr


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

    // no need to set sensing parameters or start sensing for GPIO

    // setGPIOMode enables and sets the mode for consecutive GPIO pins.
    // omitting the count (`PINS_PER_MPR`) will set the mode for a single pin
    // for a full list of modes, you can find setGPIOMode in docs then click on the link to mpr121GPIOMode
    mpr.setGPIOMode(FIRST_PIN_PER_MPR, PINS_PER_MPR, MPR_GPIO_MODE_OUTPUT_OPENDRAIN_HIGH);

    // set the state of consecutive outputs to false
    // omitting the count (`8`) will set a single pin
    mpr.writeGPIODigital(4, 8, false); // turn off 8 GPIO outputs starting at 4 (first valid pin). (this is all pins)
  }
}


void loop() {
  for (int i = 0; i < NUM_MPRS; i++) {
    mpr121 &mpr = mprs[i];
    
    for (int j = 0; j < PINS_PER_MPR; j++) {
      // workaround the IC logic bug by disabling pin 10, then setting its data to true
      if (FIRST_PIN_PER_MPR + j == 9) {
        // no count used here, so just sets for one pin
        mpr.setGPIOMode(10, MPR_GPIO_MODE_DISABLED);
        mpr.writeGPIODigital(10, true);
      }


      // brightness is from sin of time
      // dividing milliseconds by 318 should make a cycle last ~2s
      // multiply by 8 then add 8 to make the range 0 to 16
      // (16 is absolute peak, and data in 15 to 15.999... should closely match 0 to 0.999...)
      int brightness = sin(millis()/318) * 8.0f + 8.0f;

      // ensure brightness is definitely in the valid range of values
      if (brightness < 0)
        brightness = 0;
      else if (brightness > 15)
        brightness = 15;

      
      // turn on the pin with a specified brightness using writeGPIOAnalog
      // no count used here, so just sets for one pin
      mpr.writeGPIOAnalog(FIRST_PIN_PER_MPR + j, brightness);

      delay(50);
      
      // turn off the pin using writeGPIOAnalog (writeGPIODigital works too)
      // no count used here, so just sets for one pin
      mpr.writeGPIOAnalog(FIRST_PIN_PER_MPR + j, 0);


      // restore pin 10's state by setting its data to false then setting its mode back
      if (FIRST_PIN_PER_MPR + j == 9) {
        // no count used here, so just sets for one pin
        mpr.writeGPIODigital(10, false);
        mpr.setGPIOMode(10, MPR_GPIO_MODE_OUTPUT_OPENDRAIN_HIGH);
      }
    }
  }
}
