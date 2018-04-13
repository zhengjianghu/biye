// Copyright 2014 Silicon Laboratories, Inc.

/** @brief Indicates if a printing area is enabled. */
boolean emberAfPrintEnabled(int16u area);

/** @brief Enables a printing area. */
void emberAfPrintOn(int16u userArea);

/** @brief Disables a printing area. */
void emberAfPrintOff(int16u userArea);

/** @brief Enables all printing areas. */
void emberAfPrintAllOn(void);

/** @brief Disables all printing areas. */
void emberAfPrintAllOff(void);

/** @brief Prints the status of the printing areas. */
void emberAfPrintStatus(void);

/** @brief Prints a formatted message. */
void emberAfPrint(int16u area, PGM_P formatString, ...);

/** @brief Prints a formatted message followed by a newline. */
void emberAfPrintln(int16u area, PGM_P formatString, ...);

/** @brief Prints a buffer as a series of bytes in hexidecimal format. */
void emberAfPrintBuffer(int16u area,
                        const int8u *buffer,
                        int16u bufferLen,
                        boolean withSpace);

/** @brief Print an EUI64 (IEEE address) in big-endian format. */
void emberAfPrintBigEndianEui64(const EmberEUI64 eui64);

/** @brief Print an EUI64 (IEEE address) in little-endian format. */
void emberAfPrintLittleEndianEui64(const EmberEUI64 eui64);

/** @brief Prints a 16-byte ZigBee key. */
void emberAfPrintZigbeeKey(const int8u *key);

/** @brief Waits for all data currently queued to be transmitted. */
void emberAfFlush(int16u area);

extern int16u emberAfPrintActiveArea;
