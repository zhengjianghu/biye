
// Simulated low-level routines for Hardware AES Encrypt.
// This utilizes the public Rijndael AES implementation to do the heavily
// lifting.

#include PLATFORM_HEADER

#include "hal/micro/generic/aes/rijndael-api-fst.h"
#include "hal/micro/generic/aes/rijndael-alg-fst.h"

//------------------------------------------------------------------------------
// Globals

// These are used to keep track of data used by the Rijndael (AES) block cipher
// implementation.
static keyInstance myKey;
static boolean cipherInitialized = FALSE;
static cipherInstance myCipher;
static int8u myKeyData[] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
                               0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
static boolean keyInitialized = FALSE;

// We only support AES-128
#define SECURITY_BLOCK_SIZE  16 // bytes

//------------------------------------------------------------------------------
// Forward Declarations

//------------------------------------------------------------------------------

static void initializeCipher(void)
{
  // ZigBee defines the IV for AES Encrypt to be zero, per the Spec. section
  // B.1 Block-Cipher-Based Cryptographic Hash Function

  assert ( TRUE == cipherInit(&myCipher, 
                              MODE_CBC, // XXX: Is this correct???
                              NULL));   // Initialization Vector
  cipherInitialized = TRUE;
}

//------------------------------------------------------------------------------
// This routine simulates loading a key into our cryptographic engine by
// creating (and caching) a key suitable for use in our Unit Test 
// AES Block Cihper.

void emLoadKeyIntoCore(int8u* keyData)
{
  char keyMaterial[SECURITY_BLOCK_SIZE * 2];
  int8u i;
  int8u j = 0;

//  if ( keyInitialized && 
//       0 == memcmp(keyData, myKeyData, SECURITY_BLOCK_SIZE) )
//    return TRUE;

  // The Key material we pass is expected to be in ASCII.
  // So we need to convert the raw HEX into ASCII just
  // so it can be converted back by 'makeKey()'.
  for ( i = 0; i < SECURITY_BLOCK_SIZE; i++ ) {
    int8u nibble = keyData[i] >> 4;
    if ( nibble <= 0x09 )
      keyMaterial[j] = 0x30 + nibble;
    else // if ( nibble >= 0xA && nibble <= 0xF )
      keyMaterial[j] = 0x41 + nibble - 0x0A;
    j++;

    nibble = keyData[i] & 0x0F;
    if ( nibble <= 0x9 )
      keyMaterial[j] = 0x30 + nibble;
    else // if ( nibble >= 0xA && nibble <= 0xF )
      keyMaterial[j] = 0x41 + nibble - 0x0A;
    j++;

    // Cache in our global
    myKeyData[i] = keyData[i];
  }
  assert(0 <= makeKey(&myKey, 
                      DIR_ENCRYPT, 
                      SECURITY_BLOCK_SIZE * 8,
                      keyMaterial));

  keyInitialized = TRUE;
}

void emLoadDecryptKey(int8u* keyData)
{
  char keyMaterial[SECURITY_BLOCK_SIZE * 2];
  int8u i;
  int8u j = 0;

//  if ( keyInitialized &&
//       0 == memcmp(keyData, myKeyData, SECURITY_BLOCK_SIZE) )
//    return TRUE;

  // The Key material we pass is expected to be in ASCII.
  // So we need to convert the raw HEX into ASCII just
  // so it can be converted back by 'makeKey()'.
  for ( i = 0; i < SECURITY_BLOCK_SIZE; i++ ) {
    int8u nibble = keyData[i] >> 4;
    if ( nibble <= 0x09 )
      keyMaterial[j] = 0x30 + nibble;
    else // if ( nibble >= 0xA && nibble <= 0xF )
      keyMaterial[j] = 0x41 + nibble - 0x0A;
    j++;

    nibble = keyData[i] & 0x0F;
    if ( nibble <= 0x9 )
      keyMaterial[j] = 0x30 + nibble;
    else // if ( nibble >= 0xA && nibble <= 0xF )
      keyMaterial[j] = 0x41 + nibble - 0x0A;
    j++;

    // Cache in our global
    myKeyData[i] = keyData[i];
  }
  assert(0 <= makeKey(&myKey,
                      DIR_DECRYPT,
                      SECURITY_BLOCK_SIZE * 8,
                      keyMaterial));

  keyInitialized = TRUE;
}

//------------------------------------------------------------------------------
// Retrieve key cached in our global.

void emGetKeyFromCore(int8u* keyPointer)
{
  MEMCOPY(keyPointer, myKeyData, SECURITY_BLOCK_SIZE);
}

//------------------------------------------------------------------------------

void emSecurityHardwareInit(void)
{
  if ( ! cipherInitialized )
    initializeCipher();
}

//------------------------------------------------------------------------------
// This function utilizes the Rijndael AES Block Cipher implementation to
// perform real AES Encryption.  See 'hal/micro/generic/aes/rijndael-api-fst.c'

void emStandAloneDecryptBlock(int8u block[SECURITY_BLOCK_SIZE])
{
  int8u outBlock[SECURITY_BLOCK_SIZE];

  if ( ! cipherInitialized )
    initializeCipher();

  assert(0 <= blockDecrypt(&myCipher,
                           &myKey,
                           block,
                           SECURITY_BLOCK_SIZE * 8,
                           outBlock));

  MEMCOPY(block, outBlock, SECURITY_BLOCK_SIZE);
}

void emStandAloneEncryptBlock(int8u block[SECURITY_BLOCK_SIZE])
{
  int8u outBlock[SECURITY_BLOCK_SIZE];

  if ( ! cipherInitialized )
    initializeCipher();

  assert(0 <= blockEncrypt(&myCipher, 
                           &myKey, 
                           block, 
                           SECURITY_BLOCK_SIZE * 8, 
                           outBlock));

  MEMCOPY(block, outBlock, SECURITY_BLOCK_SIZE);
}

//----------------------------------------------------------------
// Wrapper for those that just want access to AES.

void emAesEncrypt(int8u block[SECURITY_BLOCK_SIZE], int8u *key)
{
  ATOMIC(
         emLoadKeyIntoCore(key);
         emStandAloneEncryptBlock(block);
       )
}

void emAesDecrypt(int8u block[SECURITY_BLOCK_SIZE], int8u *key)
{
  ATOMIC(
         emLoadDecryptKey(key);
         emStandAloneDecryptBlock(block);
       )
}
