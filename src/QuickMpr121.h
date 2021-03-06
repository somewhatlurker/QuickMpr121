/** \mainpage QuickMpr121 Documentation
 * QuickMpr121 Arduino library by somewhatlurker
 * =============================================
 * 
 * QuickMpr121 is a library for using MPR121 capacitive touch sensing ICs.
 * It's designed to be as easy to configure as possible -- changing most settings just requires setting a variable before calling start.
 * 
 * Documentation is generated for all public mpr121 members.
 *
 * For more basic information including a "quick start", check the project readme file or examples.
 * 
 * 　
 * 
 * \section notes Important notes
 * Reading data isn't thread-safe, but that shouldn't be an issue for most use cases.
 * 
 * Also note that some result buffers (returned by some functions) are shared between instances to save memory.
 * Process or save data for one mpr121 before reading data from the next (or change the MPR121_SAVE_MEMORY define to false to avoid this).
 * 
 * Changes to properties won't take effect until you restart the MPR121.
 * 
 * 
 * AN**** numbers in docs/comments refer to application notes, available on the NXP website.
 * 
 * 　
 * 
 * \section copyright Copyright
 * Copyright 2020 somewhatlurker, MIT license
 */

#pragma once
#include "Arduino.h"
#include "Wire.h"
#include "QuickMpr121Enums.h"


// please also update `PREDEFINED` in Doxyfile if changing any defines

#ifndef MPR121_I2C_BUFLEN
#define MPR121_I2C_BUFLEN 26 // note: arduino Wire library defines BUFFER_LENGTH as 32, so much larger values won't work
#endif

// use bitfields (stored in short) instead electrodeTouchBuf and electrodeOORBuf
#define MPR121_USE_BITFIELDS true

// make some buffers static (shared between instances) to save memory
#define MPR121_SAVE_MEMORY true


// define DEPRECATED so the same syntax can be used for any compiler without issues
#if __GNUC__
  # if (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 5)) // GCC 4.5+ supports deprecated with msg
    #define DEPRECATED(func, msg) func __attribute__ ((deprecated(msg)))
  #else // GCC 4.5+
    #define DEPRECATED(func, msg) func __attribute__ ((deprecated))
  #endif // GCC 4.5+
#else // __GNUC__
  #warning "Please implement DEPRECATED for your compiler"
  #define DEPRECATED(func, msg) func
#endif // __GNUC__


// pin names (mainly useful for LED pins)
#define MPR_ELE0 0
#define MPR_ELE1 1
#define MPR_ELE2 2
#define MPR_ELE3 3
#define MPR_ELE4 4
#define MPR_ELE5 5
#define MPR_ELE6 6
#define MPR_ELE7 7
#define MPR_ELE8 8
#define MPR_ELE9 9
#define MPR_ELE10 10
#define MPR_ELE11 11
#define MPR_ELEPROX 12

#define MPR_LED0 MPR_ELE4
#define MPR_LED1 MPR_ELE5
#define MPR_LED2 MPR_ELE6
#define MPR_LED3 MPR_ELE7
#define MPR_LED4 MPR_ELE8
#define MPR_LED5 MPR_ELE9
#define MPR_LED6 MPR_ELE10
#define MPR_LED7 MPR_ELE11


/**
 * Main mpr121 class.
 * Use one instance per MPR121.
 */
class mpr121 {
private:
  byte i2cAddr; ///< I2C address from constructor
  TwoWire* i2cWire; ///< TwoWire* from constructor

  static byte i2cReadBuf[MPR121_I2C_BUFLEN]; ///< Intermediate buffer for raw I2C reads
  
  #if MPR121_SAVE_MEMORY
    static short electrodeDataBuf[13]; ///< Return buffer for analog electrode data
    static byte electrodeBaselineBuf[13]; ///< Return buffer for electrode baselines
    static byte electrodeCDCBuf[13]; ///< Return buffer for electrode CDC
    static mpr121FilterCDT electrodeCDTBuf[13]; ///< Return buffer for electrode CDT
    #if !MPR121_USE_BITFIELDS
      bool electrodeTouchBuf[13]; ///< Return buffer for digital electrode data
      static bool electrodeOORBuf[15]; ///< Return buffer for out-of-range flags
    #endif
  #else // MPR121_SAVE_MEMORY
    short electrodeDataBuf[13]; ///< Return buffer for analog electrode data
    byte electrodeBaselineBuf[13]; ///< Return buffer for electrode baselines
    byte electrodeCDCBuf[13]; ///< Return buffer for electrode CDC
    mpr121FilterCDT electrodeCDTBuf[13]; ///< Return buffer for electrode CDT
    #if !MPR121_USE_BITFIELDS
      bool electrodeTouchBuf[13]; ///< Return buffer for digital electrode data
      bool electrodeOORBuf[15]; ///< Return buffer for out-of-range flags
    #endif
  #endif // MPR121_SAVE_MEMORY
  
  #if MPR121_USE_BITFIELDS
    short electrodeTouchCache; ///< Cache for digital single electrode reads
  #endif
  unsigned long electrodeTouchCacheMicros; ///< Last update time for electrodeTouchCache (or electrodeTouchBuf if no bitfields)
  
  
  /**
   * Writes a value to an MPR121 register.
   */
  void writeRegister(mpr121Register addr, byte value);

  /**
   * Reads bytes from consecutive MPR121 registers.
   * Max count is equal to the MPR121_I2C_BUFLEN define.
   */
  byte* readRegister(mpr121Register addr, byte count);
  
  /**
   * Reads a byte from an MPR121 register.
   */
  byte readRegister(mpr121Register addr) {
    return readRegister(addr, 1)[0];
  }


  /**
   * Checks if an electrode number and count are valid and suitable for use.
   * Returns true if they can be used or false if the caller should immediately return.
   * This function may modify electrode and/or count to keep them in bounds as necessary
   */
  bool checkElectrodeNum(byte &electrode, byte &count);

  /**
   * Checks if an electrode number is valid and suitable for use.
   * Returns true if it can be used or false if the caller should immediately return.
   * This function may modify electrode to keep it in bounds as necessary
   */
  bool checkElectrodeNum(byte &electrode);

  /**
   * Checks if a GPIO pin number and count are valid and suitable for use.
   * Returns true if they can be used or false if the caller should immediately return.
   * This function may modify pin and/or count to keep them in bounds as necessary
   */
  bool checkGPIOPinNum(byte &pin, byte &count);

  /**
   * Checks if a GPIO pin number is valid and suitable for use.
   * Returns true if it can be used or false if the caller should immediately return.
   * This function may modify pin to keep it in bounds as necessary
   */
  bool checkGPIOPinNum(byte &pin);


  /**
   * Sets the touch and release thresholds for consecutive electrodes.
   * (AN3892)
   */
  void setElectrodeThresholds(byte electrode, byte count, byte touchThreshold, byte releaseThreshold);
  
  /**
   * Sets the touch and release thresholds for a single electrode.
   * (AN3892)
   */
  void setElectrodeThresholds(byte electrode, byte touchThreshold, byte releaseThreshold) {
    setElectrodeThresholds(electrode, 1, touchThreshold, releaseThreshold);
  }


  /**
   * Sets the "Max Half Delta" baseline filter values.
   * Max: 63
   * (AN3891)
   */
  void setMHD(byte rising, byte falling);
  
  /**
   * Sets the "Noise Half Delta" baseline filter values.
   * Max: 63
   * (AN3891)
   */
  void setNHD(byte rising, byte falling, byte touched);
  
  /**
   * Sets the "Noise Count Limit" baseline filter values.
   * (AN3891)
   */
  void setNCL(byte rising, byte falling, byte touched);
  
  /**
   * Sets the "Filter Delay Limit" baseline filter values.
   * (AN3891)
   */
  void setFDL(byte rising, byte falling, byte touched);


  /**
   * Sets the "Max Half Delta" baseline filter values for proximity detection.
   * Max: 63
   * (AN3891/AN3893)
   */
  void setMHDProx(byte rising, byte falling);
  
  /**
   * Sets the "Noise Half Delta" baseline filter values for proximity detection.
   * Max: 63
   * (AN3891/AN3893)
   */
  void setNHDProx(byte rising, byte falling, byte touched);
  
  /**
   * Sets the "Noise Count Limit" baseline filter values for proximity detection.
   * (AN3891/AN3893)
   */
  void setNCLProx(byte rising, byte falling, byte touched);
  
  /**
   * Sets the "Filter Delay Limit" baseline filter values for proximity detection.
   * (AN3891/AN3893)
   */
  void setFDLProx(byte rising, byte falling, byte touched);

  
  /**
   * Sets the debounce counts.
   * A detection must be held for this many readings before the status register is updated.
   * Max: 7
   */
  void setDebounce(byte touchNumber, byte releaseNumber);


  /**
   * Sets "Filter Configuration".
   * (AN3890)
   * 
   * \param FFI   "First Filter Iterations"
   * \param CDC   Global "Charge Discharge Current" (μA), not used if autoconfig is enabled (Max 63, Default 16)
   * \param CDT   Global "Charge Discharge Time" (μs), not used if autoconfig is enabled
   * \param SFI   "Second Filter Iterations"
   * \param ESI   "Electrode Sample Interval" (ms)
   * 
   * Response time is equal to SFI * ESI in ms.
   */
  void setFilterConfig(mpr121FilterFFI FFI, byte CDC, mpr121FilterCDT CDT, mpr121FilterSFI SFI, mpr121FilterESI ESI);

  
  /**
   * Sets "Electrode Configuration".
   * 
   * \param CL          Calibration lock (baseline tracking and initial value settings)
   * \param ELEPROX_EN  Run with these electrodes used for proximity detection
   * \param ELE_EN      Run with this number of electrodes to scan, starting from ELE0 (excludes ELEPROX)
   */
  void setElectrodeConfiguration(mpr121ElectrodeConfigCL CL, mpr121ElectrodeConfigProx ELEPROX_EN, byte ELE_EN);

  
  /**
   * Sets "Auto-Configure" settings.
   * (AN3889/datasheet)
   * 
   * \param USL   "Up-Side Limit" -- Calculate this as `256L * (supplyMillivolts - 700) / supplyMillivolts`. If unsure, use the value for 1.8V supply (156).
   * \param LSL   "Low-Side Limit" -- Calculate this as `USL * 0.65`. If unsure, use the value for 1.8V supply (101).
   * \param TL    "Target Level" -- Calculate this as `USL * 0.9`. If unsure, use the value for 1.8V supply (140).
   * \param RETRY Number of retries for failed config before setting the out of range flag.
   * \param BVA   "Baseline Value Adjust" -- Changes how the baseline registers will be set after auto-configuration completes
   * \param ARE   "Automatic Reconfiguration Enable" -- Out of range (failed) channels will be reconfigured every sampling interval
   * \param ACE   "Automatic Configuration Enable" -- Perform auto-configuration when entering run mode
   * \param SCTS  "Skip Charge Time Search" -- Skip CDT search and use the already-set CDTx or global CDT. This results in a shorter time to configure, but the designer must supply appropriate values.
   * \param OORIE "Out-of-range interrupt enable" -- Trigger an interrupt when a channel is determined to be out of range
   * \param ARFIE "Auto-reconfiguration fail interrupt enable" -- Trigger an interrupt when auto-reconfiguration fails
   * \param ACFIE "Auto-configuration fail interrupt enable" -- Trigger an interrupt when auto-configuration fails
   */
  void setAutoConfig(byte USL, byte LSL, byte TL, mpr121AutoConfigRetry RETRY, mpr121AutoConfigBVA BVA, bool ARE, bool ACE, bool SCTS, bool OORIE, bool ARFIE, bool ACFIE);


  /**
   * Sets the GPIO PWM value for consecutive pins.
   * (AN3894)
   * 
   * Max value is 15
   * Pin 9 apparently has a logic bug and pin 10 must also have its data set high for it to work.
   *   (https://community.nxp.com/thread/305474)
   */
  void setPWM(byte pin, byte count, byte value);
  
public:
  /**
   * Creates an MPR121 device with sane default settings.
   * 
   * \param addr  The I2C address to use. If not specified (or ==0), the next valid address will be chosen automatically.
   * \param wire  You can pass in an alternative TwoWire instance.
   */
  mpr121(byte addr = 0, TwoWire *wire = &Wire);

  byte touchThresholds[13]; ///< Touch detection thresholds for ELE0-ELE11 and ELEPROX
  byte releaseThresholds[13]; ///< Release detection thresholds for ELE0-ELE11 and ELEPROX

  byte MHDrising; ///< "Max Half Delta" rising baseline adjustment value (AN3891) -- max: 63
  byte MHDfalling; ///< "Max Half Delta" falling baseline adjustment value (AN3891) -- max: 63
  
  byte NHDrising; ///< "Noise Half Delta" rising baseline adjustment value (AN3891) -- max: 63
  byte NHDfalling; ///< "Noise Half Delta" falling baseline adjustment value (AN3891) -- max: 63
  byte NHDtouched; ///< "Noise Half Delta" touched baseline adjustment value (AN3891) -- max: 63
  
  byte NCLrising; ///< "Noise Count Limit" rising baseline adjustment value (AN3891)
  byte NCLfalling; ///< "Noise Count Limit" falling baseline adjustment value (AN3891)
  byte NCLtouched; ///< "Noise Count Limit" touched baseline adjustment value (AN3891)
  
  byte FDLrising; ///< "Filter Delay Limit" rising baseline adjustment value (AN3891)
  byte FDLfalling; ///< "Filter Delay Limit" falling baseline adjustment value (AN3891)
  byte FDLtouched; ///< "Filter Delay Limit" touched baseline adjustment value (AN3891)

  
  byte MHDrisingProx; ///< "Max Half Delta" rising value for proximity detection (AN3891/AN3893) -- max: 63
  byte MHDfallingProx; ///< "Max Half Delta" falling value for proximity detection (AN3891/AN3893) -- max: 63
  
  byte NHDrisingProx; ///< "Noise Half Delta" rising value for proximity detection (AN3891/AN3893) -- max: 63
  byte NHDfallingProx; ///< "Noise Half Delta" falling value for proximity detection (AN3891/AN3893) -- max: 63
  byte NHDtouchedProx; ///< "Noise Half Delta" touched value for proximity detection (AN3891/AN3893) -- max: 63
  
  byte NCLrisingProx; ///< "Noise Count Limit" rising value for proximity detection (AN3891/AN3893)
  byte NCLfallingProx; ///< "Noise Count Limit" falling value for proximity detection (AN3891/AN3893)
  byte NCLtouchedProx; ///< "Noise Count Limit" touched value for proximity detection (AN3891/AN3893)
  
  byte FDLrisingProx; ///< "Filter Delay Limit" rising value for proximity detection (AN3891/AN3893)
  byte FDLfallingProx; ///< "Filter Delay Limit" falling value for proximity detection (AN3891/AN3893)
  byte FDLtouchedProx; ///< "Filter Delay Limit" touched value for proximity detection (AN3891/AN3893)


  byte debounceTouch; ///< Set "Debounce" count for touches (times a detection must be sampled) -- max: 7
  byte debounceRelease; ///< Set "Debounce" count for releases (times a detection must be sampled) -- max: 7


  mpr121FilterFFI FFI; ///< "First Filter Iterations" (number of samples taken for the first level of filtering)
  byte globalCDC; ///< Global "Charge Discharge Current" (μA), not used if autoconfig is enabled -- max 63
  mpr121FilterCDT globalCDT; ///< Global "Charge Discharge Time" (μs), not used if autoconfig is enabled
  mpr121FilterSFI SFI; ///< "Second Filter Iterations" (number of samples taken for the second level of filtering)
  mpr121FilterESI ESI; ///< "Electrode Sample Interval" (ms)


  mpr121ElectrodeConfigCL calLock; ///< "Calibration Lock" (baseline tracking and initial value settings)
  mpr121ElectrodeConfigProx proxEnable; ///< ELEPROX_EN: sets what electrodes will be used for proximity detection

  byte autoConfigUSL; ///< "Up-Side Limit" for auto calibration -- if not set when starting, this will be automatically set to the ideal value for 1.8V supply
  byte autoConfigLSL; ///< "Low-Side Limit" for auto calibration -- if not set when starting, this will be automatically set based on USL
  byte autoConfigTL; ///< "Target Level" for auto calibration -- if not set when starting, this will be automatically set based on USL
  mpr121AutoConfigRetry autoConfigRetry; ///< Number of retries for failed auto-config before out of range will be set
  mpr121AutoConfigBVA autoConfigBaselineAdjust; ///< "Baseline Value Adjust" changes how the baseline registers will be set after auto-configuration completes
  bool autoConfigEnableReconfig; ///< "Automatic Reconfiguration Enable" will try to reconfigure out of range (failed) channels every sampling interval
  bool autoConfigEnableCalibration; ///< "Automatic Configuration Enable" will enable/disable auto-configuration when entering run mode
  
  bool autoConfigSkipChargeTime; ///< "Skip Charge Time Search" will skip searching for charge time and use the already-set per-electrode or global value
                                 ///< 
                                 ///< This results in a shorter time to configure, but the designer must supply appropriate values.
  bool autoConfigInterruptOOR; ///< "Out-of-range interrupt enable" will trigger an interrupt when a channel is determined to be out of range
  bool autoConfigInterruptARF; ///< "Auto-reconfiguration fail interrupt enable" will trigger an interrupt when auto-reconfiguration fails
  bool autoConfigInterruptACF; ///< "Auto-configuration fail interrupt enable" will trigger an interrupt when auto-configuration fails


  /** 
   * Reads one touch state bool.
   * Also use this for reading GPIO inputs.
   * 
   * When multiple touches must be read, using the variant that returns data for all 13 is preferred.
   * This does implement a simple cache though, so performance shouldn't be drastically different.
   */
  bool readTouchState(byte electrode);

  #if MPR121_USE_BITFIELDS
    /** 
     * Reads the 13 touch state bits.
     * Also use this for reading GPIO inputs.
     */
    short readTouchState();

    /** Reads the 15 out of range bits.
     * [13]: auto-config fail flag
     * [14]: auto-reconfig fail flag
     */
    short readOORState();
  #else
    /** 
     * Reads the 13 touch state bools.
     * Also use this for reading GPIO inputs.
     */
    bool* readTouchState();

    /** Reads the 15 out of range bools.
     * [13]: auto-config fail flag
     * [14]: auto-reconfig fail flag
     */
    bool* readOORState();
  #endif

  /**
   * Reads the over current flag.
   * (over current on REXT pin, probably shouldn't happen in normal operation)
   */
  bool readOverCurrent();

  /**
   * Clears the over current flag.
   * (over current on REXT pin, probably shouldn't happen in normal operation)
   */
  void clearOverCurrent();

  /**
   * Reads filtered analog data for consecutive electrodes.
   */
  short* readElectrodeData(byte electrode, byte count);
  
  /**
   * Reads filtered analog data for a single electrode.
   */
  short readElectrodeData(byte electrode) {
    return readElectrodeData(electrode, 1)[0];
  }
  
  /**
   * Reads baseline values for consecutive electrodes.
   */
  byte* readElectrodeBaseline(byte electrode, byte count);
  
  /**
   * Reads the baseline value for a single electrode.
   */
  byte readElectrodeBaseline(byte electrode) {
    return readElectrodeBaseline(electrode, 1)[0];
  }

  /**
   * Writes a baseline value to consecutive electrodes.
   */
  void writeElectrodeBaseline(byte electrode, byte count, byte value);
  
  /**
   * Writes the baseline value for a single electrode.
   */
  void writeElectrodeBaseline(byte electrode, byte value) {
    writeElectrodeBaseline(electrode, 1, value);
  }

  /**
   * A quick way to set all ::touchThresholds and ::releaseThresholds.
   * 
   * \param prox  Whether to set proximity detection thresholds too.
   */
  void setAllThresholds(byte touched, byte released, bool prox);
  
  
  /**
   * Reads per-electrode "Charge Discharge Current" (μA) for consecutive electrodes.
   */
  byte* readElectrodeCDC(byte electrode, byte count);
  
  /**
   * Reads per-electrode "Charge Discharge Current" (μA) for a single electrode.
   */
  byte readElectrodeCDC(byte electrode) {
    return readElectrodeCDC(electrode, 1)[0];
  }
  
  /**
   * Writes per-electrode "Charge Discharge Current" (μA) for consecutive electrodes.
   * Max: 63
   */
  void writeElectrodeCDC(byte electrode, byte count, byte value);
  
  /**
   * Writes per-electrode "Charge Discharge Current" (μA) for a single electrode.
   * Max: 63
   */
  void writeElectrodeCDC(byte electrode, byte value) {
    writeElectrodeCDC(electrode, 1, value);
  }
  
  /**
   * Reads per-electrode "Charge Discharge Time" (μs) for consecutive electrodes.
   */
  mpr121FilterCDT* readElectrodeCDT(byte electrode, byte count);
  
  /**
   * Reads per-electrode "Charge Discharge Time" (μs) for a single electrode.
   */
  mpr121FilterCDT readElectrodeCDT(byte electrode) {
    return readElectrodeCDT(electrode, 1)[0];
  }
  
  /**
   * Writes per-electrode "Charge Discharge Time" (μs) for consecutive electrodes.
   */
  void writeElectrodeCDT(byte electrode, byte count, mpr121FilterCDT value);
  
  /**
   * Writes per-electrode "Charge Discharge Time" (μs) for a single electrode.
   */
  void writeElectrodeCDT(byte electrode, mpr121FilterCDT value) {
    writeElectrodeCDT(electrode, 1, value);
  }


  /**
   * Sets pin mode for consecutive GPIO pins.
   * 
   * GPIO can be used on pins 4-11 when they aren't used for sensing.
   * Use mode MPR_GPIO_MODE_OUTPUT_OPENDRAIN_HIGH for direct LED driving -- it can source up to 12mA.
   */
  void setGPIOMode(byte pin, byte count, mpr121GPIOMode mode);

  /**
   * Sets pin mode for a single GPIO pin.
   * 
   * GPIO can be used on pins 4-11 when they aren't used for sensing.
   * Use mode MPR_GPIO_MODE_OUTPUT_OPENDRAIN_HIGH for direct LED driving -- it can source up to 12mA.
   */
  void setGPIOMode(byte pin, mpr121GPIOMode mode) {
    setGPIOMode(pin, 1, mode);
  }

  /**
   * Writes a digital value to consecutive GPIO pins.
   */
  void writeGPIODigital(byte pin, byte count, bool value);

  /**
   * Writes a digital value to a single GPIO pin.
   */
  void writeGPIODigital(byte pin, bool value) {
    writeGPIODigital(pin, 1, value);
  }

  /**
   * Writes an "analog" (PWM) value to consecutive GPIO pins.
   * Max value is 15
   * 
   * Pin 9 apparently has a logic bug and pin 10 must also have its data set high for it to work.
   *   (see https://community.nxp.com/thread/305474)
   */
  void writeGPIOAnalog(byte pin, byte count, byte value);

  /**
   * Writes an "analog" (PWM) value to a single GPIO pin.
   * Max value is 15
   * 
   * Pin 9 apparently has a logic bug and pin 10 must also have its data set high for it to work.
   *   (see https://community.nxp.com/thread/305474)
   */
  void writeGPIOAnalog(byte pin, byte value) {
    writeGPIOAnalog(pin, 1, value);
  }


  /**
   * Optional alternative to using Wire.begin() and Wire.setClock().
   * Also has a built-in delay to ensure MPR121s are ready.
   */
  void begin(unsigned long clock=400000);


  /**
   * Applies settings and enters run mode with a given number of electrodes.
   * Very much based on the quick start guide (AN3944).
   */
  void start(byte electrodes);
  
  #ifndef NO_DOXYGEN
    // (deprecated) alias for start
    DEPRECATED(void startMPR(byte electrodes), "Renamed to `start`.") {
      start(electrodes);
    }
  #endif

  /**
   * Exits run mode.
   */
  void stop();
  
  #ifndef NO_DOXYGEN
    // (deprecated) alias for stop
    DEPRECATED(void stopMPR(), "Renamed to `stop`.") {
      stop();
    }
  #endif


  /**
   * Checks if the MPR121 is in run mode.
   */
  bool checkRunning();


  /**
   * Resets the MPR121.
   */
  void softReset();
};

