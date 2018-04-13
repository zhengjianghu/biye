// Copyright 2014 Silicon Laboratories, Inc.
//-----------------------------------------------------------------------------
/**
 * @addtogroup full-featured-target
 *
 * The full featured target implements communication of action codes and audio
 * from the controller to the host computer and action mappings and controller
 * identification from the host computer to the controller.
 * This plugin implements the communication protocol between the RF4CE target
 * and the host computer and implements the functions that are required for
 * both the communication with the controller and host.
 *
 * One important feature of the communication protocol between the target
 * and host is that it has built in framing that defines a unique start
 * and stop for each package that is transferred. This ensures that the
 * communication between the target and host will get into synch if data
 * is lost. And it does not require and use a request - acknowledge scheme
 * that may be too slow for transferring live audio data.
 *
 * This plugin implements the two top layers of the OSI model, the
 * Application and Presentation layers. And it uses the already available
 * serial function calls that implements transport over the Physical layer.
 *
 * Attrobutes of the Presentation layer:
 * Each message begins with the byte 0xC0 and ends with the byte 0xC1.
 * In between the framing bytes are the payload and the checksum.
 * The last byte of a message (without the framing) contains an 8-bit checksum.
 * The checksum is calculated over the entire message, without framing and
 * without including the checksum byte. The checksum is obtained by XORing
 * one and one byte.
 * To make sure that 0xC0 and 0xC1 are not contained within the payload an
 * escape sequence is used. The value 0x7E is used to XOR the following byte
 * with the value 0x20. This is used for the following three combinations:
 * 0x7E 0x5E is used for 0x7E
 * 0x7E 0xE0 is used for 0xC0
 * 0x7E 0xE1 is used for 0xC1
 *
 * Attributes of the Application layer:
 * The application layer consists of an Application Header and Application Data.
 * The Application Header is mandatory. Some messages may consist of an
 * application header only. The Application Data is the payload of the
 * application layer message.
 * @{
 */

/**
 * @brief The initialization function.
 *
 * @param port The host communication port.
 **/
void    emberAfTargetCommunicationInit(int8u port);

/**
 * @brief Send an action to the host computer.
 *
 * @param port The host communication port.
 * @param type The action type.
 * @param modifier The action modifier.
 * @param bank The action bank.
 * @param code The action code.
 * @param vendor The action vendor.
 **/
void    emberAfTargetCommunicationHostActionTx(int8u port,
                                               int8u type,
                                               int8u modifier,
                                               int8u bank,
                                               int8u code,
                                               int16u vendor);

/**
 * @brief Send audio data to the host computer.
 *
 * @param port The host communication port.
 * @param pBuf Pointer to the buffer containing audion data.
 * @param uLen The length of the audion data in bytes.
 **/
void    emberAfTargetCommunicationHostAudioTx(int8u port,
                                              const int8u *pBuf,
                                              int8u uLen);

/**
 * @brief Send an identification message to the controller.
 *
 * @param pairingIndex The pairing index to send the identification to.
 * @param flags The zrc identification flags for the identification.
 * @param seconds The length of the identification.
 **/
void    emberAfTargetCommunicationControllerIdentify(int8u pairingIndex,
                                                     int8u flags,
                                                     int8u seconds);

/**
 * @brief Send a notification message to the controller
 *
 * @param pairingIndex The pairing index to send the notification to.
 **/
void    emberAfTargetCommunicationControllerNotify(int8u uPairingIndex);

/**
 * @brief Set an action mapping in the mapping server.
 *
 * @param deviceType The device type.
 * @param bank The bank.
 * @param action The action code.
 * @param mappingFlags Mapping flags.
 * @param rfDat A pointer to the rf action mapping.
 * @param rfLen The length of the rf action mapping.
 * @param irDat A pointer to the ir action mapping.
 * @param irLen The length of the ir action mapping.
 **/
void    emberAfTargetCommunicationControllerRemapAction(int8u deviceType,
                                                        int8u bank,
                                                        int8u action,
                                                        int8u mappingFlags,
                                                        int8u *rfDat,
                                                        int8u rfLen,
                                                        int8u *irDat,
                                                        int8u irLen);

//************************************
// Test functions
#ifndef DOXYGEN_SHOULD_SKIP_THIS
void    emAfTargetCommunicationTestAppEncodingAndDecoding(void);
void    emAfTargetCommunicationTestPreEncodingAndDecoding(void);
void    emAfTargetCommunicationTestUsbTransferSpeed(int8u port);
void    emAfTargetCommunicationTestPrSecond(void);
#endif

//************************************

