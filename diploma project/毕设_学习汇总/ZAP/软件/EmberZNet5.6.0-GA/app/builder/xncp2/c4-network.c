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
#include "zcl-parse.h"



uint8_t ZigbeeUnicast(int16u destination,
                      int16u profileId,
                      int16u clusterId,
                      int8u sourceEndpoint,
                      int8u destinationEndpoint,
                      int8u messageLength,
                      int8u* message,
                      bool_t needConfirm);



DEFINE_LOGGER(C4, 128, LOG_DEBUG);
//#define ZCL_MAX_PAYLOAD_SIZE    73

void C4_DEVICE_TYPE(void * para,uint8_t type,uint8_t *data,void *stjoin)
{
	//C4ZclPara_ST * c4Para = (C4ZclPara_ST *)para;
	NodeNetworkStatus_ST *ptnode = (NodeNetworkStatus_ST *)stjoin;
	
	LogDebug("%s start\n",__FUNCTION__);
	/////////////////////////////////////////////////////	
	ptnode->deviceType = *data;
	ptnode->ready |= DVTYPE_OTTSET_BIT;
	////////////////////////////////////////////////////	
	LogDebug("%s end\n",__FUNCTION__);	
}
void C4_SSCP_ANNOUNCE_WINDOW(void * para,uint8_t type,uint8_t *data,void *stjoin)
{
	LogDebug("%s\n",__FUNCTION__);
}
void C4_SSCP_MTORR_PERIOD(void * para,uint8_t type,uint8_t *data,void *stjoin)
{
	LogDebug("%s\n",__FUNCTION__);
}
void C4_SSCP_NUM_ZAPS(void * para,uint8_t type,uint8_t *data,void *stjoin)
{
	LogDebug("%s\n",__FUNCTION__);
}
void C4_FIRMWARE_VERSION(void * para,uint8_t type,uint8_t *data,void *stjoin)
{
	//C4ZclPara_ST * c4Para = (C4ZclPara_ST *)para;
	NodeNetworkStatus_ST *ptnode = (NodeNetworkStatus_ST *)stjoin;
	
	LogDebug("%s start\n",__FUNCTION__);	
	////////////////////////////////////////////////////
	uint8_t pslen = 0;
	pslen = *data++;
	if(pslen > 16)
	{
		LogDebug("%s err1\n",__FUNCTION__);
		return;
	}
	memcpy(ptnode->firmwareVer,data,pslen);
	ptnode->firmwareVer[pslen] = 0;
	ptnode->ready |= FVER_OTTSET_BIT;
	////////////////////////////////////////////////////
	LogDebug("%s end\n",__FUNCTION__);
}
void C4_REFLASH_VERSION(void * para,uint8_t type,uint8_t *data,void *stjoin)
{
	LogDebug("%s\n",__FUNCTION__);
}
void C4_BOOT_COUNT(void * para,uint8_t type,uint8_t *data,void *stjoin)
{
	LogDebug("%s\n",__FUNCTION__);
}
void C4_PRODUCT_STRING(void * para,uint8_t type,uint8_t *data,void *stjoin)//0x0007
{
	//C4ZclPara_ST * c4Para = (C4ZclPara_ST *)para;
	NodeNetworkStatus_ST *ptnode = (NodeNetworkStatus_ST *)stjoin;	
	
	LogDebug("%s start\n",__FUNCTION__);
	////////////////////////////////////////////////////
	uint8_t pslen = 0;
	pslen = *data++;
	if(pslen > 24)
	{
		LogDebug("%s err1\n",__FUNCTION__);
		return;
	}
	memcpy(ptnode->productString,data,pslen);
	ptnode->productString[pslen] = 0;
	ptnode->ready |= PDS_OTTSET_BIT;
	////////////////////////////////////////////////////
	LogDebug("%s end\n",__FUNCTION__);
}
void C4_ACCESS_POINT_NODE_ID(void * para,uint8_t type,uint8_t *data,void *stjoin)
{
	LogDebug("%s\n",__FUNCTION__);
}
void C4_ACCESS_POINT_LONG_ID(void * para,uint8_t type,uint8_t *data,void *stjoin)
{
	LogDebug("%s\n",__FUNCTION__);
}
void C4_ACCESS_POINT_COST(void * para,uint8_t type,uint8_t *data,void *stjoin)
{
	LogDebug("%s\n",__FUNCTION__);
}
void C4_END_NODE_ACCESS_POINT_POLL_PERIOD(void * para,uint8_t type,uint8_t *data,void *stjoin)
{
	LogDebug("%s\n",__FUNCTION__);
}

void C4_SSCP_CHANNEL(void * para,uint8_t type,uint8_t *data,void *stjoin)
{
	LogDebug("%s\n",__FUNCTION__);
}
void C4_ZSERVER_IP(void * para,uint8_t type,uint8_t *data,void *stjoin)
{
	LogDebug("%s\n",__FUNCTION__);
}
void C4_ZSERVER_HOST_NAME(void * para,uint8_t type,uint8_t *data,void *stjoin)
{
	LogDebug("%s\n",__FUNCTION__);
}
void C4_END_NODE_PARENT_POLL_PERIOD(void * para,uint8_t type,uint8_t *data,void *stjoin)
{
	LogDebug("%s\n",__FUNCTION__);
}
void C4_MESH_LONG_ID(void * para,uint8_t type,uint8_t *data,void *stjoin)
{
	LogDebug("%s\n",__FUNCTION__);
}
void C4_MESH_SHORT_ID(void * para,uint8_t type,uint8_t *data,void *stjoin)
{
	LogDebug("%s\n",__FUNCTION__);
}
void C4_AP_TABLE(void * para,uint8_t type,uint8_t *data,void *stjoin)
{
	LogDebug("%s\n",__FUNCTION__);
}
void C4_AVG_RSSI(void * para,uint8_t type,uint8_t *data,void *stjoin)
{
	//C4ZclPara_ST * c4Para = (C4ZclPara_ST *)para;
	NodeNetworkStatus_ST *ptnode = (NodeNetworkStatus_ST *)stjoin;
	
	LogDebug("%s start\n",__FUNCTION__);
	/////////////////////////////////////////////////////	
	ptnode->rssi = *data;
	ptnode->ready |= RSSI_OTTSET_BIT;
	////////////////////////////////////////////////////	
	LogDebug("%s end\n",__FUNCTION__);
}
void C4_AVG_LQI(void * para,uint8_t type,uint8_t *data,void *stjoin)
{
	//C4ZclPara_ST * c4Para = (C4ZclPara_ST *)para;
	NodeNetworkStatus_ST *ptnode = (NodeNetworkStatus_ST *)stjoin;
	
	LogDebug("%s start\n",__FUNCTION__);
	/////////////////////////////////////////////////////	
	ptnode->lqi = *data;	
	ptnode->ready |= LQI_OTTSET_BIT;
	////////////////////////////////////////////////////	
	LogDebug("%s end\n",__FUNCTION__);	
}
void C4_DEVICE_BATTERY_LEVEL(void * para,uint8_t type,uint8_t *data,void *stjoin)
{
	LogDebug("%s\n",__FUNCTION__);
}
void C4_RADIO_4_BARS(void * para,uint8_t type,uint8_t *data,void *stjoin)
{
	LogDebug("%s\n",__FUNCTION__);
}

attributesIdentifierHandle attribIdHandle[0x17] = 
{
	C4_DEVICE_TYPE,//0x0000
	C4_SSCP_ANNOUNCE_WINDOW,//0x0001
	C4_SSCP_MTORR_PERIOD,//0x0002
	C4_SSCP_NUM_ZAPS,//0x0003
	C4_FIRMWARE_VERSION,//0x0004
	C4_REFLASH_VERSION,//0x0005
	C4_BOOT_COUNT,//0x0006
	C4_PRODUCT_STRING,//0x0007
	C4_ACCESS_POINT_NODE_ID,//0x0008
	C4_ACCESS_POINT_LONG_ID,//0x0009
	C4_ACCESS_POINT_COST,//0x000a
	C4_END_NODE_ACCESS_POINT_POLL_PERIOD,//0x000b
	C4_SSCP_CHANNEL,//0x000c
	C4_ZSERVER_IP,//0x000d
	C4_ZSERVER_HOST_NAME,//0x000e
	C4_END_NODE_PARENT_POLL_PERIOD,//0x000f
	C4_MESH_LONG_ID,//0x0010
	C4_MESH_SHORT_ID,//0x0011
	C4_AP_TABLE,//0x0012
	C4_AVG_RSSI,//0x0013
	C4_AVG_LQI,//0x0014
	C4_DEVICE_BATTERY_LEVEL,//0x0015
	C4_RADIO_4_BARS,//0x0016
};


//C4 attribute ID
#define C4_ATTR_DEVICE_TYPE						    0
#define C4_ATTR_SSCP_ANNOUNCE_WINDOW				1
#define C4_ATTR_SSCP_MTORR_PERIOD					2
#define C4_ATTR_SSCP_NUM_ZAPS						3
#define C4_ATTR_FIRMWARE_VERSION					4
#define C4_ATTR_REFLASH_VERSION					    5
#define C4_ATTR_BOOT_COUNT						    6
#define C4_ATTR_PRODUCT_STRING					    7
#define C4_ATTR_ACCESS_POINT_NODE_ID				8
#define C4_ATTR_ACCESS_POINT_LONG_ID				9
#define C4_ATTR_ACCESS_POINT_COST					10
#define C4_ATTR_END_NODE_ACCESS_POINT_POLL_PERIOD	11
#define C4_ATTR_SSCP_CHANNEL						12
#define C4_ATTR_ZSERVER_IP						    13
#define C4_ATTR_ZSERVER_HOST_NAME					14
#define C4_ATTR_END_NODE_PARENT_POLL_PERIOD		    15
#define C4_ATTR_MESH_LONG_ID                        16
#define C4_ATTR_MESH_SHORT_ID                       17
#define C4_ATTR_AP_TABLE                            18
#define C4_ATTR_AVG_RSSI                            19
#define C4_ATTR_AVG_LQI                             20
#define C4_ATTR_DEVICE_BATTERY_LEVEL                21
#define C4_ATTR_RADIO_4_BARS                        22


static uint16_t shortID = 0x0000;
static ZclAttrib_ST c4_clust_attrs[1] = 
{
	{ C4_ATTR_ACCESS_POINT_NODE_ID,ZCL_TYPE_U16,ZCL_ACCESS_READ_ONLY,(void *)&shortID },
};

ZclAttrib_ST *C4FindAttrib(void * para, uint16_t id);
void C4SaveAttrib(void * para, uint16_t id, uint8_t type, uint8_t *data);
void C4RecordAttrib(void * para, uint16_t id, uint8_t type, uint8_t *data,void *stjoin);
uint8_t C4SpecCommand(void * para, uint8_t *resp, uint8_t respMaxLen, ZclHdr_ST *hdr);
ZclHandler_ST zclHandle = 
{
    C4FindAttrib,C4SaveAttrib,C4RecordAttrib,C4SpecCommand
};

ZclAttrib_ST *C4FindAttrib(void * para, uint16_t id)
{
	//C4ZclPara_ST * p = (C4ZclPara_ST *)para;
	LogDebug("%s %d\n",__FUNCTION__,__LINE__);

	
	return &c4_clust_attrs[0];
}

void C4SaveAttrib(void * para, uint16_t id, uint8_t type, uint8_t *data)
{
    C4ZclPara_ST * p = (C4ZclPara_ST *)para;
	
    LogDebug("C4SaveAttrib clust 0x%04x attrib 0x%04x type 0x%x\n", p->clust_id, id, type);
}
void C4RecordAttrib(void * para, uint16_t id, uint8_t type, uint8_t *data,void *stjoin)
{
    C4ZclPara_ST * p = (C4ZclPara_ST *)para;
	
    LogDebug("C4RecordAttrib clust 0x%04x attrib 0x%04x type 0x%x\n", p->clust_id, id, type);
	attribIdHandle[id](para,type,data,stjoin);	
}

uint8_t C4SpecCommand(void * para, uint8_t *resp, uint8_t respMaxLen, ZclHdr_ST *hdr)
{
    //C4ZclPara_ST * p = (C4ZclPara_ST *)para;
	return 1;
}


uint16_t C4GetSendOptions(int16u destination, int16u profileId, int16u clusterId, int8u messageLength)
{
    uint16_t options = 0;
    if((profileId == 0xc25d) && (clusterId == 0x0001))
    {
          if (destination >= EMBER_BROADCAST_ADDRESS)
          {
		    options = /*EMBER_APS_OPTION_RETRY |*/ EMBER_APS_OPTION_SOURCE_EUI64;
          }
          else
          {
                options = EMBER_APS_OPTION_RETRY | EMBER_APS_OPTION_ENABLE_ROUTE_DISCOVERY | EMBER_APS_OPTION_ENABLE_ADDRESS_DISCOVERY | EMBER_APS_OPTION_SOURCE_EUI64 | EMBER_APS_OPTION_DESTINATION_EUI64;
          }
	}else{
        /*配置数据较长，在长度溢出的情况下，通过不带MAC的方式多传8个字节*/
        if(messageLength <= 66)/*66不溢出*/
        {
            options = /*EMBER_APS_OPTION_RETRY | */EMBER_APS_OPTION_SOURCE_EUI64 | EMBER_APS_OPTION_DESTINATION_EUI64;
        }
        else if(messageLength <= 74)/*67~74*/
        {
            options = /*EMBER_APS_OPTION_RETRY | */EMBER_APS_OPTION_SOURCE_EUI64;
        }
        else
        {
            options = 0;
        }
	}
    return options;
}

void C4IncomingMessageHandler(EmberIncomingMessageType type,
                                EmberApsFrame *apsFrame,
                                int8u lastHopLqi,
                                int8s lastHopRssi,
                                EmberNodeId sender,
                                int8u bindingIndex,
                                int8u addressIndex,
                                int8u messageLength,
                                int8u *messageContents)
{
    ZclHdr_ST hdr;
    C4ZclPara_ST zclPara;
    uint8_t resp[ZCL_MAX_PAYLOAD_SIZE];
    uint8_t respLen;
    uint8_t sendReturn;

    zclPara.clust_id = apsFrame->clusterId;
    zclPara.remote_addr = sender;
    zclPara.remote_ep = apsFrame->sourceEndpoint;
    zclPara.local_ep = apsFrame->destinationEndpoint;

    if((apsFrame->profileId == 0xc25d) && (apsFrame->clusterId == 0x0001))
    {
    /////////////////////////////////////zhouyong////////////////////////////
    	if(type == EMBER_INCOMING_BROADCAST)
		{
			ezspSendManyToOneRouteRequest(EMBER_LOW_RAM_CONCENTRATOR, 
                                    	EMBER_AF_PLUGIN_CONCENTRATOR_MAX_HOPS);
			LogDebug("ezspSendManyToOneRouteRequest\n");
		}

	/////////////////////////////////////////////////////////////////
        if (0 != ZclParseHeader(messageContents, messageLength, &hdr))
        {
            LogError("ZCL header error\n");
        }

		LogDebug("%s hdr->frm_ctrl.frm_type = %d\n",__FUNCTION__,hdr.frm_ctrl.frm_type);
		LogDebug("%s hdr->frm_ctrl.manuf_spec = %d\n",__FUNCTION__,hdr.frm_ctrl.manuf_spec);
		LogDebug("%s hdr->frm_ctrl.dir = %d\n",__FUNCTION__,hdr.frm_ctrl.dir);
		LogDebug("%s hdr->frm_ctrl.dis_def_resp = %d\n",__FUNCTION__,hdr.frm_ctrl.dis_def_resp);

		LogDebug("%s hdr->fcf = %d\n",__FUNCTION__,hdr.fcf);
		LogDebug("%s hdr->manuf_code = %d\n",__FUNCTION__,hdr.manuf_code);
		LogDebug("%s hdr->seq_num = %d\n",__FUNCTION__,hdr.seq_num);
		LogDebug("%s hdr->cmd = 0x%02x\n",__FUNCTION__,hdr.cmd);
		
        respLen = Zcl_ParseFrame(&zclHandle, &zclPara, resp, sizeof(resp), &hdr);
		////////////
		#if 0
		if(type == EMBER_INCOMING_BROADCAST)
		//if(0)
		{
			LogDebug("%s %d\n",__FUNCTION__,__LINE__);
			////////////
			respLen = 0;
			resp[respLen++] = 0x01;
			resp[respLen++] = zcl_get_sequence();
			resp[respLen++] = 0x00;
			
			resp[respLen++] = 0x01;
			resp[respLen++] = (uint8_t)sender;
			resp[respLen++] = (uint8_t)(sender >> 8);
			////////////
		}
		////////////
		#endif
        if(respLen > 0)
        {
			sendReturn = ZigbeeUnicast(sender,
			                      apsFrame->profileId,
			                      apsFrame->clusterId,
			                      apsFrame->destinationEndpoint,
			                      apsFrame->sourceEndpoint,
			                      respLen,
			                      resp,
			                      FALSE); /*src和dest端点互换*/
			LogDebug("%s %d\n",__FUNCTION__,__LINE__);

            if (0 != sendReturn)
            {
                LogError("ZigbeeUnicast error\r\n");
            }
        }


    }
    
}