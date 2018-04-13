/**
 * File: hal/micro/generic/system-timer.c
 * Description: simulation files for the system timer part of the HAL
 *
 * Copyright 2008 by Ember Corporation. All rights reserved.
 */


#include PLATFORM_HEADER


int16u halCommonGetInt16uMillisecondTick(void)
{
  return (int16u)halCommonGetInt32uMillisecondTick();
}
