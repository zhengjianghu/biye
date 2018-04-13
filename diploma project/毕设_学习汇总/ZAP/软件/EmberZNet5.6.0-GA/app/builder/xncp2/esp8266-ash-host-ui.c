/** @file ash-host-ui.c
 *  @brief  ASH Host user interface functions
 *
 *  This includes command option parsing, trace and counter functions.
 * 
 * <!-- Copyright 2009 by Ember Corporation. All rights reserved.       *80*-->
 */

#include PLATFORM_HEADER
#include "stack/include/ember-types.h"
#include "hal/micro/generic/ash-protocol.h"
#include "hal/micro/generic/ash-common.h"
#include "hal/micro/generic/em2xx-reset-defs.h"
#include "hal/micro/system-timer.h"
#include "app/util/ezsp/ezsp-enum-decode.h"
#include "app/ezsp-uart-host/ash-host.h"
#include "app/ezsp-uart-host/ash-host-priv.h"
#include "app/ezsp-uart-host/ash-host-io.h"
#include "app/ezsp-uart-host/ash-host-queues.h"
#include "app/ezsp-uart-host/ash-host-ui.h"

//------------------------------------------------------------------------------
// Local Variables
USE_LOGGER(ASH);


//------------------------------------------------------------------------------
// Error/status code to string conversion

const int8u* ashErrorString(int8u error)
{
  switch (error) {
  case EM2XX_RESET_UNKNOWN:
    return (int8u *) "unknown reset";
  case EM2XX_RESET_EXTERNAL:
    return (int8u *) "external reset";
  case EM2XX_RESET_POWERON:
    return (int8u *) "power on reset";
  case EM2XX_RESET_WATCHDOG:
    return (int8u *) "watchdog reset";
  case EM2XX_RESET_ASSERT:
    return (int8u *) "assert reset";
  case EM2XX_RESET_BOOTLOADER:
    return (int8u *) "bootloader reset";
  case EM2XX_RESET_SOFTWARE:
    return (int8u *) "software reset";
  default:
    return (int8u *) decodeEzspStatus(error);
  }
} // end of ashErrorString()

//------------------------------------------------------------------------------
// Trace output functions

static char * ashPrintFrame(int8u c, char * output, int len)
{
  if ((c & ASH_DFRAME_MASK) == ASH_CONTROL_DATA) {
    if (c & ASH_RFLAG_MASK) {
      snprintf(output, len, "DATA(%d,%d)", ASH_GET_FRMNUM(c), ASH_GET_ACKNUM(c));
    } else {
      snprintf(output, len, "data(%d,%d)", ASH_GET_FRMNUM(c), ASH_GET_ACKNUM(c));
    }
  } else if ((c & ASH_SHFRAME_MASK) == ASH_CONTROL_ACK) {
    if (ASH_GET_NFLAG(c)) {
      snprintf(output, len, "ack(%d)-  ", ASH_GET_ACKNUM(c));
    } else {
      snprintf(output, len, "ack(%d)+  ", ASH_GET_ACKNUM(c));
    }
  } else if ((c & ASH_SHFRAME_MASK) == ASH_CONTROL_NAK) {
    if (ASH_GET_NFLAG(c)) {
      snprintf(output, len, "NAK(%d)-  ", ASH_GET_ACKNUM(c));
    } else {
      snprintf(output, len, "NAK(%d)+  ", ASH_GET_ACKNUM(c));
    }
  } else if (c == ASH_CONTROL_RST) {
    snprintf(output, len, "RST      ");
  } else if (c == ASH_CONTROL_RSTACK) {
    snprintf(output, len, "RSTACK   ");
  } else if (c == ASH_CONTROL_ERROR) {
    snprintf(output, len, "ERROR    ");
  } else {
    snprintf(output, len, "???? 0x%02X", c);
  } 
  return output;
}


#define DATE_MAX 11 //10/10/2000 = 11 characters including NULL
#define TIME_MAX 13 //10:10:10.123 = 13 characters including NULL

//void ashPrintElapsedTime(void)
//{
  // int32u now;
  // now = halCommonGetInt32uMillisecondTick();
  // ashDebugPrintf("%d.%03d ", now/1000, now%1000);
//}

void ashTraceFrame(boolean sent)
{
  int8u flags;
  char output[32];

  flags = ashReadConfig(traceFlags);
  if (flags & (TRACE_FRAMES_BASIC | TRACE_FRAMES_VERBOSE)) {
    if (sent) {
      LogTrace("Tx %s   \n", ashPrintFrame(readTxControl(), output, sizeof(output)));
    } else {
      LogTrace("   %s Rx\n", ashPrintFrame(readRxControl(), output, sizeof(output)));
    }
  }
}

void ashTraceEventRecdFrame(const char *string)
{
  if (ashReadConfig(traceFlags) & TRACE_EVENTS) {
    LogTrace("Rec'd frame: %s\n", string);
  }
}

void ashTraceEventTime(const char *string)
{
  if (ashReadConfig(traceFlags) & TRACE_EVENTS) {
    LogTrace("%s\n", string);
  }
}

void ashTraceDisconnected(int8u error)
{
  if (ashReadConfig(traceFlags) & TRACE_EVENTS) {
    LogTrace("ASH disconnected: %s\n    NCP status: %s\n", ashErrorString(error), ashErrorString(ncpError));
  }
}

void ashTraceEvent(const char *string)
{
  if (ashReadConfig(traceFlags) & TRACE_EVENTS) {
    LogTrace("%s\n", string);
  }
}

void ashTraceArray(const char *name, int8u len, int8u *data)
{
  if (ashReadConfig(traceFlags) & TRACE_EVENTS) {
    //LogHexDump(LOG_TRACE, name, data, len);
  }
}

void ashTraceEzspFrameId(const char *message, int8u *ezspFrame)
{
  if (ashReadConfig(traceFlags) & TRACE_EZSP) {
    int8u frameId = ezspFrame[EZSP_FRAME_ID_INDEX];
    LogTrace("EZSP: %s %s (%02X)\r\n",
                   message, decodeFrameId(frameId), frameId);
  }
}

void ashTraceEzspBuffVerbose(const char *name, int8u *data, int8u len)
{
  if (ashReadConfig(traceFlags) & TRACE_EZSP_VERBOSE) {
    //LogHexDump(LOG_TRACE, name, data, len);
  }
}
