/*
 * QuickMpr121 Arduino library by somewhatlurker
 * =============================================
 * 
 * QuickMpr121 is a library for using MPR121 capacitive touch sensing ICs.
 * More info in QuickMpr121.h, or read the docs.
 * 
 * Copyright 2020 somewhatlurker, MIT license
 */

#include "QuickMpr121.h"

byte mpr121::i2cReadBuf[MPR121_I2C_BUFLEN];

#if MPR121_SAVE_MEMORY
  short mpr121::electrodeDataBuf[13];
  byte mpr121::electrodeBaselineBuf[13];
  byte mpr121::electrodeCDCBuf[13];
  mpr121FilterCDT mpr121::electrodeCDTBuf[13];
  #if !MPR121_USE_BITFIELDS
    bool mpr121::electrodeOORBuf[15];
  #endif
#endif // MPR121_SAVE_MEMORY

// Writes a value to an MPR121 register.
void mpr121::writeRegister(mpr121Register addr, byte value) {
  i2cWire->beginTransmission(i2cAddr);
  i2cWire->write(addr);
  i2cWire->write(value);
  i2cWire->endTransmission();
}

// Reads bytes from consecutive MPR121 registers, starting at addr.
// Max count is equal to the MPR121_I2C_BUFLEN define.
byte* mpr121::readRegister(mpr121Register addr, byte count) {
  if (count > MPR121_I2C_BUFLEN)
    count = MPR121_I2C_BUFLEN;
  
  // write the address to read from
  i2cWire->beginTransmission(i2cAddr);
  i2cWire->write(addr);
  i2cWire->endTransmission(false); // use false to restart instead of stopping
  
  i2cWire->requestFrom(i2cAddr, count, (byte)true); // sendStop is true by default where supported, but setting it guarantees support

  byte readnum = 0;
  while (i2cWire->available() && readnum < count)
  {
    i2cReadBuf[readnum] = i2cWire->read();
    readnum++;
  }

  // if not fully read, clear additional bytes to avoid returning old data
  for ( ; readnum < count; readnum++)
  {
    i2cReadBuf[readnum] = 0;
  }
  
  return i2cReadBuf;
}


// Checks if an electrode number and count are valid and suitable for use.
// Returns true if they can be used or false if the caller should immediately return.
// This function may modify electrode and/or count to keep them in bounds as necessary
bool mpr121::checkElectrodeNum(byte &electrode, byte &count) {
  if (electrode > 12)
    return false;

  if (electrode + count > 13)
    count = 13 - electrode;

  return true;
}

// Checks if an electrode number is valid and suitable for use.
// Returns true if it can be used or false if the caller should immediately return.
// This function may modify electrode to keep it in bounds as necessary
bool mpr121::checkElectrodeNum(byte &electrode) {
  if (electrode > 12)
    return false;

  return true;
}

// Checks if a GPIO pin number and count are valid and suitable for use.
// Returns true if they can be used or false if the caller should immediately return.
// This function may modify pin and/or count to keep them in bounds as necessary
bool mpr121::checkGPIOPinNum(byte &pin, byte &count) {
  if (pin < 4 || pin > 11)
    return false;
  
  if (pin + count > 12)
    count = 12 - pin;
    
  return true;
}

// Checks if a GPIO pin number is valid and suitable for use.
// Returns true if it can be used or false if the caller should immediately return.
// This function may modify pin to keep it in bounds as necessary
bool mpr121::checkGPIOPinNum(byte &pin) {
  if (pin < 4 || pin > 11)
    return false;
    
  return true;
}


// Sets the touch and release thresholds for consecutive electrodes.
// (AN3892)
void mpr121::setElectrodeThresholds(byte electrode, byte count, byte touchThreshold, byte releaseThreshold) {
  if (!checkElectrodeNum(electrode, count))
    return;

  for (byte i = 0; i < count; i++) {
    writeRegister((mpr121Register)(MPRREG_ELE0_TOUCH_THRESHOLD + (i+electrode)*2), touchThreshold);
    writeRegister((mpr121Register)(MPRREG_ELE0_RELEASE_THRESHOLD + (i+electrode)*2), releaseThreshold);
  }
}


// Sets the "Max Half Delta" baseline filter values.
// Max: 63
// (AN3891)
void mpr121::setMHD(byte rising, byte falling) {
  writeRegister(MPRREG_MHD_RISING, rising);
  writeRegister(MPRREG_MHD_FALLING, falling);
}
  
// Sets the "Noise Half Delta" baseline filter values.
// Max: 63
// (AN3891)
void mpr121::setNHD(byte rising, byte falling, byte touched) {
  writeRegister(MPRREG_NHD_AMOUNT_RISING, rising);
  writeRegister(MPRREG_NHD_AMOUNT_FALLING, falling);
  writeRegister(MPRREG_NHD_AMOUNT_TOUCHED, touched);
}

// Sets the "Noise Count Limit" baseline filter values.
// (AN3891)
void mpr121::setNCL(byte rising, byte falling, byte touched) {
  writeRegister(MPRREG_NCL_RISING, rising);
  writeRegister(MPRREG_NCL_FALLING, falling);
  writeRegister(MPRREG_NCL_TOUCHED, touched);
}

// Sets the "Filter Delay Limit" baseline filter values.
// (AN3891)
void mpr121::setFDL(byte rising, byte falling, byte touched) {
  writeRegister(MPRREG_FDL_RISING, rising);
  writeRegister(MPRREG_FDL_FALLING, falling);
  writeRegister(MPRREG_NCL_TOUCHED, touched);
}


// Sets the "Max Half Delta" baseline filter values for proximity detection.
// Max: 63
// (AN3891/AN3893)
void mpr121::setMHDProx(byte rising, byte falling) {
  writeRegister(MPRREG_ELEPROX_MHD_RISING, rising);
  writeRegister(MPRREG_ELEPROX_MHD_FALLING, falling);
}

// Sets the "Noise Half Delta" baseline filter values for proximity detection.
// Max: 63
// (AN3891/AN3893)
void mpr121::setNHDProx(byte rising, byte falling, byte touched) {
  writeRegister(MPRREG_ELEPROX_NHD_AMOUNT_RISING, rising);
  writeRegister(MPRREG_ELEPROX_NHD_AMOUNT_FALLING, falling);
  writeRegister(MPRREG_ELEPROX_NHD_AMOUNT_TOUCHED, touched);
}

// Sets the "Noise Count Limit" baseline filter values for proximity detection.
// (AN3891/AN3893)
void mpr121::setNCLProx(byte rising, byte falling, byte touched) {
  writeRegister(MPRREG_ELEPROX_NCL_RISING, rising);
  writeRegister(MPRREG_ELEPROX_NCL_FALLING, falling);
  writeRegister(MPRREG_ELEPROX_NCL_TOUCHED, touched);
}

// Sets the "Filter Delay Limit" baseline filter values for proximity detection.
// (AN3891/AN3893)
void mpr121::setFDLProx(byte rising, byte falling, byte touched) {
  writeRegister(MPRREG_ELEPROX_FDL_RISING, rising);
  writeRegister(MPRREG_ELEPROX_FDL_FALLING, falling);
  writeRegister(MPRREG_ELEPROX_NCL_TOUCHED, touched);
}


// Sets the debounce counts.
// A detection must be held for this many readings before the status register is updated.
// Max: 7
void mpr121::setDebounce(byte touchNumber, byte releaseNumber) {
  touchNumber &= 0b0111;
  releaseNumber &= 0b0111;

  writeRegister(MPRREG_DEBOUNCE, (touchNumber << 4) | releaseNumber);
}


// Sets "Filter Configuration".
// (AN3890)
// 
// FFI: "First Filter Iterations"
// CDC: Global "Charge Discharge Current" (μA), not used if autoconfig is enabled (Max 63, Default 16)
// CDT: Global "Charge Discharge Time" (μs), not used if autoconfig is enabled
// SFI: "Second Filter Iterations"
// ESI: "Electrode Sample Interval" (ms)
// 
// Response time is equal to SFI * ESI in ms.
void mpr121::setFilterConfig(mpr121FilterFFI FFI, byte CDC, mpr121FilterCDT CDT, mpr121FilterSFI SFI, mpr121FilterESI ESI) {
  byte FFI_2 = (byte)FFI & 0b00000011;
  byte CDC_6 = CDC & 0b00111111;
  byte CDT_3 = (byte)CDT & 0b00000111;
  byte SFI_2 = (byte)SFI & 0b00000011;
  byte ESI_3 = (byte)ESI & 0b00000111;

  writeRegister(MPRREG_FILTER_GLOBAL_CDC_CONFIG, (FFI_2 << 6) | CDC_6);
  writeRegister(MPRREG_FILTER_GLOBAL_CDT_CONFIG, (CDT_3 << 5) | (SFI_2 << 3) | ESI_3);

  // update FFI in autoconf
  byte autoConf = readRegister(MPRREG_AUTOCONFIG_CONTROL_0) & 0b00111111;
  autoConf |= (FFI_2 << 6);
  writeRegister(MPRREG_AUTOCONFIG_CONTROL_0, autoConf);
}


// Sets "Electrode Configuration".
// 
// CL: Calibration lock (baseline tracking and initial value settings)
// ELEPROX_EN: Run with these electrodes used for proximity detection
// ELE_EN: Run with this number of electrodes to scan, starting from ELE0 (excludes ELEPROX)
void mpr121::setElectrodeConfiguration(mpr121ElectrodeConfigCL CL, mpr121ElectrodeConfigProx ELEPROX_EN, byte ELE_EN) {
  byte CL_2 = CL & 0b00000011;
  byte ELEPROX_EN_2 = ELEPROX_EN & 0b00000011;
  byte ELE_EN_4 = ELE_EN & 0b00001111;

  writeRegister(MPRREG_ELECTRODE_CONFIG, (CL_2 << 6) | (ELEPROX_EN_2 << 4) | ELE_EN_4);
}



// Sets "Auto-Configure" settings.
// (AN3889/datasheet)
// 
// USL: "Up-Side Limit" -- Calculate this as `256L * (supplyMillivolts - 700) / supplyMillivolts`. If unsure, use the value for 1.8V supply (156).
// LSL: "Low-Side Limit" -- Calculate this as `USL * 0.65`. If unsure, use the value for 1.8V supply (101).
// TL: "Target Level" -- Calculate this as `USL * 0.9`. If unsure, use the value for 1.8V supply (140).
// RETRY: Number of retries for failed config before setting the out of range flag.
// BVA: "Baseline Value Adjust" -- Changes how the baseline registers will be set after auto-configuration completes
// ARE: "Automatic Reconfiguration Enable" -- Out of range (failed) channels will be reconfigured every sampling interval
// ACE: "Automatic Configuration Enable" -- Perform auto-configuration when entering run mode
// SCTS: "Skip Charge Time Search" -- Skip CDT search and use the already-set CDTx or global CDT. This results in a shorter time to configure, but the designer must supply appropriate values.
// OORIE: "Out-of-range interrupt enable" -- Trigger an interrupt when a channel is determined to be out of range
// ARFIE: "Auto-reconfiguration fail interrupt enable" -- Trigger an interrupt when auto-reconfiguration fails
// ACFIE: "Auto-configuration fail interrupt enable" -- Trigger an interrupt when auto-configuration fails
void mpr121::setAutoConfig(byte USL, byte LSL, byte TL, mpr121AutoConfigRetry RETRY, mpr121AutoConfigBVA BVA, bool ARE, bool ACE, bool SCTS, bool OORIE, bool ARFIE, bool ACFIE) {
  byte FFI = (readRegister(MPRREG_AFE_CONFIG) >> 6) & 0b00000011;
  byte RETRY_2 = RETRY & 0b00000011;
  byte BVA_2 = BVA & 0b00000011;
  
  writeRegister(MPRREG_AUTOCONFIG_USL, USL);
  writeRegister(MPRREG_AUTOCONFIG_LSL, LSL);
  writeRegister(MPRREG_AUTOCONFIG_TL, TL);
  writeRegister(MPRREG_AUTOCONFIG_CONTROL_0, (FFI << 6) | (RETRY_2 << 4) | (BVA_2 << 2) | ((ARE ? 1 : 0) << 1) | (ACE ? 1 : 0));
  writeRegister(MPRREG_AUTOCONFIG_CONTROL_1, ((SCTS ? 1 : 0) << 7) | ((OORIE ? 1 : 0) << 2) | ((ARFIE ? 1 : 0) << 1) | (ACFIE ? 1 : 0));
}


// Sets the GPIO PWM value for consecutive pins.
// (AN3894)
// 
// Max value is 15
// Pin 9 apparently has a logic bug and pin 10 must also have its data set high for it to work.
//   (https://community.nxp.com/thread/305474)
void mpr121::setPWM(byte pin, byte count, byte value) {
  if (!checkGPIOPinNum(pin, count))
    return;

  pin -= 4; // easier to make it 0-indexed now

  byte value_4 = value & 0b1111;
  
  mpr121Register reg = MPRREG_PWM_DUTY_0;
  byte regVal = 0;

  for (byte i = 0; i < count; i++) {
    if (i == 0 || (pin + i) % 2 == 0) { // if just starting or moving to a new register's start
      reg = (mpr121Register)(MPRREG_PWM_DUTY_0 + (pin + i)/2);
      regVal = readRegister(reg);
    }
    
    if ((pin + i) % 2 == 0)
      regVal = (regVal & 0b11110000) | value_4;
    else
      regVal = (regVal & 0b00001111) | (value_4 << 4);

    if ((pin + i) % 2 == 1 || i == count - 1) // if the last of a register or the last iteration
      writeRegister(reg, regVal);
  }
}


// Creates an MPR121 device with sane default settings.
// addr:  The I2C address to use. If not specified (or ==0), the next valid address will be chosen automatically.
//wire:  You can pass in an alternative TwoWire instance.
mpr121::mpr121(byte addr, TwoWire *wire)
{
  static byte usedAddresses = 0;
  
  if (addr == 0) {
    if (!bitRead(usedAddresses, 0)) {
      addr = 0x5a;
    }
    else if (!bitRead(usedAddresses, 1)) {
      addr = 0x5b;
    }
    else if (!bitRead(usedAddresses, 2)) {
      addr = 0x5c;
    }
    else if (!bitRead(usedAddresses, 3)) {
      addr = 0x5d;
    }
    else {
      addr = 0x5a; // at least fall back to *an* mpr121 if all addresses are taken
    }
  }
  
  if (addr >= 0x5a && addr <= 0x5d)
    bitSet(usedAddresses, addr - 0x5a);
  
  i2cAddr = addr;
  i2cWire = wire;

  // values from getting started guide
  // MHDrising = 0x01;
  // MHDfalling = 0x01;
  // NHDrising = 0x01;
  // NHDfalling = 0x01;
  // NHDtouched = 0x00; // ?
  // NCLrising = 0x00;
  // NCLfalling = 0xff;
  // NCLtouched = 0x00; // ?
  // FDLrising = 0x00;
  // FDLfalling = 0x02;
  // FDLtouched = 0x00; // ?

  // values from AN3893
  // MHDrisingProx = 0xff;
  // MHDfallingProx = 0x01;
  // NHDrisingProx = 0xff;
  // NHDfallingProx = 0x01;
  // NHDtouchedProx = 0x00;
  // NCLrisingProx = 0x00;
  // NCLfallingProx = 0xff;
  // NCLtouchedProx = 0x00;
  // FDLrisingProx = 0x00;
  // FDLfallingProx = 0xff;
  // FDLtouchedProx = 0x00;

  // adjusted values
  MHDrising = 0x01;
  MHDfalling = 0x01;
  NHDrising = 0x01;
  NHDfalling = 0x03;
  NHDtouched = 0x00;
  NCLrising = 0x04;
  NCLfalling = 0xc0;
  NCLtouched = 0x00;
  FDLrising = 0x00;
  FDLfalling = 0x02;
  FDLtouched = 0x00;

  // no clue what might be good here lol
  MHDrisingProx = 0x20;
  MHDfallingProx = 0x01;
  NHDrisingProx = 0x10;
  NHDfallingProx = 0x03;
  NHDtouchedProx = 0x00;
  NCLrisingProx = 0x04;
  NCLfallingProx = 0xc0;
  NCLtouchedProx = 0x00;
  FDLrisingProx = 0x00;
  FDLfallingProx = 0x80;
  FDLtouchedProx = 0x00;

  for (byte i = 0; i < 13; i++)
  {
    touchThresholds[i] = 0x0f;
    releaseThresholds[i] = 0x0a;
  }

  debounceTouch = 0x00;
  debounceRelease = 0x00;

  FFI = MPR_FFI_6;
  globalCDC = 16;
  globalCDT = MPR_CDT_0_5;
  SFI = MPR_SFI_4;
  ESI = MPR_ESI_1; // 4 samples, 1ms rate ==> 4ms response time

  calLock = MPR_CL_TRACKING_ENABLED;
  proxEnable = MPR_ELEPROX_DISABLED;

  autoConfigUSL = 0;
  autoConfigLSL = 0;
  autoConfigTL = 0;
  autoConfigRetry = MPR_AUTOCONFIG_RETRY_DISABLED;
  autoConfigBaselineAdjust = MPR_AUTOCONFIG_BVA_SET_CLEAR3;
  autoConfigEnableReconfig = true;
  autoConfigEnableCalibration = true;
  
  autoConfigSkipChargeTime = false;
  autoConfigInterruptOOR = false;
  autoConfigInterruptARF = false;
  autoConfigInterruptACF = false;
}


// Reads one touch state bool.
// Also use this for reading GPIO inputs.
// 
// When multiple touches must be read, using the variant that returns data for all 13 is preferred.
// This should implement some form of caching -- use electrodeTouchCache/electrodeTouchBuf (depending on MPR121_USE_BITFIELDS) and electrodeTouchCacheMicros
bool mpr121::readTouchState(byte electrode) {
  if (!checkElectrodeNum(electrode))
    return false;
  
  unsigned long microsNow = micros();
  if (microsNow > electrodeTouchCacheMicros + 500 || microsNow < electrodeTouchCacheMicros) { // cache for 500 microseconds
    electrodeTouchCacheMicros = microsNow;
    #if MPR121_USE_BITFIELDS
      electrodeTouchCache = readTouchState();
    #else
      // this line actually isn't needed, because the data would already be in electrodeTouchBuf after calling readTouchState()
      // memcpy(electrodeTouchBuf, readTouchState(), sizeof(electrodeTouchBuf));
      readTouchState();
    #endif
  }
  
  #if MPR121_USE_BITFIELDS
    return bitRead(electrodeTouchCache, electrode);
  #else
    return electrodeTouchBuf[electrode];
  #endif
}


#if MPR121_USE_BITFIELDS
  // Reads the 13 touch state bits.
  // Also use this for reading GPIO inputs.
  short mpr121::readTouchState() {
    byte* rawdata = readRegister(MPRREG_ELE0_TO_ELE7_TOUCH_STATUS, 2);
    return rawdata[0] | ((rawdata[1] & 0b00011111) << 8);
  }

  // Reads the 15 out of range bits.
  // [13]: auto-config fail flag
  // [14]: auto-reconfig fail flag
  short mpr121::readOORState() {
    byte* rawdata = readRegister(MPRREG_ELE0_TO_ELE7_OOR_STATUS, 2);
    byte autoConfBits = ((rawdata[1] & 0b10000000) >> 2) | (rawdata[1] & 0b01000000);
    return rawdata[0] | ((rawdata[1] & 0b00011111) << 8) | (autoConfBits << 8);
  }
#else // MPR121_USE_BITFIELDS
  // Reads the 13 touch state bools.
  // Also use this for reading GPIO inputs.
  bool* mpr121::readTouchState() {
    byte* rawdata = readRegister(MPRREG_ELE0_TO_ELE7_TOUCH_STATUS, 2);
    
    electrodeTouchBuf[0] = bitRead(rawdata[0], 0);
    electrodeTouchBuf[1] = bitRead(rawdata[0], 1);
    electrodeTouchBuf[2] = bitRead(rawdata[0], 2);
    electrodeTouchBuf[3] = bitRead(rawdata[0], 3);
    electrodeTouchBuf[4] = bitRead(rawdata[0], 4);
    electrodeTouchBuf[5] = bitRead(rawdata[0], 5);
    electrodeTouchBuf[6] = bitRead(rawdata[0], 6);
    electrodeTouchBuf[7] = bitRead(rawdata[0], 7);
    
    electrodeTouchBuf[8] = bitRead(rawdata[1], 0);
    electrodeTouchBuf[9] = bitRead(rawdata[1], 1);
    electrodeTouchBuf[10] = bitRead(rawdata[1], 2);
    electrodeTouchBuf[11] = bitRead(rawdata[1], 3);
    electrodeTouchBuf[12] = bitRead(rawdata[1], 4);
    
    // electrodeTouchBuf[13] = bitRead(rawdata[1], 7);
  
    return electrodeTouchBuf;
  }

  // Reads the 15 out of range bools.
  // [13]: auto-config fail flag
  // [14]: auto-reconfig fail flag
  bool* mpr121::readOORState() {
    byte* rawdata = readRegister(MPRREG_ELE0_TO_ELE7_OOR_STATUS, 2);
    
    electrodeOORBuf[0] = bitRead(rawdata[0], 0);
    electrodeOORBuf[1] = bitRead(rawdata[0], 1);
    electrodeOORBuf[2] = bitRead(rawdata[0], 2);
    electrodeOORBuf[3] = bitRead(rawdata[0], 3);
    electrodeOORBuf[4] = bitRead(rawdata[0], 4);
    electrodeOORBuf[5] = bitRead(rawdata[0], 5);
    electrodeOORBuf[6] = bitRead(rawdata[0], 6);
    electrodeOORBuf[7] = bitRead(rawdata[0], 7);
    
    electrodeOORBuf[8] = bitRead(rawdata[1], 0);
    electrodeOORBuf[9] = bitRead(rawdata[1], 1);
    electrodeOORBuf[10] = bitRead(rawdata[1], 2);
    electrodeOORBuf[11] = bitRead(rawdata[1], 3);
    electrodeOORBuf[12] = bitRead(rawdata[1], 4);
    
    electrodeOORBuf[13] = bitRead(rawdata[1], 7);
    electrodeOORBuf[14] = bitRead(rawdata[1], 6);
  
    return electrodeOORBuf;
  }
#endif // MPR121_USE_BITFIELDS


// Reads the over current flag.
// (over current on REXT pin, probably shouldn't happen in normal operation)
bool mpr121::readOverCurrent() {
  return bitRead(readRegister(MPRREG_ELE8_TO_ELEPROX_TOUCH_STATUS), 7);
}

// Clears the over current flag.
// (over current on REXT pin, probably shouldn't happen in normal operation)
void mpr121::clearOverCurrent() {
  writeRegister(MPRREG_ELE8_TO_ELEPROX_TOUCH_STATUS, 0b10000000);
}
  

// Reads filtered analog data for consecutive electrodes.
short* mpr121::readElectrodeData(byte electrode, byte count) {
  if (!checkElectrodeNum(electrode, count))
    return electrodeDataBuf;

  byte* rawdata = readRegister((mpr121Register)(MPRREG_ELE0_FILTERED_DATA_LSB + electrode*2), count*2);

  for (byte i = 0; i < count; i++) {
    electrodeDataBuf[electrode + i] = rawdata[i*2] | ((rawdata[i*2 + 1] & 0b00000011) << 8);
  }

  return &electrodeDataBuf[electrode];
}
  
// Reads baseline values for consecutive electrodes.
byte* mpr121::readElectrodeBaseline(byte electrode, byte count) {
  if (!checkElectrodeNum(electrode, count))
    return electrodeBaselineBuf;

  byte* rawdata = readRegister((mpr121Register)(MPRREG_ELE0_BASELINE + electrode), count);

  for (byte i = 0; i < count; i++) {
    electrodeBaselineBuf[electrode + i] = rawdata[i];
  }

  return &electrodeBaselineBuf[electrode];
}

// Write a baseline value to consecutive electrodes.
void mpr121::writeElectrodeBaseline(byte electrode, byte count, byte value) {
  if (!checkElectrodeNum(electrode, count))
    return;

  for (byte i = 0; i < count; i++) {
    writeRegister((mpr121Register)(MPRREG_ELE0_BASELINE + (i+electrode)), value);
  }
}

// A quick way to set all touchThresholds and releaseThresholds.
// prox: Whether to set proximity detection thresholds too.
void mpr121::setAllThresholds(byte touched, byte released, bool prox) {
  byte maxElectrode = 11;
  if (prox)
    maxElectrode = 12;

  for (byte i = 0; i <= maxElectrode; i++) {
    touchThresholds[i] = touched;
    releaseThresholds[i] = released;
  }
}


// Reads per-electrode "Charge Discharge Current" (μA) for consecutive electrodes.
// Max: 63
byte* mpr121::readElectrodeCDC(byte electrode, byte count) {
  if (!checkElectrodeNum(electrode, count))
    return electrodeCDCBuf;

  byte* rawdata = readRegister((mpr121Register)(MPRREG_ELE0_CDC + electrode), count);

  for (byte i = 0; i < count; i++) {
    electrodeCDCBuf[electrode + i] = rawdata[i];
  }

  return &electrodeCDCBuf[electrode];
}

// Writes per-electrode "Charge Discharge Current" (μA) for consecutive electrodes.
// Max: 63
void mpr121::writeElectrodeCDC(byte electrode, byte count, byte value) {
  if (!checkElectrodeNum(electrode, count))
    return;
  
  for (byte i = 0; i < count; i++) {
    writeRegister((mpr121Register)(MPRREG_ELE0_CDC + (electrode + i)), value);
  }
}

// Reads per-electrode "Charge Discharge Time" (μs) for consecutive electrodes.
mpr121FilterCDT* mpr121::readElectrodeCDT(byte electrode, byte count) {
  if (!checkElectrodeNum(electrode, count))
    return electrodeCDTBuf;

  byte* rawdata = readRegister((mpr121Register)(MPRREG_ELE0_ELE1_CDT + electrode/2), (electrode % 2 == 0 ? (count+1) / 2 : (count+2) / 2));

  for (byte i = 0; i < count; i++) {
    mpr121FilterCDT value;
    if (electrode % 2 == 0)
      value = (mpr121FilterCDT)(( rawdata[i / 2] >> (i % 2 == 0 ? 0 : 4) ) & 0b111);
    else
      value = (mpr121FilterCDT)(( rawdata[(i+1) / 2] >> (i % 2 == 0 ? 4 : 0) ) & 0b111);
    
    electrodeCDTBuf[electrode + i] = value;
  }

  return &electrodeCDTBuf[electrode];
}

// Writes per-electrode "Charge Discharge Time" (μs) for consecutive electrodes.
void mpr121::writeElectrodeCDT(byte electrode, byte count, mpr121FilterCDT value) {
  if (!checkElectrodeNum(electrode, count))
    return;
  
  byte value_3 = value & 0b111;
  
  mpr121Register reg = MPRREG_ELE0_ELE1_CDT;
  byte regVal = 0;

  for (byte i = 0; i < count; i++) {
    if (i == 0 || (electrode + i) % 2 == 0) { // if just starting or moving to a new register's start
      reg = (mpr121Register)(MPRREG_ELE0_ELE1_CDT + (electrode + i)/2);
      regVal = readRegister(reg);
    }
    
    if ((electrode + i) % 2 == 0)
      regVal = (regVal & 0b11110000) | value_3;
    else
      regVal = (regVal & 0b00001111) | (value_3 << 4);

    if ((electrode + i) % 2 == 1 || i == count - 1) // if the last of a register or the last iteration
      writeRegister(reg, regVal);
  }
}


// Sets pin mode for consecutive GPIO pins.
// GPIO can be used on pins 4-11 when they aren't used for sensing.
// Use mode MPR_GPIO_MODE_OUTPUT_OPENDRAIN_HIGH for direct LED driving -- it can source up to 12mA.
void mpr121::setGPIOMode(byte pin, byte count, mpr121GPIOMode mode) {
  if (!checkGPIOPinNum(pin, count))
    return;

  pin -= 4; // easier to make it 0-indexed now

  
  byte enableByte = readRegister(MPRREG_GPIO_ENABLE);

  // disable the modified outputs while changing stuff around
  for (byte i = 0; i < count; i++) {
    bitClear(enableByte, pin + i);
  }
  writeRegister(MPRREG_GPIO_ENABLE, enableByte);

  if (mode == MPR_GPIO_MODE_DISABLED)
    return; // all done, no need to worry about other values


  byte tempByte;
  bool tempVal;

  // set direction
  tempByte = readRegister(MPRREG_GPIO_DIRECTION);
  tempVal = bitRead(mode, 2); // settings are bit-packed into the enum to save doing lookups
  for (byte i = 0; i < count; i++) {
    bitWrite(tempByte, pin + i, tempVal);
  }
  writeRegister(MPRREG_GPIO_DIRECTION, tempByte);

  // set control 0 (enable internal resistors or open drain mode)
  tempByte = readRegister(MPRREG_GPIO_CONTROL_0);
  tempVal = bitRead(mode, 1);
  for (byte i = 0; i < count; i++) {
    bitWrite(tempByte, pin + i, tempVal);
  }
  writeRegister(MPRREG_GPIO_CONTROL_0, tempByte);

  // set control 1 (internal resistor or open drain level)
  tempByte = readRegister(MPRREG_GPIO_CONTROL_1);
  tempVal = bitRead(mode, 0);
  for (byte i = 0; i < count; i++) {
    bitWrite(tempByte, pin + i, tempVal);
  }
  writeRegister(MPRREG_GPIO_CONTROL_1, tempByte);


  // set enable to final value
  tempVal = bitRead(mode, 3);
  for (byte i = 0; i < count; i++) {
    bitWrite(enableByte, pin + i, tempVal);
  }
  writeRegister(MPRREG_GPIO_ENABLE, enableByte);
}


// Writes a digital value to consecutive GPIO pins.
void mpr121::writeGPIODigital(byte pin, byte count, bool value) {
  if (!checkGPIOPinNum(pin, count))
    return;

  // disable PWM for affected pins
  // (doesn't use 0-indexed GPIO pin number)
  setPWM(pin, count, 0);

  pin -= 4; // easier to make it 0-indexed now

  
  mpr121Register reg = value ? MPRREG_GPIO_DATA_SET : MPRREG_GPIO_DATA_CLEAR;

  byte tempByte = 0;
  for (byte i = 0; i < count; i++) {
    bitSet(tempByte, pin + i);
  }
  writeRegister(reg, tempByte);
}


// Writes an "analog" (PWM) value to consecutive GPIO pins.
// Max value is 15
// Pin 9 apparently has a logic bug and pin 10 must also have its data set high for it to work.
//   (see https://community.nxp.com/thread/305474)
void mpr121::writeGPIOAnalog(byte pin, byte count, byte value) {
  if (!checkGPIOPinNum(pin, count))
    return;

  // set PWM for affected pins
  // (doesn't use 0-indexed GPIO pin number)
  setPWM(pin, count, value);

  pin -= 4; // easier to make it 0-indexed now


  mpr121Register reg = value == 0 ? MPRREG_GPIO_DATA_CLEAR : MPRREG_GPIO_DATA_SET;

  byte tempByte = 0;
  for (byte i = 0; i < count; i++) {
    bitSet(tempByte, pin + i);
  }
  writeRegister(reg, tempByte);
}


// Optional alternative to using Wire.begin() and Wire.setClock().
// Also has a built-in delay to ensure MPR121s are ready.
void mpr121::begin(unsigned long clock) {
  while (millis() < 5)
    delay(1);

  i2cWire->end(); // apparently some platforms have issues with double starts, I guess this fixes it
  i2cWire->begin();
  i2cWire->setClock(clock);
}


// Applies settings and enters run mode with a given number of electrodes.
// Very much based on the quick start guide (AN3944).
void mpr121::start(byte electrodes) {
  stop();
  
  // restrict value of numeric properties with < 8 bits to actual sent values
  MHDrising &= 0b00111111;
  MHDfalling &= 0b00111111;
  NHDrising &= 0b00111111;
  NHDfalling &= 0b00111111;
  NHDtouched &= 0b00111111;
  MHDrisingProx &= 0b00111111;
  MHDfallingProx &= 0b00111111;
  NHDrisingProx &= 0b00111111;
  NHDfallingProx &= 0b00111111;
  NHDtouchedProx &= 0b00111111;
  debounceTouch &= 0b0111;
  debounceRelease &= 0b0111;
  globalCDC &= 0b00111111;

  // set/calculate autoconfig values
  if (autoConfigUSL == 0)
    autoConfigUSL = 156;
  if (autoConfigLSL == 0)
    autoConfigLSL = autoConfigUSL * 0.65;
  if (autoConfigTL == 0)
    autoConfigTL = autoConfigUSL * 0.9;

  setMHD(MHDrising, MHDfalling);
  setNHD(NHDrising, NHDfalling, NHDtouched);
  setNCL(NCLrising, NCLfalling, NCLtouched);
  setFDL(FDLrising, FDLfalling, FDLtouched);
  
  setMHDProx(MHDrisingProx, MHDfallingProx);
  setNHDProx(NHDrisingProx, NHDfallingProx, NHDtouchedProx);
  setNCLProx(NCLrisingProx, NCLfallingProx, NCLtouchedProx);
  setFDLProx(FDLrisingProx, FDLfallingProx, FDLtouchedProx);
  
  for (byte i = 0; i < 13; i++)
  {
    mpr121::setElectrodeThresholds(i, touchThresholds[i], releaseThresholds[i]);
  }

  setDebounce(debounceTouch, debounceRelease);

  setFilterConfig(FFI, globalCDC, globalCDT, SFI, ESI);

  setAutoConfig(autoConfigUSL, autoConfigLSL, autoConfigTL, autoConfigRetry, autoConfigBaselineAdjust, autoConfigEnableReconfig, autoConfigEnableCalibration,
                autoConfigSkipChargeTime, autoConfigInterruptOOR, autoConfigInterruptARF, autoConfigInterruptACF);

  // OVCF blocks starting, so reset it 
  if (readOverCurrent())
    clearOverCurrent();
  
  setElectrodeConfiguration(calLock, proxEnable, electrodes);
}

// Exits run mode.
void mpr121::stop() {
  byte oldConfig = readRegister(MPRREG_ELECTRODE_CONFIG);
  writeRegister(MPRREG_ELECTRODE_CONFIG, oldConfig & 0b11000000);
}


// Checks if the MPR121 is in run mode.
bool mpr121::checkRunning() {
  return (readRegister(MPRREG_ELECTRODE_CONFIG) & 0b00111111) != 0;
}


// Resets the MPR121.
void mpr121::softReset() {
  writeRegister(MPRREG_SOFT_RESET, 0x63);
}

