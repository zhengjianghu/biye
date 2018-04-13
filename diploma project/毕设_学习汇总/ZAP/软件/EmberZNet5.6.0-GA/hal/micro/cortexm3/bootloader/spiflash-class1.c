/*
 * File: spiflash-class1.c
 * Description: SPIFlash driver that supports a variety of SPI flash parts
 *   Including: Winbond W25X20BV (2Mbit), W25Q80BV (8Mbit)
 *              Macronix MX25L2006E (2Mbit), MX25L8006E (8Mbit)
 *                       MX25L1606E (16Mbit), MX25U1635E (16Mbit 2Volt)
 *              Atmel/Adesto AT25DF041A (4Mbit), AT25DF081A (8Mbit)
 *              Numonyx/Micron M25P20 (2Mbit), M25P40 (4Mbit),
 *                             M25P80 (8Mbit), M25P16 (16Mbit)
 *   It could be easily extended to support other parts that talk the
 *    same protocol as these.
 *
 *   This driver does not support any write protection functionality
 *
 * Copyright 2013 Silicon Laboratories, Inc.                                *80*
 *
 */

/*
 * When SPI_FLASH_SC1 is defined, serial controller one will be used to
 * communicate with the external flash.
 * - This must be enabled in the case of app bootloading over USB, since USB
 *   uses SC2. Also ensure that the board jumpers are in the right place and
 *   the debugger is not connected to any SC1 pins.
 * - This must be enabled in the case of PRO2+-connected devices since SC2
 *   is used to interact with the PRO2+ radio.
 */

#include PLATFORM_HEADER
#include "hal/micro/bootloader-eeprom.h"
#include "bootloader-common.h"
#include "bootloader-serial.h"
#include "hal/micro/micro.h"
#include BOARD_HEADER
#include "hal/micro/cortexm3/memmap.h"

// abstract away which pins and serial controller are being used
#include "external-flash-gpio-select.h"

#define THIS_DRIVER_VERSION (0x0109)

#if BAT_VERSION != THIS_DRIVER_VERSION
  #error External Flash Driver must be updated to support new API requirements
#endif

/* Debug
#define LEDON()  GPIO_PACLR = BIT(6)
#define LEDOFF() GPIO_PASET = BIT(6)
*/

//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~ Generic SPI Routines ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

// WARNING! hardware FIFO is only 4 bytes, so don't push more than 4 before popping!
static void halSpiPush8(int8u txData)
{
  while( (EXTERNAL_FLASH_SCx_SPISTAT&SC_SPITXFREE) != SC_SPITXFREE ) {
    // wait for space available
  }
  EXTERNAL_FLASH_SCx_DATA = txData;
}

static int8u halSpiPop8(void)
{
  // WARNING!  spiPop8 must be matched 1:1 with spiPush8 calls made first
  //  or else this could spin forever!!
  while( (EXTERNAL_FLASH_SCx_SPISTAT&SC_SPIRXVAL) != SC_SPIRXVAL ) {
    // wait for byte to be avail
  }
  return EXTERNAL_FLASH_SCx_DATA;
}

static void halSpiPushN8(int8u n) {
  while(n--) {
    halSpiPush8(0xFF);
  }
}

static void halSpiPopN8(int8u n) {
  while(n--) {
    halSpiPop8();
  }
}

/* These spi funcs are not currently used
static void halSpiPush16MSB(int16u txData)
{
  halSpiPush8((txData >> 8) & 0xFF);
  halSpiPush8(txData & 0xFF);
}
*/

static void halSpiPush24MSB(int32u txData)
{
  halSpiPush8((txData >> 16) & 0xFF);
  halSpiPush8((txData >> 8) & 0xFF);
  halSpiPush8(txData & 0xFF);
}

/* These spi funcs are not currently used
static void halSpiPush32MSB(int32u txData)
{
  halSpiPush16MSB((txData >> 16) & 0xFFFF);
  halSpiPush16MSB(txData & 0xFFFF);
}
*/

static int16u halSpiPop16MSB(void)
{
  int16u result;
  result = ((int16u)halSpiPop8()) << 8;
  result |= ((int16u)halSpiPop8());
  return result;
}

/* These spi funcs are not currently used
static int32u halSpiPop24MSB(void)
{
  int32u result;
  result = ((int32u)halSpiPop8()) << 16;
  result |= ((int32u)halSpiPop8()) << 8;
  result |= ((int32u)halSpiPop8());
  return result;
}

static int32u halSpiPop32MSB(void)
{
  int32u result;
  result = ((int32u)halSpiPop16MSB()) << 16;
  result |= ((int32u)halSpiPop16MSB());
  return result;
}
*/

static void halSpiWrite8(int8u txData)
{
  halSpiPush8(txData);
  halSpiPop8();
}

/* These spi funcs are not currently used
static void halSpiWrite16MSB(int16u txData)
{
  halSpiPush16MSB(txData);
  halSpiPopN8(2);
}

static void halSpiWrite24MSB(int32u txData)
{
  halSpiPush24MSB(txData);
  halSpiPopN8(3);
}

static void halSpiWrite32MSB(int32u txData)
{
  halSpiPush32MSB(txData);
  halSpiPopN8(4);
}
*/

static int8u halSpiRead8(void)
{
  halSpiPush8(0xFF);
  return halSpiPop8();
}

/* These spi funcs are not currently used
static int16u halSpiRead16MSB(void)
{
  halSpiPushN8(2);
  return halSpiPop16MSB();
}

static int32u halSpiRead24MSB(void)
{
  halSpiPushN8(3);
  return halSpiPop24MSB();
}

static int32u halSpiRead32MSB(void)
{
  halSpiPushN8(4);
  return halSpiPop32MSB();
}
*/

//
// ~~~~~~~~~~~~~~~~~~~~~~~~ Device Specific Interface ~~~~~~~~~~~~~~~~~~~~~~~~~~
//

// Note: This driver does not support block write protection features

#define DEVICE_SIZE_2M                      (262144L)
#define DEVICE_SIZE_4M                      (524288L)
#define DEVICE_SIZE_8M                      (1048576L)
#define DEVICE_SIZE_16M                     (2097152L)

// Pages are the write buffer granularity
#define DEVICE_PAGE_SIZE                    (256)
#define DEVICE_PAGE_MASK                    (255)
// Sectors are the erase granularity
// *except* for Numonyx parts which only support BLOCK erase granularity
#define DEVICE_SECTOR_SIZE                  (4096)
#define DEVICE_SECTOR_MASK                  (4095)
// Blocks define a larger erase granularity
#define DEVICE_BLOCK_SIZE                   (65536L)
#define DEVICE_BLOCK_MASK                   (65535L)
// The flash word size in bytes
#define DEVICE_WORD_SIZE                    (1)

// JEDEC Manufacturer IDs
#define MFG_ID_WINBOND                      (0xEF)
#define MFG_ID_MACRONIX                     (0xC2)
#define MFG_ID_ATMEL                        (0x1F)
#define MFG_ID_NUMONYX                      (0x20)

// JEDEC Device IDs
#define DEVICE_ID_WINBOND_2M                (0x3012)
#define DEVICE_ID_WINBOND_4M                (0x3013)
#define DEVICE_ID_WINBOND_8M                (0x4014)
#define DEVICE_ID_MACRONIX_2M               (0x2012)
#define DEVICE_ID_MACRONIX_8M               (0x2014)
#define DEVICE_ID_MACRONIX_16M              (0x2015)
#define DEVICE_ID_MACRONIX_16M_2V           (0x2535)
#define DEVICE_ID_ATMEL_4M                  (0x4401)
#define DEVICE_ID_ATMEL_8M                  (0x4501)
#define DEVICE_ID_NUMONYX_2M                (0x2012)
#define DEVICE_ID_NUMONYX_4M                (0x2013)
#define DEVICE_ID_NUMONYX_8M                (0x2014)
#define DEVICE_ID_NUMONYX_16M               (0x2015)

// Protocol commands
#define CMD_WRITE_ENABLE                    (0x06)
#define CMD_WRITE_DISABLE                   (0x04)
#define CMD_READ_STATUS                     (0x05)
#define CMD_WRITE_STATUS                    (0x01)
#define CMD_READ_DATA                       (0x03)
#define CMD_PAGE_PROG                       (0x02)
#define CMD_ERASE_4K                        (0x20)
#define CMD_ERASE_64K                       (0xD8)
#define CMD_ERASE_CHIP                      (0xC7)
#define CMD_POWER_DOWN                      (0xB9)
#define CMD_POWER_UP                        (0xAB)
#define CMD_JEDEC_ID                        (0x9F)
#define CMD_UNIQUE_ID                       (0x4B)

// Bitmasks for status register fields
#define STATUS_BUSY_MASK                    (0x01)
#define STATUS_WEL_MASK                     (0x02)

// These timings represent the worst case out of all chips supported by this
//  driver.  Some chips may perform faster.
// (in general Winbond is faster than Macronix is faster than Numonyx)
#define TIMING_POWERON_MAX_US               (10000)
#define TIMING_SLEEP_MAX_US                 (10)
#define TIMING_WAKEUP_MAX_US                (30)
#define TIMING_PROG_MAX_US                  (5000)
#define TIMING_WRITE_STATUS_MAX_US          (40000)
// (MS units are 1024Hz based)
#define TIMING_ERASE_4K_MAX_MS              (410)
#define TIMING_ERASE_64K_MAX_MS             (3072)
#define TIMING_ERASE_WINBOND_2M_MAX_MS      (2048)
#define TIMING_ERASE_WINBOND_4M_MAX_MS      (4096)
#define TIMING_ERASE_WINBOND_8M_MAX_MS      (6144)
#define TIMING_ERASE_MACRONIX_2M_MAX_MS     (3892)
#define TIMING_ERASE_MACRONIX_8M_MAX_MS     (15360)
#define TIMING_ERASE_MACRONIX_16M_MAX_MS    (30720)
#define TIMING_ERASE_MACRONIX_16M_2V_MAX_MS (20480)
#define TIMING_ERASE_ATMEL_4M_MAX_MS        (7168)
#define TIMING_ERASE_ATMEL_8M_MAX_MS        (28672)
#define TIMING_ERASE_NUMONYX_2M_MAX_MS      (6144)
#define TIMING_ERASE_NUMONYX_4M_MAX_MS      (10240)
#define TIMING_ERASE_NUMONYX_8M_MAX_MS      (20480)
#define TIMING_ERASE_NUMONYX_16M_MAX_MS     (40960)


static const HalEepromInformationType windbond2MInfo = {
  EEPROM_INFO_VERSION,
  EEPROM_CAPABILITIES_ERASE_SUPPORTED | EEPROM_CAPABILITIES_PAGE_ERASE_REQD,
  TIMING_ERASE_4K_MAX_MS,
  TIMING_ERASE_WINBOND_2M_MAX_MS,
  DEVICE_SECTOR_SIZE,
  DEVICE_SIZE_2M,
  "W25X20BV",
  DEVICE_WORD_SIZE // word size in bytes
};

static const HalEepromInformationType windbond8MInfo = {
  EEPROM_INFO_VERSION,
  EEPROM_CAPABILITIES_ERASE_SUPPORTED | EEPROM_CAPABILITIES_PAGE_ERASE_REQD,
  TIMING_ERASE_4K_MAX_MS,
  TIMING_ERASE_WINBOND_8M_MAX_MS,
  DEVICE_SECTOR_SIZE,
  DEVICE_SIZE_8M,
  "W25Q80BV",
  DEVICE_WORD_SIZE // word size in bytes
};

static const HalEepromInformationType macronix2MInfo = {
  EEPROM_INFO_VERSION,
  EEPROM_CAPABILITIES_ERASE_SUPPORTED | EEPROM_CAPABILITIES_PAGE_ERASE_REQD,
  TIMING_ERASE_4K_MAX_MS,
  TIMING_ERASE_MACRONIX_2M_MAX_MS,
  DEVICE_SECTOR_SIZE,
  DEVICE_SIZE_2M,
  "MX25L2006E",
  DEVICE_WORD_SIZE // word size in bytes
};

static const HalEepromInformationType macronix8MInfo = {
  EEPROM_INFO_VERSION,
  EEPROM_CAPABILITIES_ERASE_SUPPORTED | EEPROM_CAPABILITIES_PAGE_ERASE_REQD,
  TIMING_ERASE_4K_MAX_MS,
  TIMING_ERASE_MACRONIX_8M_MAX_MS,
  DEVICE_SECTOR_SIZE,
  DEVICE_SIZE_8M,
  "MX25L8006E",
  DEVICE_WORD_SIZE // word size in bytes
};

static const HalEepromInformationType macronix16MInfo = {
  EEPROM_INFO_VERSION,
  EEPROM_CAPABILITIES_ERASE_SUPPORTED | EEPROM_CAPABILITIES_PAGE_ERASE_REQD,
  TIMING_ERASE_4K_MAX_MS,
  TIMING_ERASE_MACRONIX_16M_MAX_MS,
  DEVICE_SECTOR_SIZE,
  DEVICE_SIZE_16M,
  "MX25L1606E",
  DEVICE_WORD_SIZE // word size in bytes
};

static const HalEepromInformationType macronix16M2VInfo = {
  EEPROM_INFO_VERSION,
  EEPROM_CAPABILITIES_ERASE_SUPPORTED | EEPROM_CAPABILITIES_PAGE_ERASE_REQD,
  TIMING_ERASE_4K_MAX_MS,
  TIMING_ERASE_MACRONIX_16M_2V_MAX_MS,
  DEVICE_SECTOR_SIZE,
  DEVICE_SIZE_16M,
  "MX25U1635E",
  DEVICE_WORD_SIZE // word size in bytes
};

static const HalEepromInformationType atmel4MInfo = {
  EEPROM_INFO_VERSION,
  EEPROM_CAPABILITIES_ERASE_SUPPORTED | EEPROM_CAPABILITIES_PAGE_ERASE_REQD,
  TIMING_ERASE_4K_MAX_MS,
  TIMING_ERASE_ATMEL_4M_MAX_MS,
  DEVICE_SECTOR_SIZE,
  DEVICE_SIZE_4M,
  "AT25DF041A",
  DEVICE_WORD_SIZE // word size in bytes
};

static const HalEepromInformationType atmel8MInfo = {
  EEPROM_INFO_VERSION,
  EEPROM_CAPABILITIES_ERASE_SUPPORTED | EEPROM_CAPABILITIES_PAGE_ERASE_REQD,
  TIMING_ERASE_4K_MAX_MS,
  TIMING_ERASE_ATMEL_8M_MAX_MS,
  DEVICE_SECTOR_SIZE,
  DEVICE_SIZE_8M,
  "AT25DF081A",
  DEVICE_WORD_SIZE // word size in bytes
};

static const HalEepromInformationType numonyx2MInfo = {
  EEPROM_INFO_VERSION,
  EEPROM_CAPABILITIES_ERASE_SUPPORTED | EEPROM_CAPABILITIES_PAGE_ERASE_REQD,
  TIMING_ERASE_64K_MAX_MS,
  TIMING_ERASE_NUMONYX_2M_MAX_MS,
  DEVICE_BLOCK_SIZE, // Numonyx does not support smaller sector erase
  DEVICE_SIZE_2M,
  "M25P20",
  DEVICE_WORD_SIZE // word size in bytes
};

static const HalEepromInformationType numonyx4MInfo = {
  EEPROM_INFO_VERSION,
  EEPROM_CAPABILITIES_ERASE_SUPPORTED | EEPROM_CAPABILITIES_PAGE_ERASE_REQD,
  TIMING_ERASE_64K_MAX_MS,
  TIMING_ERASE_NUMONYX_4M_MAX_MS,
  DEVICE_BLOCK_SIZE, // Numonyx does not support smaller sector erase
  DEVICE_SIZE_4M,
  "M25P40",
  DEVICE_WORD_SIZE // word size in bytes
};

static const HalEepromInformationType numonyx8MInfo = {
  EEPROM_INFO_VERSION,
  EEPROM_CAPABILITIES_ERASE_SUPPORTED | EEPROM_CAPABILITIES_PAGE_ERASE_REQD,
  TIMING_ERASE_64K_MAX_MS,
  TIMING_ERASE_NUMONYX_8M_MAX_MS,
  DEVICE_BLOCK_SIZE, // Numonyx does not support smaller sector erase
  DEVICE_SIZE_8M,
  "M25P80",
  DEVICE_WORD_SIZE // word size in bytes
};

static const HalEepromInformationType numonyx16MInfo = {
  EEPROM_INFO_VERSION,
  EEPROM_CAPABILITIES_ERASE_SUPPORTED | EEPROM_CAPABILITIES_PAGE_ERASE_REQD,
  TIMING_ERASE_64K_MAX_MS,
  TIMING_ERASE_NUMONYX_16M_MAX_MS,
  DEVICE_BLOCK_SIZE, // Numonyx does not support smaller sector erase
  DEVICE_SIZE_16M,
  "M25P16",
  DEVICE_WORD_SIZE // word size in bytes
};


typedef enum {
  UNKNOWN_DEVICE,
  WINBOND_2M_DEVICE,
  WINBOND_8M_DEVICE,
  MACRONIX_2M_DEVICE,
  MACRONIX_8M_DEVICE,
  MACRONIX_16M_DEVICE,
  MACRONIX_16M_2V_DEVICE,
  ATMEL_4M_DEVICE,
  ATMEL_8M_DEVICE,
  //N.B. If add more ATMEL_ devices, update halEepromInit() accordingly
  NUMONYX_2M_DEVICE,
  NUMONYX_4M_DEVICE,
  NUMONYX_8M_DEVICE,
  NUMONYX_16M_DEVICE,
  //N.B. If add more NUMONYX_ devices, update halEepromErase() accordingly
} DeviceType;

// Initialization constants.  For more detail on the resulting waveforms,
// see the EM35x datasheet.
#define SPI_ORD_MSB_FIRST (0<<SC_SPIORD_BIT) // Send the MSB first
#define SPI_ORD_LSB_FIRST (1<<SC_SPIORD_BIT) // Send the LSB first

#define SPI_PHA_FIRST_EDGE (0<<SC_SPIPHA_BIT)  // Sample on first edge
#define SPI_PHA_SECOND_EDGE (1<<SC_SPIPHA_BIT) // Sample on second edge

#define SPI_POL_RISING_LEAD  (0<<SC_SPIPOL_BIT) // Leading edge is rising
#define SPI_POL_FALLING_LEAD (1<<SC_SPIPOL_BIT) // Leading edge is falling

#if    !defined(EXTERNAL_FLASH_RATE_LINEAR)       \
    || !defined(EXTERNAL_FLASH_RATE_EXPONENTIAL)

  #if    defined(EXTERNAL_FLASH_RATE_LINEAR)      \
      || defined(EXTERNAL_FLASH_RATE_EXPONENTIAL)

    #error Partial Flash serial rate definition. Please define both \
           EXTERNAL_FLASH_RATE_LINEAR and EXTERNAL_FLASH_RATE_EXPONENTIAL when \
           specifying a custom rate.

  #endif

  // configure for fastest allowable rate
  // rate = 12 MHz / ((LIN + 1) * (2^EXP))
  #if defined(CORTEXM3_EM350) || defined(CORTEXM3_STM32W108)
    #define EXTERNAL_FLASH_RATE_LINEAR  (1)     // limited to 6MHz
  #else
    #define EXTERNAL_FLASH_RATE_LINEAR  (0)     // 12Mhz - FOR EM35x
  #endif

  #define EXTERNAL_FLASH_RATE_EXPONENTIAL  (0)

#endif

#if EXTERNAL_FLASH_RATE_LINEAR < 0 || EXTERNAL_FLASH_RATE_LINEAR > 15
  #error EXTERNAL_FLASH_RATE_LINEAR must be between 0 and 15 (inclusive)
#endif

#if EXTERNAL_FLASH_RATE_EXPONENTIAL < 0 || EXTERNAL_FLASH_RATE_EXPONENTIAL > 15
  #error EXTERNAL_FLASH_RATE_EXPONENTIAL must be between 0 and 15 (inclusive)
#endif

static void waitNotBusy(void)
{
  while(halEepromBusy()) {
    // Do nothing
  }
}


static DeviceType getDeviceType(void)
{
  int8u mfgId;
  int16u deviceId;

  // cannot check for busy in this API since it is used by
  //  init.  Callers must verify not busy individually.
  EXTERNAL_FLASH_CS_ACTIVE();
  halSpiPush8(CMD_JEDEC_ID);
  halSpiPushN8(3);
  halSpiPop8();
  mfgId = halSpiPop8();
  deviceId = halSpiPop16MSB();
  EXTERNAL_FLASH_CS_INACTIVE();

  switch(mfgId) {
    case MFG_ID_WINBOND:
      switch(deviceId) {
        case DEVICE_ID_WINBOND_2M:
          return WINBOND_2M_DEVICE;
        case DEVICE_ID_WINBOND_8M:
          return WINBOND_8M_DEVICE;
        default:
          return UNKNOWN_DEVICE;
      }
    case MFG_ID_MACRONIX:
      switch(deviceId) {
        case DEVICE_ID_MACRONIX_2M:
          return MACRONIX_2M_DEVICE;
        case DEVICE_ID_MACRONIX_8M:
          return MACRONIX_8M_DEVICE;
        case DEVICE_ID_MACRONIX_16M:
          return MACRONIX_16M_DEVICE;
        case DEVICE_ID_MACRONIX_16M_2V:
          return MACRONIX_16M_2V_DEVICE;
        default:
          return UNKNOWN_DEVICE;
      }
    case MFG_ID_ATMEL:
      switch(deviceId)
      {
        case DEVICE_ID_ATMEL_4M:
          return ATMEL_4M_DEVICE;
        case DEVICE_ID_ATMEL_8M:
          return ATMEL_8M_DEVICE;
        default:
          return UNKNOWN_DEVICE;
      }
    case MFG_ID_NUMONYX:
      switch(deviceId) {
        case DEVICE_ID_NUMONYX_2M:
          return NUMONYX_2M_DEVICE;
        case DEVICE_ID_NUMONYX_4M:
          return NUMONYX_4M_DEVICE;
        case DEVICE_ID_NUMONYX_8M:
          return NUMONYX_8M_DEVICE;
        case DEVICE_ID_NUMONYX_16M:
          return NUMONYX_16M_DEVICE;
        default:
          return UNKNOWN_DEVICE;
      }
    default:
      return UNKNOWN_DEVICE;
  }
}


static void setWEL(void)
{
  EXTERNAL_FLASH_CS_ACTIVE();
  halSpiWrite8(CMD_WRITE_ENABLE);
  EXTERNAL_FLASH_CS_INACTIVE();
}


int8u halEepromInit(void)
{
  DeviceType deviceType;

  //-----SPI Master GPIO configuration
  halGpioSetConfig(EXTERNAL_FLASH_MOSI_PIN, GPIOCFG_OUT_ALT);
  halGpioSetConfig(EXTERNAL_FLASH_MISO_PIN, GPIOCFG_IN);
  halGpioSetConfig(EXTERNAL_FLASH_SCLK_PIN, GPIOCFG_OUT_ALT);
  halGpioSetConfig(EXTERNAL_FLASH_nCS_PIN , GPIOCFG_OUT);

  //-----SPI Master SCx configuration
  EXTERNAL_FLASH_SCx_RATELIN = EXTERNAL_FLASH_RATE_LINEAR;
  EXTERNAL_FLASH_SCx_RATEEXP = EXTERNAL_FLASH_RATE_EXPONENTIAL;
  EXTERNAL_FLASH_SCx_SPICFG  = 0;
  EXTERNAL_FLASH_SCx_SPICFG  = (1 << SC_SPIMST_BIT)|  // 4; master control bit
                          (SPI_ORD_MSB_FIRST | SPI_PHA_FIRST_EDGE | SPI_POL_RISING_LEAD);
  EXTERNAL_FLASH_SCx_MODE    = EXTERNAL_FLASH_SCx_MODE_SPI;

  // Set EEPROM_POWER_PIN high as part of EEPROM init
  CONFIGURE_EEPROM_POWER_PIN();
  SET_EEPROM_POWER_PIN();

  // Ensure the device is ready to access after applying power
  // We delay even if shutdown control isn't used to play it safe
  // since we don't know how quickly init may be called after boot
  halCommonDelayMicroseconds(TIMING_POWERON_MAX_US);

  // Release the chip from powerdown mode
  EXTERNAL_FLASH_CS_ACTIVE();
  halSpiWrite8(CMD_POWER_UP);
  EXTERNAL_FLASH_CS_INACTIVE();
  halCommonDelayMicroseconds(TIMING_WAKEUP_MAX_US);

  deviceType = getDeviceType();
  if(deviceType == UNKNOWN_DEVICE) {
    return EEPROM_ERR_INVALID_CHIP;
  }
  // For Atmel devices, need to unprotect them because default is protected
  if(deviceType >= ATMEL_4M_DEVICE && deviceType <= ATMEL_8M_DEVICE) {
    setWEL();
    EXTERNAL_FLASH_CS_ACTIVE();
    halSpiWrite8(CMD_WRITE_STATUS);
    halSpiWrite8(0); // No protect bits set
    EXTERNAL_FLASH_CS_INACTIVE();
  }
  return EEPROM_SUCCESS;
}


void halEepromShutdown(void)
{
  // wait for any outstanding operations to complete before pulling the plug
  waitNotBusy();

  // always enter low power mode, even if using shutdown control
  //  since sometimes leakage prevents shutdown control from
  //  completely turning off the part.
  EXTERNAL_FLASH_CS_ACTIVE();
  halSpiWrite8(CMD_POWER_DOWN);
  EXTERNAL_FLASH_CS_INACTIVE();

  // pull power if using shutdown control
  CLR_EEPROM_POWER_PIN();
}


const HalEepromInformationType *halEepromInfo(void)
{
  waitNotBusy();
  switch(getDeviceType()) {
    case WINBOND_2M_DEVICE:
      return &windbond2MInfo;
    case WINBOND_8M_DEVICE:
      return &windbond8MInfo;
    case MACRONIX_2M_DEVICE:
      return &macronix2MInfo;
    case MACRONIX_8M_DEVICE:
      return &macronix8MInfo;
    case MACRONIX_16M_DEVICE:
      return &macronix16MInfo;
    case MACRONIX_16M_2V_DEVICE:
      return &macronix16M2VInfo;
    case ATMEL_4M_DEVICE:
      return &atmel4MInfo;
    case ATMEL_8M_DEVICE:
      return &atmel8MInfo;
    case NUMONYX_2M_DEVICE:
      return &numonyx2MInfo;
    case NUMONYX_4M_DEVICE:
      return &numonyx4MInfo;
    case NUMONYX_8M_DEVICE:
      return &numonyx8MInfo;
    case NUMONYX_16M_DEVICE:
      return &numonyx16MInfo;
    default:
      return NULL;
  }
}

int32u halEepromSize(void)
{
  return halEepromInfo()->partSize;
}

boolean halEepromBusy(void)
{
  int8u status;

  EXTERNAL_FLASH_CS_ACTIVE();
  halSpiPush8(CMD_READ_STATUS);
  halSpiPush8(0xFF);
  halSpiPop8();
  status = halSpiPop8();
  EXTERNAL_FLASH_CS_INACTIVE();
  if( (status & STATUS_BUSY_MASK) == STATUS_BUSY_MASK )
    return TRUE;
  else
    return FALSE;
}


static int32u getDeviceSize(DeviceType *pDeviceType)
{
  DeviceType deviceType;
  waitNotBusy();
  if(pDeviceType == NULL) {
    deviceType = getDeviceType();
  } else {
    deviceType = *pDeviceType;
    if(deviceType == UNKNOWN_DEVICE) {
      deviceType = getDeviceType();
      *pDeviceType = deviceType;
    }
  }
  switch(deviceType) {
    case WINBOND_2M_DEVICE:
    case NUMONYX_2M_DEVICE:
    case MACRONIX_2M_DEVICE:
      return DEVICE_SIZE_2M;
    case ATMEL_4M_DEVICE:
    case NUMONYX_4M_DEVICE:
      return DEVICE_SIZE_4M;
    case WINBOND_8M_DEVICE:
    case ATMEL_8M_DEVICE:
    case MACRONIX_8M_DEVICE:
    case NUMONYX_8M_DEVICE:
      return DEVICE_SIZE_8M;
    case MACRONIX_16M_DEVICE:
    case MACRONIX_16M_2V_DEVICE:
    case NUMONYX_16M_DEVICE:
      return DEVICE_SIZE_16M;
    default:
      return 0;
  }
}

static boolean verifyAddressRange(int32u address, int16u length,
                                  DeviceType *pDeviceType)
{
  // all parts support addresses less than DEVICE_SIZE_2M
  if( (address + length) <= DEVICE_SIZE_2M )
    return TRUE;

  // if address is greater than DEVICE_SIZE_2M, need to query the chip
  if( (address + length) <= getDeviceSize(pDeviceType) )
    return TRUE;

  // out of range
  return FALSE;
}


int8u halEepromRead(int32u address, int8u *data, int16u totalLength)
{
  if( !verifyAddressRange(address, totalLength, NULL) )
    return EEPROM_ERR_ADDR;

  waitNotBusy();

  EXTERNAL_FLASH_CS_ACTIVE();
  halSpiPush8(CMD_READ_DATA);
  halSpiPush24MSB(address);
  halSpiPopN8(4);

  while(totalLength--) {
    *data++ = halSpiRead8();
  }
  EXTERNAL_FLASH_CS_INACTIVE();

  return EEPROM_SUCCESS;
}


static void writePage(int32u address, const int8u *data, int16u len)
{
  waitNotBusy();
  setWEL();

  EXTERNAL_FLASH_CS_ACTIVE();
  halSpiPush8(CMD_PAGE_PROG);
  halSpiPush24MSB(address);
  halSpiPopN8(4);

  while(len--) {
    halSpiWrite8(*data++);
  }
  EXTERNAL_FLASH_CS_INACTIVE();
}


static boolean verifyErased(int32u address, int16u len)
{
  waitNotBusy();

  EXTERNAL_FLASH_CS_ACTIVE();
  halSpiPush8(CMD_READ_DATA);
  halSpiPush24MSB(address);
  halSpiPopN8(4);

  while(len--) {
    if(halSpiRead8() != 0xFF) {
      return FALSE;
    }
  }
  EXTERNAL_FLASH_CS_INACTIVE();
  return TRUE;
}


int8u halEepromWrite(int32u address, const int8u *data, int16u totalLength)
{
  int32u nextPageAddr;
  int16u len;

  if( !verifyAddressRange(address, totalLength, NULL) )
    return EEPROM_ERR_ADDR;

  if(!verifyErased(address, totalLength)) {
    return EEPROM_ERR_ERASE_REQUIRED;
  }

  if( address & DEVICE_PAGE_MASK) {
    // handle unaligned first block
    nextPageAddr = (address & (~DEVICE_PAGE_MASK)) + DEVICE_PAGE_SIZE;
    if((address + totalLength) < nextPageAddr){
      // fits all within first block
      len = totalLength;
    } else {
      len = (int16u) (nextPageAddr - address);
    }
  } else {
    len = (totalLength>DEVICE_PAGE_SIZE)? DEVICE_PAGE_SIZE : totalLength;
  }
  while(totalLength) {
    writePage(address, data, len);
    totalLength -= len;
    address += len;
    data += len;
    len = (totalLength>DEVICE_PAGE_SIZE)? DEVICE_PAGE_SIZE : totalLength;
  }

  return EEPROM_SUCCESS;
}


static void doEraseCmd(int8u command, int32u address)
{
  waitNotBusy();
  setWEL();
  EXTERNAL_FLASH_CS_ACTIVE();
  halSpiPush8(command);
  halSpiPush24MSB(address);
  halSpiPopN8(4);
  EXTERNAL_FLASH_CS_INACTIVE();
}


int8u halEepromErase(int32u address, int32u totalLength)
{
  DeviceType deviceType = UNKNOWN_DEVICE;
  int32u sectorMask = DEVICE_SECTOR_MASK;
  int32u deviceSize = getDeviceSize(&deviceType);
  // Numonyx/Micron parts only support block erase, not sector
  if( (deviceType >= NUMONYX_2M_DEVICE)
    &&(deviceType <= NUMONYX_16M_DEVICE) ) {
    sectorMask = DEVICE_BLOCK_MASK;
  }
  // Length must be a multiple of the sector size
  if( totalLength & sectorMask )
    return EEPROM_ERR_PG_SZ;
  // Address must be sector aligned
  if( address & sectorMask )
    return EEPROM_ERR_PG_BOUNDARY;
  // Address and length must be in range
  if( !verifyAddressRange(address, totalLength, &deviceType) )
    return EEPROM_ERR_ADDR;

  // Test for full chip erase
  if( (address == 0) && (totalLength == deviceSize) ) {
    waitNotBusy();
    setWEL();
    EXTERNAL_FLASH_CS_ACTIVE();
    halSpiWrite8(CMD_ERASE_CHIP);
    EXTERNAL_FLASH_CS_INACTIVE();
    return EEPROM_SUCCESS;
  }

  // first handle leading partial blocks
  while(totalLength && (address & DEVICE_BLOCK_MASK)) {
    doEraseCmd(CMD_ERASE_4K, address);
    address += DEVICE_SECTOR_SIZE;
    totalLength -= DEVICE_SECTOR_SIZE;
  }
  // handle any full blocks
  while(totalLength >= DEVICE_BLOCK_SIZE) {
    doEraseCmd(CMD_ERASE_64K, address);
    address += DEVICE_BLOCK_SIZE;
    totalLength -= DEVICE_BLOCK_SIZE;
  }
  // finally handle any trailing partial blocks
  while(totalLength) {
    doEraseCmd(CMD_ERASE_4K, address);
    address += DEVICE_SECTOR_SIZE;
    totalLength -= DEVICE_SECTOR_SIZE;
  }
  return EEPROM_SUCCESS;
}
