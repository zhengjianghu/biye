/*
 * File: em260-common.c
 * Description: Public common code for NCP applications.
 *
 * Author(s): Maurizio Nanni, maurizio.nanni@ember.com
 *
 * Copyright 2012 by Ember Corporation. All rights reserved.
 */
#include PLATFORM_HEADER

#include "stack/include/ember.h"

#include "hal/hal.h"

#include "app/util/ezsp/ezsp-enum.h"
#include "app/util/ezsp/ezsp-frame-utilities.h"

#include "app/util/source-route-common.h"

int8u sourceRouteTableSize = EMBER_SOURCE_ROUTE_TABLE_SIZE;
SourceRouteTableEntry *sourceRouteTable;

//------------------------------------------------------------------------------
// GPIO configuration.

#ifdef CORTEXM3

static void setOutReg(int32u volatile * outReg, int8u portPin, int8u out)
{
  *outReg = (*outReg & ~(1<<(portPin%8))) | out<<(portPin%8);
}

static void setCfgReg(int32u volatile * cfgReg, int8u portPin, int8u cfg)
{
  *cfgReg = (*cfgReg & ~(0xF<<((portPin%4)*4))) | cfg<<((portPin%4)*4);
}

void emProcessSetGpioCurrentConfiguration(void)
{
  int8u portPin = fetchInt8u();
  int8u cfg = fetchInt8u();
  int8u out = fetchInt8u();

  //Ensure the desired cfg and out register values fit within those field sizes.
  cfg &= 0xF;
  out &= 0x1;

  //PC7 is the last pin that exists.  Throw an error if portPin is invalid.
  if(portPin > PORTC_PIN(7)) {
    appendInt8u(EZSP_ERROR_INVALID_VALUE);
    return;
  }

  //If portPin is found in EZSP_PIN_BLACKOUT_MASK (which is defined in
  //the board header) it is critical to host-NCP communications and must
  //not be modified.
  if(EZSP_PIN_BLACKOUT_MASK & (((GpioMaskType)1)<<portPin)) {
    appendInt8u(EZSP_ERROR_INVALID_CALL);
    return;
  }

  switch(portPin/8) {
  case 0:
    setOutReg(&GPIO_PAOUT, portPin, out);
    break;
  case 1:
    setOutReg(&GPIO_PBOUT, portPin, out);
    break;
  case 2:
    setOutReg(&GPIO_PCOUT, portPin, out);
    break;
  };

  switch(portPin/4) {
  case 0:
    setCfgReg(&GPIO_PACFGL, portPin, cfg);
    break;
  case 1:
    setCfgReg(&GPIO_PACFGH, portPin, cfg);
    break;
  case 2:
    setCfgReg(&GPIO_PBCFGL, portPin, cfg);
    break;
  case 3:
    setCfgReg(&GPIO_PBCFGH, portPin, cfg);
    break;
  case 4:
    setCfgReg(&GPIO_PCCFGL, portPin, cfg);
    break;
  case 5:
    setCfgReg(&GPIO_PCCFGH, portPin, cfg);
    break;
  };

  appendInt8u(EZSP_SUCCESS);
}

static void setCfgValue(int16u * cfgVar, int8u portPin, int8u cfg)
{
  *cfgVar = (*cfgVar & ~(0xF<<((portPin%4)*4))) | cfg<<((portPin%4)*4);
}

static void setOutValue(int8u * outVar, int8u portPin, int8u out)
{
  *outVar = (*outVar & ~(1<<(portPin%8))) | out<<(portPin%8);
}

void emProcessSetGpioPowerUpDownConfiguration(void)
{
  int8u portPin = fetchInt8u();
  int8u puCfg = fetchInt8u();
  int8u puOut = fetchInt8u();
  int8u pdCfg = fetchInt8u();
  int8u pdOut = fetchInt8u();

  //Ensure the desired cfg and out register values fit within those field sizes.
  puCfg &= 0xF;
  puOut &= 0x1;
  pdCfg &= 0xF;
  pdOut &= 0x1;

  //PC7 is the last pin that exists.  Throw an error if portPin is invalid.
  if(portPin > PORTC_PIN(7)) {
    appendInt8u(EZSP_ERROR_INVALID_VALUE);
    return;
  }

  //If portPin is found in EZSP_PIN_BLACKOUT_MASK (which is defined in
  //the board header) it is critical to host-NCP communications and must
  //not be modified.
  if(EZSP_PIN_BLACKOUT_MASK & (((GpioMaskType)1)<<portPin)) {
    appendInt8u(EZSP_ERROR_INVALID_CALL);
    return;
  }

  setCfgValue(&gpioCfgPowerUp[portPin/4], portPin, puCfg);
  setCfgValue(&gpioCfgPowerDown[portPin/4], portPin, pdCfg);

  setOutValue(&gpioOutPowerUp[portPin/8], portPin, puOut);
  setOutValue(&gpioOutPowerDown[portPin/8], portPin, pdOut);

  appendInt8u(EZSP_SUCCESS);
}

void emProcessSetGpioRadioPowerMask(void)
{
  int32u newMask = fetchInt32u();

  //Any pin defined in EZSP_PIN_BLACKOUT_MASK is critical to host-NCP
  //communications and must not be controlled by halStackRadioPower*Board
  newMask &= ~EZSP_PIN_BLACKOUT_MASK;

  //Just to be safe, mask off bits for pins that don't exist.
  gpioRadioPowerBoardMask = ((1<<(PORTC_PIN(7)+1))-1) & newMask;
}

#else

void emProcessSetGpioCurrentConfiguration(void)
{
  appendInt8u(EZSP_ERROR_INVALID_CALL);
}

void emProcessSetGpioPowerUpDownConfiguration(void)
{
  appendInt8u(EZSP_ERROR_INVALID_CALL);
}

void emProcessSetGpioRadioPowerMask(void)
{
  appendInt8u(EZSP_ERROR_INVALID_CALL);
}

#endif

// *****************************************
// Convenience Stubs
// *****************************************

#ifndef __NCP_CONFIG__

#ifndef EMBER_APPLICATION_HAS_SET_OR_GET_EZSP_TOKEN_HANDLER
void emberSetOrGetEzspTokenCommandHandler(boolean isSet)
{
  appendInt8u(EMBER_INVALID_CALL);
}
#endif

#endif /* __NCP_CONFIG__ */
