// Copyright 2014 Silicon Laboratories, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_HAL
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif
#include EMBER_AF_API_RF4CE_PROFILE
#include EMBER_AF_API_RF4CE_GDP
#include EMBER_AF_API_RF4CE_ZRC20
#include EMBER_AF_API_RF4CE_TARGET_COMMUNICATION
#include EMBER_AF_API_SERIAL

//-----------------------------------------------------------------------------
// #defines
//-----------------------------------------------------------------------------

// This is a test vendor ID and must be substituted with a vendor ID
// for RF4CE certified products. By default it is using the vendor ID specified
// in app builder.
#define RF4CE_VENDOR_ID EMBER_AF_RF4CE_VENDOR_ID

// Vendor specific profile to be used when sending voice packets
#define EMBER_AF_RF4CE_PROFILE_VENDOR_SPECIFIC_VOICE       0xFE

// UART configuration
#if defined(CORTEXM3_EM3588)
#define HOST_PORT       3
#elif defined(CORTEXM3_EM346)
#define HOST_PORT       1
#else
#error Part not supported
#endif
#define HOST_BAUD       BAUD_115200
#define HOST_PARITY     PARITY_NONE
#define HOST_STOPBITS   1

// When the binding button is pressed, the push button state is set to validate
// the current or future binding attempt.  An LED is illuminated during any
// binding attempt to provide status to the user.
#if defined(CORTEXM3_EM3588)
#define BINDING_BUTTON BUTTON0
#elif defined(CORTEXM3_EM346)
#define BINDING_BUTTON BUTTON1
#else
#error Part not supported
#endif

#define BINDING_LED    BOARDLED2

// This application is event driven and the following events are used to
// perform operations like starting network operations and validating the binding
// procedure.
EmberEventControl networkEventControl;
EmberEventControl bindingEventControl;

// The application plays tunes to provide status to the user.  A rising two-
// tone tune indicates success while a falling two-tone tune indicates failure.
// A single note indicates that an operation has begun.
static int8u PGM happyTune[] = {
  NOTE_B4, 1,
  0,       1,
  NOTE_B5, 1,
  0,       0
};
static int8u PGM sadTune[] = {
  NOTE_B5, 1,
  0,       1,
  NOTE_B4, 5,
  0,       0
};
static int8u PGM waitTune[] = {
  NOTE_B4, 1,
  0,       0
};

static void pairingTableCheckAndAdjust(int8u pairingIndex);

void emberAfMainInitCallback(void)
{
#if HOST_PORT != 3
  EmberStatus status;
#endif

  // During startup, the Main plugin will attempt to resume network operations
  // based on information in non-volatile memory.  This is known as a warm
  // start.  If a warm start is possible, the stack status handler will be
  // called during initialization with an indication that the network is up.
  // Otherwise, during a cold start, the device comes up without a network.  We
  // always want a network, but we don't know whether we will be doing a cold
  // start and therefore need to start operations ourselves or if we will be
  // doing a warm start and won't have to do anything.  To handle this
  // uncertainty, we set the network event to active, which will schedule it to
  // fire in the next iteration of the main loop, immediately after
  // initialization.  If we receive a stack status notification indicating that
  // the network is up before the event fires, we know we did a warm start and
  // have nothing to do and can therefore cancel the event.  If the event does
  // fire, we know we did a cold start and need to start network operations
  // ourselves.  This logic is handled here and in the stack status and network
  // event handlers below.

  emberAfAppPrintln( "============== Init Full Featured Target ==============");

#if HOST_PORT != 3
  // Initialize the UART if it is not the USB CDC port that is already opened.
  status = emberSerialInit( HOST_PORT, HOST_BAUD, HOST_PARITY, HOST_STOPBITS);
  emberAfAppPrintln( "emberSerialInit, status=0x%x", status);
#endif

  emberEventControlSetActive(networkEventControl);
  emberAfTargetCommunicationInit( HOST_PORT);
}

void emberAfStackStatusCallback(EmberStatus status)
{
  // When the network comes up, we cancel the network event
  // because we know we did a warm start, as described above.  If the network
  // goes down, we use the same network event to start network operations again
  // as soon as possible.
  if (status == EMBER_NETWORK_UP) {
    emberAfCorePrintln( "Network UP");
    emberEventControlSetInactive(networkEventControl);
    halPlayTune_P(happyTune, TRUE);
  } else if (status == EMBER_NETWORK_DOWN
             && emberNetworkState() == EMBER_NO_NETWORK) {
    emberEventControlSetActive(networkEventControl);
  }
}

void halButtonIsr(int8u button, int8u state)
{
  // When the binding is pressed, we set the corresponding
  // event to active.  This causes the event to fire in the next iteration of
  // the main loop.  This approach is used because button indications are
  // received in an interrupt, so the application must not perform any non-
  // trivial operations here.
  if (button == BINDING_BUTTON && state == BUTTON_PRESSED) {
    emberEventControlSetActive(bindingEventControl);
  }
}

boolean emberAfPluginIdleSleepOkToSleepCallback(int32u durationMs)
{
  // This application never sleeps, so FALSE is always returned.
  return FALSE;
}

void emberAfPluginRf4ceGdpStartValidationCallback(int8u pairingIndex)
{
  // This function is called to begin the validation procedure for a binding
  // attempt.  We simply indicate the attempt to the user and wait for a button
  // push to confirm validation or for a timeout.
  emberAfCorePrintln("Binding %p: 0x%x", "attempt", pairingIndex);
  halPlayTune_P(waitTune, TRUE);
  halSetLed(BINDING_LED);
}

boolean emberAfPluginRf4ceGdpIncomingBindProxyCallback(const EmberEUI64 ieeeAddr)
{
  // This function is called to accept a proxy binding attempt.  We always
  // accept proxy bindings after indicating the attempt to the user.
  emberAfCorePrintln("Proxy binding %p", "attempt");
  halPlayTune_P(waitTune, TRUE);
  halSetLed(BINDING_LED);
  return TRUE;
}

void emberAfPluginRf4ceGdpBindingCompleteCallback(
                                         EmberAfRf4ceGdpBindingStatus status,
                                         int8u pairingIndex)
{
  // When the binding process completes, we receive notification of the success
  // or failure of the operation.
  if (status == EMBER_SUCCESS) {
    emberAfCorePrintln("Binding %p: 0x%x", "complete", pairingIndex);
    halPlayTune_P(happyTune, TRUE);
    // After a successful bind, notify the controller that new action mappings
    // may be available.
    emberAfTargetCommunicationControllerNotify(pairingIndex);
  } else {
    emberAfCorePrintln("Binding %p: 0x%x", "failed", status);
    halPlayTune_P(sadTune, TRUE);
  }
  halClearLed(BINDING_LED);
  pairingTableCheckAndAdjust(pairingIndex);
}

void emberAfPluginRf4ceZrc20ActionCallback(
                                     const EmberAfRf4ceZrcActionRecord *record)
{
  emberAfCorePrintln( "Action Callback: pairingIndex=%d", record->pairingIndex);
  // Send the action to the host.
  emberAfTargetCommunicationHostActionTx(HOST_PORT,
                                         record->actionType,
                                         record->modifierBits,
                                         record->actionBank,
                                         record->actionCode,
                                         record->actionVendorId);
}

void emberAfPluginRf4ceProfileIncomingMessageCallback(int8u pairingIndex,
                                                      int8u profileId,
                                                      int16u vendorId,
                                                      EmberRf4ceTxOption txOptions,
                                                      const int8u *message,
                                                      int8u messageLength)
{
  // Voice audio is received in a vendor specific profile and passed on to the
  // host
  if ((profileId == EMBER_AF_RF4CE_PROFILE_VENDOR_SPECIFIC_VOICE)
      && (vendorId == RF4CE_VENDOR_ID)){
    emberAfTargetCommunicationHostAudioTx(HOST_PORT, message, messageLength);
  }
}

void networkEventHandler(void)
{
  // The network event is scheduled during initialization to handle cold starts
  // and also if the network ever goes down.  In either situation, we want to
  // start network operations.  If the call succeeds, we are done and just wait
  // for the inevitable stack status notification.  If it fails, we set the
  // event to try again right away.
  if (emberAfRf4ceStart() == EMBER_SUCCESS) {
    emberEventControlSetInactive(networkEventControl);
    halPlayTune_P(waitTune, TRUE);
  } else {
    emberEventControlSetActive(networkEventControl);
    halPlayTune_P(sadTune, TRUE);
  }
}

void bindingEventHandler(void)
{
  // The binding event is scheduled whenever the binding button is pressed.  We
  // set the push button state to validate the current binding attempt or to
  // automatically validate a future binding attempt.  The push button state is
  // cleared automatically by the GDP plugin after some time, so the binding
  // button needs to be pressed for each binding attempt.
  emberAfCorePrintln( "Binding event handler");
  emberAfRf4ceGdpPushButton(TRUE);
  halPlayTune_P(waitTune, TRUE);
  emberEventControlSetInactive(bindingEventControl);
}


// The purpose of this function is to erase entries in the pairing table when
// it gets full. Only the current entry is kept.
// This will remove all previous entries and make room for new pairings.
static void pairingTableCheckAndAdjust(int8u pairingIndex)
{
  EmberRf4cePairingTableEntry xEnt;
  int8u i;

  for(i=0 ; i<EMBER_RF4CE_PAIRING_TABLE_SIZE ; i++) {
    emberAfRf4ceGetPairingTableEntry(i, &xEnt);
    if( emberAfRf4cePairingTableEntryIsUnused(&xEnt)) {
      break;
    }
  }

  if(i==EMBER_RF4CE_PAIRING_TABLE_SIZE) {
    emberAfCorePrintln("  Erasing pairing table entries...");
    for(i=0 ; i<EMBER_RF4CE_PAIRING_TABLE_SIZE ; i++) {
      if(i!=pairingIndex) {
        emberAfRf4ceSetPairingTableEntry(i, NULL);
      }
    }
  }
}


