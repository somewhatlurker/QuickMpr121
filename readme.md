# QuickMpr121 Arduino library by somewhatlurker
===============================================

This is a library for using MPR121 capacitive touch sensing ICs.
It's designed to be as easy to configure as possible -- changing most settings just requires setting a variable before calling start.  
It does use more memory than most libraries, but it's not unmanageable on most MCUs.

MPR121 offers 12 capacitive touch sensing pins, 8 of which can also be used as GPIO if not used for sensing.
It also allows for proximity detection by combining some or all sensing electrodes into one larger electrode.  
Up to four MPR121s can be used on one I2C bus by changing their address (address pin can be connected to different references).

This library implements full digital or analog sensing, and GPIO with PWM.
It also allows configuration of autoconfig and important sampling/filtering parameters.

　

### Basic usage
```
#include <QuickMpr121.h>

...

mpr121 mpr = mpr121(address);
// optionally set custom parameters
// for different response time (default is 4ms): `mpr.SFI = MPR_SFI_XXX; mpr.ESI = MPR_ESI_XXX;`
// for better autoconfig: `mpr.autoConfigUSL = 256L(supplyMillivolts - 700) / supplyMillivolts;`

mpr121.begin(); // just sets up the Wire lib. mpr121 can run in 400kHz mode. if you have issues with it or want to use 100kHz, use `mpr121.begin(100000)`.
mpr.start();

short touches = mpr.readTouchState();
bool touch0 = bitRead(touches, 0);
```

More complete examples are in the examples folder (accessible in Arduino IDE menus).  
Full docs are at docs/html/index.html.

　

### Important notes
Reading data isn't thread-safe, but that shouldn't be an issue for most use cases.

Also note that some result buffers (returned by some functions) are shared between instances to save memory.
Process or save data for one mpr121 before reading data from the next (or change the MPR121_SAVE_MEMORY define to false to avoid this).

Changes to properties won't take effect until you restart the MPR121.


AN**** numbers in docs/comments refer to application notes, available on the NXP website.

　

### Copyright
Copyright 2020 somewhatlurker, MIT license