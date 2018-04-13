// *******************************************************************
// * af-main-host.c
// *
// *
// * Copyright 2007 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include PLATFORM_HEADER
#include "stack/include/ember-types.h"
#include "stack/include/error.h"
#include "app/util/ezsp/ezsp-protocol.h"
#include "app/util/ezsp/ezsp.h"
#include "hal/micro/system-timer.h"
#include "hal/micro/generic/ash-protocol.h"
#include "app/util/source-route-host.h"


#include "ncp.h"
#include "zcl-parse.h"


uint16_t C4GetSendOptions(int16u destination, int16u profileId, int16u clusterId, int8u messageLength);
void C4IncomingMessageHandler(EmberIncomingMessageType type,
                                EmberApsFrame *apsFrame,
                                int8u lastHopLqi,
                                int8s lastHopRssi,
                                EmberNodeId sender,
                                int8u bindingIndex,
                                int8u addressIndex,
                                int8u messageLength,
                                int8u *messageContents);

DEFINE_LOGGER(NCP, 128, LOG_DEBUG);



static boolean ncpNeedsResetAndInit = FALSE;
static boolean currentSenderEui64IsValid;
static EmberEUI64 currentSenderEui64;


// The address table plugin is enabled by default. If it gets disabled for some
// reason, we still need to define these #defines to some default value.
#ifndef EMBER_AF_PLUGIN_ADDRESS_TABLE
#define EMBER_AF_PLUGIN_ADDRESS_TABLE_SIZE 2
#define EMBER_AF_PLUGIN_ADDRESS_TABLE_TRUST_CENTER_CACHE_SIZE 2
#endif

#if defined(EMBER_AF_PLUGIN_CONCENTRATOR)
#define SOURCE_ROUTE_TABLE_SIZE EMBER_SOURCE_ROUTE_TABLE_SIZE
#else
// Define a small size that will not consume much memory on NCP
#define SOURCE_ROUTE_TABLE_SIZE 2
#endif

#define ZIGBEE_PRIMARY_CHANNEL_MASK ((int32u)(BIT32(11)    \
                                                 | BIT32(14)  \
                                                 | BIT32(15)  \
                                                 | BIT32(19)  \
                                                 | BIT32(20)  \
                                                 | BIT32(24)  \
                                                 | BIT32(25)))
#define ZIGBEE_NETWORK_CREATOR_RADIO_POWER 3

static void PrintEmberVersion(EmberVersion versionStruct)
{
    char_t version[32];

    if (versionStruct.special != 0) {
        snprintf(version, sizeof(version), "stack ver. [%d.%d.%d.%d build %d]\n",
                 versionStruct.major,
                 versionStruct.minor,
                 versionStruct.patch,
                 versionStruct.special,
                 versionStruct.build);
    } else {
        snprintf(version, sizeof(version), "stack ver. [%d.%d.%d build %d]\n",
                 versionStruct.major,
                 versionStruct.minor,
                 versionStruct.patch,
                 versionStruct.build);
    }
    LogInfo(version);
}

static void ncpGetVersion(void)
{
    // Note that NCP == Network Co-Processor

    EmberVersion versionStruct;

    // the EZSP protocol version that the NCP is using
    int8u ncpEzspProtocolVer;

    // the stackType that the NCP is running
    int8u ncpStackType;

    int16u ncpStackVer;

    // the EZSP protocol version that the Host is running
    // we are the host so we set this value
    int8u hostEzspProtocolVer = EZSP_PROTOCOL_VERSION;

    // send the Host version number to the NCP. The NCP returns the EZSP
    // version that the NCP is running along with the stackType and stackVersion
    ncpEzspProtocolVer = ezspVersion(hostEzspProtocolVer,
                                     &ncpStackType,
                                     &ncpStackVer);

    // verify that the stack type is what is expected
    if (ncpStackType != EZSP_STACK_TYPE_MESH) {
        LogError("stack type 0x%x is not expected!\n",
                 ncpStackType);
        assert(FALSE);
    }

    // verify that the NCP EZSP Protocol version is what is expected
    if (ncpEzspProtocolVer != EZSP_PROTOCOL_VERSION) {
        LogError("NCP EZSP protocol version of 0x%x does not match Host version 0x%x\r\n",
                 ncpEzspProtocolVer,
                 hostEzspProtocolVer);
        assert(FALSE);
    }

    LogInfo("ezsp ver 0x%x stack type 0x%x \n",
            ncpEzspProtocolVer, ncpStackType);

    if (EZSP_SUCCESS != ezspGetVersionStruct(&versionStruct)) {
        // NCP has Old style version number
        LogInfo("stack ver [0x%2x]\n", ncpStackVer);
    } else {
        // NCP has new style version number
        PrintEmberVersion(versionStruct);
    }
}
// **********************************************************************
// this function sets an EZSP config value and prints out the results to
// the serial output
// **********************************************************************
EzspStatus emberAfSetEzspConfigValue(EzspConfigId configId,
                                     int16u value,
                                     PGM_P configIdName)
{

    EzspStatus ezspStatus = ezspSetConfigurationValue(configId, value);
    LogDebug("Ezsp Config: set %s to 0x%2x:\n", configIdName, value);

    // If this fails, odds are the simulated NCP doesn't have enough
    // memory allocated to it.
    if(ezspStatus != EZSP_SUCCESS)
    {
        LogError("Ezsp Config set error 0x%x\n", ezspStatus);
    }

    return ezspStatus;
}


// **********************************************************************
// this function sets an EZSP policy and prints out the results to
// the serial output
// **********************************************************************
EzspStatus emberAfSetEzspPolicy(EzspPolicyId policyId,
                                EzspDecisionId decisionId,
                                PGM_P policyName,
                                PGM_P decisionName)
{
    EzspStatus ezspStatus = ezspSetPolicy(policyId, decisionId);
    LogDebug("Ezsp Policy: set %s to \"%s\"\n", policyName, decisionName);

    if(ezspStatus != EZSP_SUCCESS)
    {
        LogError("Ezsp Policy set error 0x%x\n", ezspStatus);
    }
    return ezspStatus;
}

// **********************************************************************
// this function sets an EZSP value and prints out the results to
// the serial output
// **********************************************************************
EzspStatus emberAfSetEzspValue(EzspValueId valueId,
                               int8u valueLength,
                               int8u *value,
                               PGM_P valueName)

{
    EzspStatus ezspStatus = ezspSetValue(valueId, valueLength, value);
    int32u promotedValue = 0;
    if (valueLength <= 4) {
        promotedValue = (int32u)(*value);
    }
    LogDebug("Ezsp Value : set %s to ", valueName);

    // print the value based on the length of the value
    switch (valueLength) {
    case 1:
    case 2:
    case 4:
        LogAppend(LOG_DEBUG, "0x%4x\n", promotedValue);
        break;
    default:
        LogAppend(LOG_DEBUG, "{val of len %x}\n", valueLength);
    }

    if(ezspStatus != EZSP_SUCCESS)
    {
        LogError("Ezsp Value set error 0x%x\n", ezspStatus);
    }

    return ezspStatus;
}



static int8u ezspNextSequence(void)
{
    static int8u ezspSequenceNumber = 0;
    ezspSequenceNumber++;
    if((ezspSequenceNumber == 0) || (ezspSequenceNumber == 0xff))
    {
        ezspSequenceNumber = 1;
    }
    return ezspSequenceNumber;
}

EmberStatus emberAfEzspSetSourceRoute(EmberNodeId id)
{
#ifdef EZSP_APPLICATION_HAS_ROUTE_RECORD_HANDLER
    int16u relayList[ZA_MAX_HOPS];
    int8u relayCount;
    if (emberFindSourceRoute(id, &relayCount, relayList)
            && ezspSetSourceRoute(id, relayCount, relayList) != EMBER_SUCCESS) {
        return EMBER_SOURCE_ROUTE_FAILURE;
    }
#endif
    return EMBER_SUCCESS;
}


static void storeLowHighInt16u(int8u* contents, int16u value)
{
    contents[0] = LOW_BYTE(value);
    contents[1] = HIGH_BYTE(value);
}
// This is an ineffecient way to generate a random key for the host.
// If there is a pseudo random number generator available on the host,
// that may be a better mechanism.

EmberStatus emberAfGenerateRandomKey(EmberKeyData* result)
{
    int16u data;
    int8u* keyPtr = emberKeyContents(result);
    int8u i;

    // Since our EZSP command only generates a random 16-bit number,
    // we must call it repeatedly to get a 128-bit random number.

    for ( i = 0; i < 8; i++ ) {
        EmberStatus status = ezspGetRandomNumber(&data);

        if ( status != EMBER_SUCCESS ) {
            return status;
        }

        storeLowHighInt16u(keyPtr, data);
        keyPtr+=2;
    }

    return EMBER_SUCCESS;
}




EmberStatus zaTrustCenterSecurityInit(boolean centralizedNetwork)
{
    static EmberKeyData preconfiguredLinkKey = EMBER_AF_SECURITY_PROFILE_HA_PRECONFIGURED_KEY;
    EmberInitialSecurityState state;
    EmberExtendedSecurityBitmask extended;
    EmberStatus status;

    MEMSET(&state, 0, sizeof(EmberInitialSecurityState));
    state.bitmask = EMBER_AF_SECURITY_PROFILE_HA_TC_SECURITY_BITMASK;
    MEMMOVE(emberKeyContents(&state.preconfiguredKey),
            &preconfiguredLinkKey,
            EMBER_ENCRYPTION_KEY_SIZE);

    // Random network key (highly recommended)
    status = emberAfGenerateRandomKey(&(state.networkKey));
    if (status != EMBER_SUCCESS) {
        return status;
    }

// Check for distributed network.
    if (!centralizedNetwork)
        state.bitmask |= EMBER_DISTRIBUTED_TRUST_CENTER_MODE;

    LogDebug("set state to: 0x%2x\n", state.bitmask);
    status = emberSetInitialSecurityState(&state);
    if (status != EMBER_SUCCESS) {
        LogError("security init TC: 0x%x\n", status);
        return status;
    }

    extended = EMBER_AF_SECURITY_PROFILE_HA_TC_EXTENDED_SECURITY_BITMASK;
	


	
    // Don't need to check on the status here, emberSetExtendedSecurityBitmask
    // always returns EMBER_SUCCESS.
    LogDebug("set extended security to: 0x%2x\n", extended);
    emberSetExtendedSecurityBitmask(extended);


    emberAfSetEzspPolicy(EZSP_TC_KEY_REQUEST_POLICY,
                         EMBER_AF_SECURITY_PROFILE_HA_TC_LINK_KEY_REQUEST_POLICY,
                         "TC Key Request",
                         (EMBER_AF_SECURITY_PROFILE_HA_TC_LINK_KEY_REQUEST_POLICY
                          ==  EMBER_AF_ALLOW_TC_KEY_REQUESTS
                          ? "Allow"
                          : "Deny"));

    emberAfSetEzspPolicy(EZSP_APP_KEY_REQUEST_POLICY,
                         EMBER_AF_SECURITY_PROFILE_HA_APP_LINK_KEY_REQUEST_POLICY,
                         "App. Key Request",
                         (EMBER_AF_SECURITY_PROFILE_HA_APP_LINK_KEY_REQUEST_POLICY
                          == EMBER_AF_ALLOW_APP_KEY_REQUESTS
                          ? "Allow"
                          : "Deny"));

    emberAfSetEzspPolicy(EZSP_TRUST_CENTER_POLICY,
                         EZSP_ALLOW_PRECONFIGURED_KEY_JOINS,
                         "Trust Center Policy",
                         "Allow preconfigured key joins");



    if (status != EMBER_SUCCESS) {
        return status;
    }

#if EMBER_KEY_TABLE_SIZE
    emberClearKeyTable();
#endif

    return EMBER_SUCCESS;
}

#if 1
#define PORTA_PIN(y) ((0<<3)|y)
#define PORTB_PIN(y) ((1<<3)|y)
#define PORTC_PIN(y) ((2<<3)|y)
#define PORTD_PIN(y) ((3<<3)|y)
#define PORTE_PIN(y) ((4<<3)|y)
#define PORTF_PIN(y) ((5<<3)|y)
#endif
void ezspSetAPGpioConfig(void)
{	
	EzspStatus err = 0;
	
	err = ezspSetGpioCurrentConfiguration(PORTA_PIN(7),1,0);
	if(err > EZSP_SUCCESS)
		{LogDebug("%s %d err = %d\n",__FUNCTION__,__LINE__,err);}
	err = ezspSetGpioCurrentConfiguration(PORTA_PIN(3),1,1);
	if(err > EZSP_SUCCESS)
		{LogDebug("%s %d err = %d\n",__FUNCTION__,__LINE__,err);}
	err = ezspSetGpioCurrentConfiguration(PORTA_PIN(6),1,1);
	if(err > EZSP_SUCCESS)
		{LogDebug("%s %d err = %d\n",__FUNCTION__,__LINE__,err);}
	err = ezspSetGpioCurrentConfiguration(PORTC_PIN(5),9,0);
	if(err > EZSP_SUCCESS)
		{LogDebug("%s %d err = %d\n",__FUNCTION__,__LINE__,err);}
}

void ezspNetworkFoundHandler(EmberZigbeeNetwork *networkFound,
                             int8u lqi,
                             int8s rssi)
{
#if 0
    emberAfPushCallbackNetworkIndex();
    emberAfNetworkFoundCallback(networkFound, lqi, rssi);
    emberAfPopNetworkIndex();
#endif
    LogDebug("ezspNetworkFoundHandler\n");
}

void ezspScanCompleteHandler(int8u channel, EmberStatus status)
{
#if 0
    emberAfPushCallbackNetworkIndex();
    emberAfScanCompleteCallback(channel, status);
    emberAfPopNetworkIndex();
#endif
    LogDebug("ezspScanCompleteHandler\n");
}

void ezspEnergyScanResultHandler(int8u channel, int8s rssi)
{
#if 0
    emberAfPushCallbackNetworkIndex();
    emberAfEnergyScanResultCallback(channel, rssi);
    emberAfPopNetworkIndex();
#endif
    LogDebug("ezspEnergyScanResultHandler\n");
}


// This is not called if the incoming message did not contain the EUI64 of
// the sender.
void ezspIncomingSenderEui64Handler(EmberEUI64 senderEui64)
{
	int8u i = 0;
	
    // current sender is now valid
    MEMMOVE(currentSenderEui64, senderEui64, EUI64_SIZE);
    currentSenderEui64IsValid = TRUE;

	LogDebug("%s\n",__FUNCTION__);
	LogAppend(LOG_INFO,"ieee=");
	for(i = 0;i < 8;i++)
	{
		LogAppend(LOG_INFO,"%02x",senderEui64[7-i]);
	}
	LogAppend(LOG_INFO,"\n");
}


/*
undefined reference to `ezspIncomingRouteErrorHandler'
undefined reference to `ezspTrustCenterJoinHandler'
*/
void ezspTrustCenterJoinHandler(EmberNodeId newNodeId,
                                EmberEUI64 newNodeEui64,
                                EmberDeviceUpdate status,
                                EmberJoinDecision policyDecision,
                                EmberNodeId parentOfNewNode)
{
    LogDebug("%s\n",__FUNCTION__);
	LogHexDump(LOG_INFO, "newNodeEui64", newNodeEui64, 8);
	LogDebug("newNodeId=0x%x;\n",newNodeId);
	LogDebug("status=%d;\n",status);
	LogDebug("policyDecision=%d;\n",policyDecision);
	LogDebug("parentOfNewNode=0x%x;\n",parentOfNewNode);	

	if(status == 2)
	{
		LogDebug("%s DEVICE_LEFT\n",__FUNCTION__);
		clearNodeBuf(newNodeId);
		zapNodeNetworkStatusNotify(0,newNodeEui64,NET_TYPE,0,0,0,NULL,NULL);
	}	
}

void ezspIncomingRouteErrorHandler(EmberStatus status, EmberNodeId target)
{
    LogDebug("%s\n",__FUNCTION__);
}


// This is called when an EZSP error is reported
void ezspErrorHandler(EzspStatus status)
{
    LogError("ezspErrorHandler 0x%x", status);

    // Rather than detect whether or not we can recover from the error,
    // we just flag the NCP for reboot.
    ncpNeedsResetAndInit = TRUE;
}
// *******************************************************************
// Handlers required to use the Ember Stack.

// Called when the stack status changes, usually as a result of an
// attempt to form, join, or leave a network.
void ezspStackStatusHandler(EmberStatus status)
{
#if 0
    emberAfPushCallbackNetworkIndex();
    emAfStackStatusHandler(status);
    emberAfPopNetworkIndex();
#endif
	LogDebug("%s\n",__FUNCTION__);
    switch (status) {
    case EMBER_NETWORK_UP:
    case EMBER_TRUST_CENTER_EUI_HAS_CHANGED:  // also means NETWORK_UP
        LogDebug("EMBER_NETWORK_UP\n");
		zapNetworkStatusNotify(1);
        break;

    case EMBER_RECEIVED_KEY_IN_THE_CLEAR:
    case EMBER_NO_NETWORK_KEY_RECEIVED:
    case EMBER_NO_LINK_KEY_RECEIVED:
    case EMBER_PRECONFIGURED_KEY_REQUIRED:
    case EMBER_MOVE_FAILED:
    case EMBER_JOIN_FAILED:
    case EMBER_NO_BEACONS:
    case EMBER_CANNOT_JOIN_AS_ROUTER:
    case EMBER_NETWORK_DOWN:
        if (status == EMBER_NETWORK_DOWN) {
            LogDebug("EMBER_NETWORK_DOWN\n");
			NodeStatusInit();
			zapNetworkStatusNotify(0);
        } else {
            LogDebug("EMBER_JOIN_FAILED\n");
        }
        break;

    default:
        LogError("EVENT: stackStatus 0x%x\n", status);
    }

}



// Called when a message we sent is acked by the destination or when an
// ack fails to arrive after several retransmissions.
void ezspMessageSentHandler(EmberOutgoingMessageType type,
                            int16u indexOrDestination,
                            EmberApsFrame *apsFrame,
                            int8u messageTag,
                            EmberStatus status,
                            int8u messageLength,
                            int8u *messageContents)
{
	LogDebug("%s indexOrDestination = %x  type = %d status = %d\n",__FUNCTION__,indexOrDestination,type,status);

	uint8_t result = 1;
	EmberEUI64 ieee;
	ezspLookupEui64ByNodeId(indexOrDestination,ieee);

	if(status == EMBER_SUCCESS)
	{
		result = 1;
	}
	else
	{
		result = 0;
	}
	
	zapSentNotify(ieee,apsFrame->profileId,apsFrame->clusterId,
				apsFrame->sourceEndpoint,apsFrame->destinationEndpoint,
                messageLength,messageContents,result);
}

void ezspIncomingMessageHandler(EmberIncomingMessageType type,
                                EmberApsFrame *apsFrame,
                                int8u lastHopLqi,
                                int8s lastHopRssi,
                                EmberNodeId sender,
                                int8u bindingIndex,
                                int8u addressIndex,
                                int8u messageLength,
                                int8u *messageContents)
{
	
	LogDebug("%s\n",__FUNCTION__);

	LogDebug("apsFrame.profileId=0x%04x;\n",apsFrame->profileId);
	LogDebug("apsFrame.clusterId=0x%04x;\n",apsFrame->clusterId);
	LogDebug("apsFrame.sourceEndpoint=%d;\n",apsFrame->sourceEndpoint);
	LogDebug("apsFrame.destinationEndpoint=%d;\n",apsFrame->destinationEndpoint);
	LogDebug("apsFrame.options=0x%x;\n",apsFrame->options);
	LogDebug("apsFrame.groupId=%d;\n",apsFrame->groupId);
	LogDebug("apsFrame.sequence=0x%x;\n",apsFrame->sequence);

	LogDebug("type=%d;\n",type);
	LogDebug("lastHopLqi=%d;\n",lastHopLqi);
	LogDebug("lastHopRssi=%d;\n",lastHopRssi);
	LogDebug("sender=0x%x;\n",sender);
	LogDebug("bindingIndex=0x%x;\n",bindingIndex);
	LogDebug("addressIndex=0x%x;\n",addressIndex);

	LogDebug("soso:messageLength=%d\n",messageLength);
	LogHexDump(LOG_INFO, "messageContents", messageContents, messageLength);
	
	EmberEUI64 addrttt;
	ezspLookupEui64ByNodeId(sender,&addrttt[0]);
	LogHexDump(LOG_INFO, "ezspLookupEui64ByNodeId", addrttt, 8);

	//u64 ieee64bit = 0;
		
    currentSenderEui64IsValid = FALSE;	
    if(apsFrame->profileId == 0xc25d)
    {
        C4IncomingMessageHandler(type,
                                 apsFrame,
                                 lastHopLqi,
                                 lastHopRssi,
                                 sender,
                                 bindingIndex,
                                 addressIndex,
                                 messageLength,
                                 messageContents);
    }
	else if(apsFrame->profileId == 0x0000)
	{
		LogDebug("ZDO\n");
	}
	else
	{
		EmberEUI64 ieee;
		ezspLookupEui64ByNodeId(sender,ieee);
		zapNewPacketNotify(ieee,apsFrame->profileId,apsFrame->clusterId,
							apsFrame->sourceEndpoint,apsFrame->destinationEndpoint,
                      		messageLength,messageContents);
	}
}

void ezspIncomingRouteRecordHandler(EmberNodeId source,
                                    EmberEUI64 sourceEui,
                                    int8u lastHopLqi,
                                    int8s lastHopRssi,
                                    int8u relayCount,
                                    int8u *relayList)
{
	LogDebug("%s\n",__FUNCTION__);
	LogDebug("source = 0x%04x\n",source);
	LogHexDump(LOG_INFO,"sourceEui",sourceEui,8);
	LogDebug("lastHopLqi = 0x%02x\n",lastHopLqi);
	LogDebug("lastHopRssi = %d\n",lastHopRssi);
	LogDebug("relayCount = 0x%02x\n",relayCount);
	//LogDebug("source = 0x%02x\n",source);
}
void ezspChildJoinHandler(int8u index,
                          boolean joining,
                          EmberNodeId childId,
                          EmberEUI64 childEui64,
                          EmberNodeType childType)
{
	LogDebug("%s\n",__FUNCTION__);

	LogDebug("index = %d\n",index);
	LogDebug("joining = %d\n",joining);
	LogDebug("childId = 0x%04x\n",childId);
	LogHexDump(LOG_INFO,"childEui64\n",childEui64,8);	
	LogDebug("childType = %d\n",childType);

	if(!joining)
	{
		LogDebug("%s DEVICE_LEFT\n",__FUNCTION__);
		clearNodeBuf(childId);
		zapNodeNetworkStatusNotify(0,childEui64,NET_TYPE,0,0,0,NULL,NULL);
	}
}


/*出错返回0xFF，成功返回messageTag用于confirm，如果不需要confirm则返回0*/
uint8_t ZigbeeUnicast(int16u destination,
                      int16u profileId,
                      int16u clusterId,
                      int8u sourceEndpoint,
                      int8u destinationEndpoint,
                      int8u messageLength,
                      int8u* message,
                      bool_t needConfirm)
{
    EmberStatus status;
    EmberApsFrame apsFrame;
    uint8_t messageTag = 0;


    apsFrame.profileId = profileId;
    apsFrame.clusterId = clusterId;
    apsFrame.sourceEndpoint = sourceEndpoint;
    apsFrame.destinationEndpoint = destinationEndpoint;
    apsFrame.options = C4GetSendOptions(destination, profileId, clusterId, messageLength);

    LogDebug("unicast to dst 0x%04x, profile 0x%04x, cluster 0x%04x, SE/DE %d/%d len=%d\r\n",
             destination,
             apsFrame.profileId,
             apsFrame.clusterId,
             apsFrame.sourceEndpoint,
             apsFrame.destinationEndpoint,
             messageLength);

    if(needConfirm)
    {
        messageTag = ezspNextSequence();
    }


    status = emberAfEzspSetSourceRoute(destination);
    if (status == EMBER_SUCCESS) {
        status = ezspSendUnicast(EMBER_OUTGOING_DIRECT,
                                 destination,
                                 &apsFrame,
                                 messageTag,
                                 messageLength,
                                 message,
                                 &apsFrame.sequence);
    }

    if (status == EMBER_SUCCESS)
    {
        return messageTag;
    }
    else
    {
        return 0xFF;
    }

}


#define ZA_PROFILE_ID (0x104)
uint8_t zcl_tx(uint8 *dat, uint8 len, uint16 dest, uint16 cID, uint8 SE, uint8 DE)
{
    return ZigbeeUnicast(dest,ZA_PROFILE_ID,cID,SE,DE,len,dat,FALSE);
}



void zapInit(void)
{
    EzspStatus ezspStatus;

    LogDebug("ZigbeeInit\n");
	LogDebug("ZigbeeInit shenkai log\n");

    // ezspInit resets the NCP by calling halNcpHardReset on a SPI host or
    // ashResetNcp on a UART host
    ezspStatus = ezspInit();
	LogDebug("%s %d\n",__FUNCTION__,__LINE__);

    if (ezspStatus != EZSP_SUCCESS) {
        LogError("ezspInit 0x%x\n", ezspStatus);
        assert(FALSE);
    }
    LogDebug("ezspInit OK\n");

    ncpGetVersion();

	ezspSetAPGpioConfig();

    // set the source route table size
    emberAfSetEzspConfigValue(EZSP_CONFIG_SOURCE_ROUTE_TABLE_SIZE,
                              SOURCE_ROUTE_TABLE_SIZE,
                              "source route table size");

    emberAfSetEzspConfigValue(EZSP_CONFIG_SECURITY_LEVEL,
                              EMBER_SECURITY_LEVEL,
                              "security level");

    // set the address table size
    emberAfSetEzspConfigValue(EZSP_CONFIG_ADDRESS_TABLE_SIZE,
                              EMBER_AF_PLUGIN_ADDRESS_TABLE_SIZE,
                              "address table size");

    // set the trust center address cache size
    emberAfSetEzspConfigValue(EZSP_CONFIG_TRUST_CENTER_ADDRESS_CACHE_SIZE,
                              EMBER_AF_PLUGIN_ADDRESS_TABLE_TRUST_CENTER_CACHE_SIZE,
                              "TC addr cache");

    // the stack profile is defined in the config file
    emberAfSetEzspConfigValue(EZSP_CONFIG_STACK_PROFILE,
                              EMBER_STACK_PROFILE,
                              "stack profile");


    // MAC indirect timeout should be 7.68 secs
    emberAfSetEzspConfigValue(EZSP_CONFIG_INDIRECT_TRANSMISSION_TIMEOUT,
                              7680,
                              "MAC indirect TX timeout");

    // Max hops should be 2 * nwkMaxDepth, where nwkMaxDepth is 15
    emberAfSetEzspConfigValue(EZSP_CONFIG_MAX_HOPS,
                              30,
                              "max hops");

    emberAfSetEzspConfigValue(EZSP_CONFIG_TX_POWER_MODE,
                              EMBER_AF_TX_POWER_MODE,
                              "tx power mode");

    emberAfSetEzspConfigValue(EZSP_CONFIG_SUPPORTED_NETWORKS,
                              EMBER_SUPPORTED_NETWORKS,
                              "supported networks");

    // allow other devices to modify the binding table
    emberAfSetEzspPolicy(EZSP_BINDING_MODIFICATION_POLICY,
                         EZSP_CHECK_BINDING_MODIFICATIONS_ARE_VALID_ENDPOINT_CLUSTERS,
                         "binding modify",
                         "allow for valid endpoints & clusters only");

    // return message tag and message contents in ezspMessageSentHandler()
    emberAfSetEzspPolicy(EZSP_MESSAGE_CONTENTS_IN_CALLBACK_POLICY,
                         EZSP_MESSAGE_TAG_AND_CONTENTS_IN_CALLBACK,
                         "message content in msgSent",
                         "return");

    emberAfSetEzspPolicy(EZSP_UNICAST_REPLIES_POLICY,
                         EZSP_HOST_WILL_NOT_SUPPLY_REPLY,
                         "unicast reply policy",
                         "ncp will reply");

    {
        int8u value[2];
        value[0] = LOW_BYTE(EMBER_AF_INCOMING_BUFFER_LENGTH);
        value[1] = HIGH_BYTE(EMBER_AF_INCOMING_BUFFER_LENGTH);
        emberAfSetEzspValue(EZSP_VALUE_MAXIMUM_INCOMING_TRANSFER_SIZE,
                            2,     // value length
                            value,
                            "maximum incoming transfer size");
        value[0] = LOW_BYTE(EMBER_AF_MAXIMUM_SEND_PAYLOAD_LENGTH);
        value[1] = HIGH_BYTE(EMBER_AF_MAXIMUM_SEND_PAYLOAD_LENGTH);
        emberAfSetEzspValue(EZSP_VALUE_MAXIMUM_OUTGOING_TRANSFER_SIZE,
                            2,     // value length
                            value,
                            "maximum outgoing transfer size");
    }
}

void zapTick(void)
{
    // see if the NCP has anything waiting to send us
    ezspTick();	
    while (ezspCallbackPending()) 
	{
        ezspCallback();
    }

#if 0
    // check if we have hit an EZSP Error and need to reset and init the NCP
    if (ncpNeedsResetAndInit) {
        ncpNeedsResetAndInit = FALSE;
        // re-initialize the NCP
        emAfResetAndInitNCP();
    }
#endif
}

uint8_t zapNetworkInit(void)
{
	if(ezspNetworkInit() == EMBER_SUCCESS)
		return 1;
	return 0;
}
void zapSendMTORR(void)
{
	LogDebug("\nezspSendManyToOneRouteRequest\n");
	ezspSendManyToOneRouteRequest(EMBER_LOW_RAM_CONCENTRATOR,EMBER_AF_PLUGIN_CONCENTRATOR_MAX_HOPS);
	//ezspSendManyToOneRouteRequest(EMBER_HIGH_RAM_CONCENTRATOR,EMBER_AF_PLUGIN_CONCENTRATOR_MAX_HOPS);
	
}


#define USE_NCP_ZAP_PORT
#ifdef USE_NCP_ZAP_PORT
uint8_t zapFormNetwork(const char *netType,
							uint16_t panId,
							int8_t radioTxPower,
							uint8_t radioChannel)
{
    EmberNetworkParameters networkParameters;
    EmberStatus status;
    uint8_t channeldefault = (halCommonGetRandom() & 0x0F) + EMBER_MIN_802_15_4_CHANNEL_NUMBER;
	uint16_t panIddefault = halCommonGetRandom();
    int32u channelMask = ZIGBEE_PRIMARY_CHANNEL_MASK;

	LogDebug("%s panId = %d,radioTxPower = %d,radioChannel = %d\n", __FUNCTION__,panId,radioTxPower,radioChannel);

	if(netType == NULL)
		{LogDebug("%s %d\n", __FUNCTION__,__LINE__);return 0;}
	if(radioTxPower > 100)
		{LogDebug("%s %d\n", __FUNCTION__,__LINE__);return 0;}
	if((radioChannel<EMBER_MIN_802_15_4_CHANNEL_NUMBER)||(radioChannel>EMBER_MAX_802_15_4_CHANNEL_NUMBER))
	{
		if(radioChannel != 0xff)
			{LogDebug("%s %d\n", __FUNCTION__,__LINE__);return 0;}
	}
	LogDebug("%s params ok\n", __FUNCTION__);
	
    status = EMBER_ERR_FATAL;
    while (channelMask && status != EMBER_SUCCESS) {
        // Find the next channel on which to try forming a network.
        while (!(channelMask & BIT32(channeldefault)))
        {
            channeldefault = (channeldefault == EMBER_MAX_802_15_4_CHANNEL_NUMBER
                       ? EMBER_MIN_802_15_4_CHANNEL_NUMBER
                       : channeldefault + 1);
        }
		if(panId == 0xffff)
		{
			LogDebug("%s %d\n", __FUNCTION__,__LINE__);
			networkParameters.panId = panIddefault;
		}			
		else
		{
			LogDebug("%s %d\n", __FUNCTION__,__LINE__);
			networkParameters.panId = panId;
		}

		if((uint8_t)radioTxPower == 0xff)
		{
			LogDebug("%s %d\n", __FUNCTION__,__LINE__);
			networkParameters.radioTxPower = ZIGBEE_NETWORK_CREATOR_RADIO_POWER;
		}			
		else
		{
			LogDebug("%s %d\n", __FUNCTION__,__LINE__);
			networkParameters.radioTxPower = radioTxPower;
		}

		if(radioChannel == 0xff)
		{
			LogDebug("%s %d\n", __FUNCTION__,__LINE__);
			networkParameters.radioChannel = (int8_t)channeldefault;
		}			
		else
		{
			LogDebug("%s %d\n", __FUNCTION__,__LINE__);
			networkParameters.radioChannel = (int8_t)radioChannel;
		}	

        zaTrustCenterSecurityInit(FALSE);

        status = ezspFormNetwork(&networkParameters);
        LogDebug("Form. Channel: 0x%x. Status: 0x%x. panId: 0x%x\n", networkParameters.radioChannel, status,networkParameters.panId);


        // Clear the channel bit that you just tried.
        channelMask &=~BIT32(channeldefault);
    }

    return 1;
}
							
uint8_t zapLeaveNetwork(const char *netType)
{
	EmberStatus status;
	LogDebug("%s \n", __FUNCTION__);
	status = ezspLeaveNetwork();
	
	if(status != EMBER_SUCCESS)
		return 0;
	return 1;
}

uint8_t zapPermitJion(uint8_t seconds)
{
	EmberStatus status;
    uint8_t secPermitJoinsWindow = seconds;

	LogDebug("%s AllowJoin for %d seconds\n", __FUNCTION__,secPermitJoinsWindow);
    status = ezspPermitJoining(secPermitJoinsWindow);
    //permitJoinsTimer = 0;
    //trustCenter.trustCenterPermitJoins(true);
    if(status != EMBER_SUCCESS)
		return 0;
	return 1;
}

uint8_t zapRemoveDevice(uint8_t ieee[8])
{
	EmberStatus status;
	
	EmberNodeId destShort;
	EmberEUI64 destLong;
	EmberEUI64 targetLong;

	LogDebug("%s \n", __FUNCTION__);
	destShort = ezspLookupNodeIdByEui64(ieee);	
	memcpy(destLong,ieee,EUI64_SIZE);
	memcpy(targetLong,ieee,EUI64_SIZE);
	status = ezspRemoveDevice(destShort,destLong,targetLong);

	if(status != EMBER_SUCCESS)
		return 0;
	return 1;
}

uint8_t zapSetChannel(uint8_t channel)
{
	EmberStatus status;
	
	LogDebug("%s \n", __FUNCTION__);
	status = ezspSetRadioChannel(channel);
	
	if(status != EMBER_SUCCESS)
		return 0;
	return 1;
}

/*å‡ºé”™è¿”å›ž0xFFï¼ŒæˆåŠŸè¿”å›žmessageTagç”¨äºŽconfirmï¼Œå¦‚æžœä¸éœ€è¦confirmåˆ™è¿”ï¿?*/
uint8_t zapSendUnicast(uint8_t ieee[8],
							uint16_t profileId,
                      		uint16_t clusterId,
                      		uint8_t sourceEndpoint,
                      		uint8_t destinationEndpoint,
                      		uint8_t messageLength,
                      		uint8_t* message,
                      		uint8_t needConfirm)
{
	EmberNodeId destination;
    EmberStatus status;
    EmberApsFrame apsFrame;
    uint8_t messageTag = 0;

	LogDebug("%s \n", __FUNCTION__);
	
	destination = ezspLookupNodeIdByEui64(ieee);
	
    apsFrame.profileId = profileId;
    apsFrame.clusterId = clusterId;
    apsFrame.sourceEndpoint = sourceEndpoint;
    apsFrame.destinationEndpoint = destinationEndpoint;
    apsFrame.options = C4GetSendOptions(destination, profileId, clusterId, messageLength);

    LogDebug("unicast to dst 0x%04x, profile 0x%04x, cluster 0x%04x, SE/DE %d/%d len=%d\r\n",
             destination,
             apsFrame.profileId,
             apsFrame.clusterId,
             apsFrame.sourceEndpoint,
             apsFrame.destinationEndpoint,
             messageLength);

    if(needConfirm)
    {
        messageTag = ezspNextSequence();
    }


    status = emberAfEzspSetSourceRoute(destination);
    if (status == EMBER_SUCCESS) {
        status = ezspSendUnicast(EMBER_OUTGOING_DIRECT,
                                 destination,
                                 &apsFrame,
                                 messageTag,
                                 messageLength,
                                 message,
                                 &apsFrame.sequence);
    }

    if (status == EMBER_SUCCESS)
    {
    	return 1;
        //return messageTag;
    }
    else
    {
    	LogDebug("ezspSendUnicast = err else\n");
        //return 0xFF;
		return 0;
    }
}
#endif

//#define USE_NCP_NOTIFY
#ifdef USE_NCP_NOTIFY
void zapSentNotify(uint8_t ieee[8],
						uint16_t profileId,
                  		uint16_t clusterId,
                  		uint8_t sourceEndpoint,
                  		uint8_t destinationEndpoint,
                  		uint8_t messageLength,
                  		uint8_t* message)
{
	LogDebug("%s\n",__FUNCTION__);
	LogHexDump(LOG_INFO, "ieee", ieee, 8);
	LogDebug("profileId = 0x%04x\n",profileId);
	LogDebug("clusterId = 0x%04x\n",clusterId);
	LogDebug("sourceEndpoint = %d\n",sourceEndpoint);
	LogDebug("destinationEndpoint = %d\n",destinationEndpoint);
	LogDebug("messageLength = %d\n",messageLength);
	LogHexDump(LOG_INFO, "message", message, messageLength);
}

void zapNewPacketNotify(uint8_t ieee[8],
								uint16_t profileId,
								uint16_t clusterId,
								uint8_t sourceEndpoint,
								uint8_t destinationEndpoint,
								uint8_t messageLength,
								uint8_t* message)
{
	LogDebug("%s\n",__FUNCTION__);
	LogHexDump(LOG_INFO, "ieee", ieee, 8);
	LogDebug("profileId = 0x%04x\n",profileId);
	LogDebug("clusterId = 0x%04x\n",clusterId);
	LogDebug("sourceEndpoint = %d\n",sourceEndpoint);
	LogDebug("destinationEndpoint = %d\n",destinationEndpoint);
	LogDebug("messageLength = %d\n",messageLength);
	LogHexDump(LOG_INFO, "message", message, messageLength);
}

void zapNodeNetworkStatusNotify(uint8_t status,
											uint8_t ieee[8],
											const char *netType,
											uint8_t deviceType,
											int8_t rssi,
											uint8_t lqi,
											char *firmwareVer,
											char *productString)
{
	LogDebug("%s\n",__FUNCTION__);
	LogDebug("status = %d\n",status);
	LogDebug("deviceType = %d\n",deviceType);
	LogDebug("rssi = %d\n",rssi);
	LogDebug("lqi = %d\n",lqi);
	if(ieee)
		LogHexDump(LOG_INFO, "ieee", ieee, 8);
	if(netType)
		LogDebug("netType = %s\n",netType);
	if(firmwareVer)
		LogDebug("firmwareVer = %s\n",firmwareVer);
	if(productString)
		LogDebug("productString = %s\n",productString);	
}

void zapNetworkStatusNotify(uint8_t status)
{
	LogDebug("%s\n",__FUNCTION__);
	LogDebug("status = %d\n",status);
}
#endif
