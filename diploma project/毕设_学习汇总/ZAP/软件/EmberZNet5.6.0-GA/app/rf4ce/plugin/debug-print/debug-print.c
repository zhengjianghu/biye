// Copyright 2014 Silicon Laboratories, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_EMBER_TYPES
#include EMBER_AF_API_HAL
#include EMBER_AF_API_SERIAL
#include "debug-print.h"

// When EMBER_AF_DEBUG_PRINT_USE_PORT is defined, the underlying serial code
// functions require a port, so one is passed in.
#ifdef EMBER_AF_DEBUG_PRINT_USE_PORT
  #define emAfWaitSend()            emberSerialWaitSend(APP_SERIAL)
  #define emAfPrintf(...)           emberSerialPrintf(APP_SERIAL, __VA_ARGS__)
  #define emAfPrintfLine(...)       emberSerialPrintfLine(APP_SERIAL, __VA_ARGS__)
  #define emAfPrintCarriageReturn() emberSerialPrintCarriageReturn(APP_SERIAL)
  #define emAfPrintfVarArg(...)     emberSerialPrintfVarArg(APP_SERIAL, __VA_ARGS__)
#else
  #define emAfWaitSend()            emberSerialWaitSend()
  #define emAfPrintf(...)           emberSerialPrintf(__VA_ARGS__)
  #define emAfPrintfLine(...)       emberSerialPrintfLine(__VA_ARGS__)
  #define emAfPrintCarriageReturn() emberSerialPrintCarriageReturn()
  #define emAfPrintfVarArg(...)     emberSerialPrintfVarArg(__VA_ARGS__)
#endif

// A internal printing area is a 16-bit value.  The high byte is an index and
// the low byte is a bitmask.  The index is used to look up a byte and the
// bitmask is used to check if a single bit in that byte is set.  If it is set,
// the area is enabled.  Otherwise, the area is diabled.
#define AREA_INDEX(area)    HIGH_BYTE(area)
#define AREA_BITMASK(area)  LOW_BYTE(area)

// Areas can be enabled or disabled at runtime.  This is not done using the
// internal area, but with a user area, which is simply an offset into a zero-
// indexed array of areas.  The idea is that area names are printed like this:
//   [0] Core : YES
//   [1] Debug : no
//   [2] Applicaion : YES
//   ...
// If the user wanted to turn on the "Debug" area, he would call
// emberAfPrintOn(1), presumably via a CLI command.  The internal area can be
// reconstructed from the user area through bit magic.
#define USER_AREA_TO_INTERNAL_AREA(userArea) \
  HIGH_LOW_TO_INT(userArea / 8, BIT(userArea % 8))

    int16u emberAfPrintActiveArea = 0;

#ifdef EMBER_AF_PRINT_BITS
  static int8u bitmasks[] = EMBER_AF_PRINT_BITS;

  #define ENABLE(userArea) \
    printEnable(USER_AREA_TO_INTERNAL_AREA(userArea), TRUE);
  #define DISABLE(userArea) \
    printEnable(USER_AREA_TO_INTERNAL_AREA(userArea), FALSE);
  #define ENABLE_ALL() MEMSET(bitmasks, 0xFF, sizeof(bitmasks))
  #define DISABLE_ALL() MEMSET(bitmasks, 0x00, sizeof(bitmasks))

  static void printEnable(int16u area, boolean on)
  {
    int8u index = AREA_INDEX(area);
    if (index < sizeof(bitmasks)) {
      int8u bitmask = AREA_BITMASK(area);
      if (on) {
        SETBITS(bitmasks[index], bitmask);
      } else {
        CLEARBITS(bitmasks[index], bitmask);
      }
    }
  }
#else
  #define ENABLE(userArea)
  #define DISABLE(userArea)
  #define ENABLE_ALL()
  #define DISABLE_ALL()
#endif

#ifdef EMBER_AF_PRINT_NAMES
  static PGM_P names[] = EMBER_AF_PRINT_NAMES;
#endif

boolean emberAfPrintEnabled(int16u area)
{
  emberAfPrintActiveArea = area;
  if (area == 0xFFFF) {
    return TRUE;
#ifdef EMBER_AF_PRINT_BITS
  } else {
    int8u index = AREA_INDEX(area);
    if (index < sizeof(bitmasks)) {
      int8u bitmask = AREA_BITMASK(area);
      return READBITS(bitmasks[index], bitmask);
    }
#endif
  }
  return FALSE;
}

void emberAfPrintOn(int16u userArea)
{
  ENABLE(userArea);
}

void emberAfPrintOff(int16u userArea)
{
  DISABLE(userArea);
}

void emberAfPrintAllOn(void)
{
  ENABLE_ALL();
}

void emberAfPrintAllOff(void)
{
  DISABLE_ALL();
}

void emberAfPrintStatus(void)
{
#ifdef EMBER_AF_PRINT_NAMES
  int8u i;
  for (i = 0; i < EMBER_AF_PRINT_NAME_NUMBER; i++) {
    emAfPrintfLine("[%d] %p : %p",
                   i,
                   names[i],
                   (emberAfPrintEnabled(USER_AREA_TO_INTERNAL_AREA(i))
                    ? "YES"
                    : "no"));
    emAfWaitSend();
  }
#endif
}

#if defined(EMBER_AF_PRINT_AREA_NAME) && defined(EMBER_AF_PRINT_NAMES)
  // If the area is bogus, this may still print a name, but it shouldn't crash.
  static void printAreaName(int16u area)
  {
    int16u index;
    int8u bitmask = AREA_BITMASK(area);
    int8u bit;

    for (bit = 0; bit < 8; bit++) {
      if (READBIT(bitmask, bit)) {
        break;
      }
    }
    index = AREA_INDEX(area) * 8 + bit;

    if (area != 0xFFFF
        && index < EMBER_AF_PRINT_NAME_NUMBER) {
      emAfPrintf("%p:", names[index]);
    }
  }
#else
  #define printAreaName(area)
#endif

static void printVarArg(int16u area,
                        boolean newline,
                        PGM_P formatString,
                        va_list args) {
  if (emberAfPrintEnabled(area)) {
    printAreaName(area);
    emAfPrintfVarArg(formatString, args);
    if (newline) {
      emAfPrintCarriageReturn();
    }
  }
}

void emberAfPrint(int16u area, PGM_P formatString, ...)
{
  va_list args;
  va_start(args, formatString);
  printVarArg(area, FALSE, formatString, args);
  va_end(args);
}

void emberAfPrintln(int16u area, PGM_P formatString, ...)
{
  va_list args;
  va_start(args, formatString);
  printVarArg(area, TRUE, formatString, args);
  va_end(args);
}

static void printBuffer(int16u area,
                        const int8u *buffer,
                        int16u bufferLen,
                        PGM_P formatString)
{
  if (emberAfPrintEnabled(area)) {
    int16u index;
    for (index = 0; index < bufferLen; index++) {
      emberAfPrint(area, formatString, buffer[index]);
      if (index % 16 == 6) {
        emberAfFlush(area);
      }
    }
  }
}

void emberAfPrintBuffer(int16u area,
                        const int8u *buffer,
                        int16u bufferLen,
                        boolean withSpace)
{
  printBuffer(area, buffer, bufferLen, (withSpace ? "%x " : "%x"));
}

void emberAfPrintBigEndianEui64(const EmberEUI64 eui64)
{
  emberAfPrint(emberAfPrintActiveArea,
               "(%c)%x%x%x%x%x%x%x%x",
               '>',
               eui64[7],
               eui64[6],
               eui64[5],
               eui64[4],
               eui64[3],
               eui64[2],
               eui64[1],
               eui64[0]);
}

void emberAfPrintLittleEndianEui64(const EmberEUI64 eui64)
{
  emberAfPrint(emberAfPrintActiveArea,
               "(%c)%x%x%x%x%x%x%x%x",
               '<',
               eui64[0],
               eui64[1],
               eui64[2],
               eui64[3],
               eui64[4],
               eui64[5],
               eui64[6],
               eui64[7]);
}

void emberAfPrintZigbeeKey(const int8u *key)
{
  // ZigBee keys are 16 bytes.
  emberAfPrintBuffer(emberAfPrintActiveArea, key, 8, TRUE);
  emberAfPrint(emberAfPrintActiveArea, " ");
  emberAfPrintBuffer(emberAfPrintActiveArea, key + 8, 8, TRUE);
  emberAfPrintln(emberAfPrintActiveArea, "");
}

void emberAfFlush(int16u area)
{
  if (emberAfPrintEnabled(area)) {
    emAfWaitSend();
  }
}
