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


#include "Platform.h"
#include "Logger.h"
#include "zcl-parse.h"



void zapNodeNetworkStatusNotify(uint8_t status,
											uint8_t ieee[8],
											const char *netType,
											uint8_t deviceType,
											int8_t rssi,
											uint8_t lqi,
											char *firmwareVer,
											char *productString);


DEFINE_LOGGER(ZCL, 128, LOG_DEBUG);

#if 0
static char halfbyteTohex(uint8_t byte)
{
	if(byte < 0x0a)
		return ('0' + byte);
	else if((byte > 0x09)&&(byte < 0x10))
		return ('a' + byte - 0x0a);
	else
		return 0;
}

static void bytearrTohexstring(uint8_t *barr,uint8_t arrlen,char *str)
{
	uint8_t icnt = 0;
	uint8_t tmp = 0;
	uint8_t strcnt = 0;
	
	for(icnt = 0;icnt < arrlen;icnt++)
	{
		tmp = barr[arrlen-1-icnt];

		str[strcnt++] = halfbyteTohex((tmp>>4)&0x0f);
		str[strcnt++] = halfbyteTohex(tmp&0x0f);
	}
	str[strcnt] = 0;
}
#endif


NodeNetworkStatus_ST stNodeStatus[NODE_STATUS_BUF_SIZE];
void NodeStatusInit(void)
{
	uint8_t icnt = 0;

	for(icnt = 0;icnt < NODE_STATUS_BUF_SIZE;icnt++)
	{
		stNodeStatus[icnt].userd = 0;
		stNodeStatus[icnt].nodeId = 0;
		stNodeStatus[icnt].ready = 0;
	}
}

void clearNodeBuf(uint16_t shortid)

{
	uint8_t icnt = 0;
	
	for(icnt = 0;icnt < NODE_STATUS_BUF_SIZE;icnt++)
	{
		if(stNodeStatus[icnt].userd == 1)
		{
			if(stNodeStatus[icnt].nodeId == shortid)
			{
				stNodeStatus[icnt].userd = 0;
			}
		}			
	}
}

NodeNetworkStatus_ST *getEmptyNodeBuf(void)
{
	uint8_t icnt = 0;
	for(icnt = 0;icnt < NODE_STATUS_BUF_SIZE;icnt++)
	{
		if(stNodeStatus[icnt].userd == 0)
		{
			memset(&stNodeStatus[icnt],0x00,sizeof(NodeNetworkStatus_ST));		
			return (NodeNetworkStatus_ST *)&stNodeStatus[icnt];
		}			
	}
	return NULL;
}
NodeNetworkStatus_ST *seekNodeBuf(uint16_t shortid)
{
	uint8_t icnt = 0;

	for(icnt = 0;icnt < NODE_STATUS_BUF_SIZE;icnt++)
	{
		if(stNodeStatus[icnt].userd == 1)
		{
			if(stNodeStatus[icnt].nodeId == shortid)
			{
				return (NodeNetworkStatus_ST *)&stNodeStatus[icnt];
			}
		}			
	}
	return NULL;
}

/**************************************************************************/
/*!
 Parse the ZCL into a structure that makes it easier for us to process.
 */
/**************************************************************************/
int32_t ZclParseHeader(uint8_t *data, uint8_t len, ZclHdr_ST *hdr)
{
    uint8_t *data_ptr;

    if ((NULL == data) ||  (NULL == hdr))
    {
        LogError("NULL pointer\n");
        return -1;
    }
    if (len < 3)
    {
        LogError("length less than ZCL header\r\n");
        return -1;
    }

    data_ptr = data;

    // get the fcf for further decoding
    hdr->fcf = *data_ptr++;

    // decode the fcf and fill out the frm ctrl structure
    hdr->frm_ctrl.frm_type = (hdr->fcf & ZCL_FRM_TYPE_MASK) >> ZCL_FRM_TYPE_OFF;
    hdr->frm_ctrl.manuf_spec = (hdr->fcf & ZCL_MANUF_SPEC_MASK) >> ZCL_MANUF_SPEC_OFF;
    hdr->frm_ctrl.dir = (hdr->fcf & ZCL_DIR_MASK) >> ZCL_DIR_OFF;
    hdr->frm_ctrl.dis_def_resp = (hdr->fcf & ZCL_DIS_DEF_MASK) >> ZCL_DIS_DEF_OFF;

    // get the rest of the fields
    if (hdr->frm_ctrl.manuf_spec)
    {
        if (len < 5)
        {
            return -1;
        }
        hdr->manuf_code = *(uint16_t *) data_ptr;
        data_ptr += sizeof(uint16_t);
    }

    hdr->seq_num = *data_ptr++;
    hdr->cmd = *data_ptr++;
    hdr->payload = data_ptr;
    hdr->payload_len = len - (data_ptr - data);
    return 0;
}



/**************************************************************************/
/*!
 Generate the ZCL frame control field.
 */
/**************************************************************************/
static uint8_t zcl_gen_fcf(ZclHdr_ST *hdr)
{
    uint8_t fcf = 0;

    fcf |= (hdr->frm_ctrl.frm_type & 0x3);
    fcf |= (hdr->frm_ctrl.manuf_spec & 0x1) << ZCL_MANUF_SPEC_OFF;
    fcf |= (hdr->frm_ctrl.dir & 0x1) << ZCL_DIR_OFF;
    fcf |= (hdr->frm_ctrl.dis_def_resp & 0x1) << ZCL_DIS_DEF_OFF;
    return fcf;
}


static uint8_t zcl_get_seq_num(void)
{
	static uint8_t zcl_sequence = 0;
	return zcl_sequence++;
}


/**************************************************************************/
/*!
 Generate the ZCL frame header. The empty data array that will be used for
 the command gets passed in and it will be filled out. The size of the
 header gets passed back to the caller.
 生成zcl的header，传入缓冲区指针，和zcl header结构体，返回偏移了header长度后的缓冲区新的位置，用于填充zcl body
 */
/**************************************************************************/
uint8_t * zcl_gen_hdr(uint8_t *data, ZclHdr_ST *hdr)
{
    *data++ = zcl_gen_fcf(hdr);

    if (hdr->frm_ctrl.manuf_spec)
    {
        *(uint16_t *) data = hdr->manuf_code;
        data += sizeof(uint16_t);
    }

    *data++ = hdr->seq_num;
    *data++ = hdr->cmd;

    return data;
}


/**************************************************************************/
/*!
 Decodes the attribute size and returns the size in bytes. This will
 be used when calculating the length of the following attribute value for
 either decoding of a ZCL frame or generating a ZCL response.
 */
/**************************************************************************/
uint8_t zcl_get_attrib_size(uint8_t type, void * data)
{
    switch (type)
    {
    case ZCL_TYPE_8BIT:
    case ZCL_TYPE_BOOL:
    case ZCL_TYPE_8BITMAP:
    case ZCL_TYPE_U8:
    case ZCL_TYPE_S8:
    case ZCL_TYPE_ENUM8:
        return 1;

    case ZCL_TYPE_16BIT:
    case ZCL_TYPE_16BITMAP:
    case ZCL_TYPE_U16:
    case ZCL_TYPE_S16:
    case ZCL_TYPE_ENUM16:
    case ZCL_TYPE_CLUST_ID:
    case ZCL_TYPE_ATTRIB_ID:
        return 2;

    case ZCL_TYPE_32BIT:
    case ZCL_TYPE_32BITMAP:
    case ZCL_TYPE_U32:
    case ZCL_TYPE_S32:
    case ZCL_TYPE_TIME_OF_DAY:
    case ZCL_TYPE_DATE:
    case ZCL_TYPE_UTCTIME:
        return 4;

    case ZCL_TYPE_BYTE_ARRAY:
    case ZCL_TYPE_CHAR_STRING:
        return (*(uint8_t *) data + 1);

    case ZCL_TYPE_64BIT:
    case ZCL_TYPE_64BITMAP:
    case ZCL_TYPE_U64:
    case ZCL_TYPE_S64:
    case ZCL_TYPE_IEEE_ADDR:
        return 8;

    case ZCL_TYPE_SEC_KEY:
        return 16;

    case ZCL_TYPE_LONG_BYTE_ARRAY:
    case ZCL_TYPE_LONG_CHAR_STRING:
        return (*(uint16_t *) data + 2); 

	

    case ZCL_TYPE_24BIT:
    case ZCL_TYPE_24BITMAP:
    case ZCL_TYPE_U24:
    case ZCL_TYPE_S24:
        return 3;

    case ZCL_TYPE_40BIT:
    case ZCL_TYPE_40BITMAP:
    case ZCL_TYPE_U40:
    case ZCL_TYPE_S40:
        return 5;

    case ZCL_TYPE_48BIT:
    case ZCL_TYPE_48BITMAP:
    case ZCL_TYPE_U48:
    case ZCL_TYPE_S48:
        return 6;

    case ZCL_TYPE_56BIT:
    case ZCL_TYPE_56BITMAP:
    case ZCL_TYPE_U56:
    case ZCL_TYPE_S56:
        return 7;

    case ZCL_TYPE_NULL:
    case ZCL_TYPE_INVALID:
        return 0;

    default:
        LogError("unsupported ZCL type 0x%x\r\n", type);
        return 0xff;
    }
}


/**************************************************************************/
/*!
 Generate the ZCL Read Attributes command. An empty array is passed in
 and it gets filled out in the function. An attribute ID list and the number
 of attributes is required to be passed into the function. Only the ID from
 the attrib list is used and are used to specify the attributes that are to be read.

 The command data is contained in the zcl header that's passed in. When
 finished, the function will return the length of the data.
 */
/**************************************************************************/
uint8_t zcl_gen_read_attrib(uint8_t *data, uint8_t maxLen, ZclHdr_ST *hdr, uint16_t *attrib_list, uint8_t attrib_num)
{
    uint8_t i, *data_ptr;

    // gen the zcl frame header
    data_ptr = zcl_gen_hdr(data, hdr);

    // gen the payload
    for (i = 0; i < attrib_num; i++)
    {
        if (data_ptr - data + 2 > maxLen)
        {
            LogError("attribute 0x%2x request buffer insuffcient\r\n", attrib_list[i]);
            break;
        }
        else
        {
            *(uint16_t *) data_ptr = attrib_list[i];
            data_ptr += sizeof(uint16_t);
        }
    }

    return (data_ptr - data);
}

/**************************************************************************/
/*!
 Generate the ZCL Write attributes command. The length of the write attribute
 command frame gets passed back to the caller. An attrib list and number of
 attributes needs to be passed into the function. The attrib IDs and values will
 be used from the attrib list and will specify the attrib ID to be written to
 as well as the value to write.
 */
/**************************************************************************/
uint8_t zcl_gen_write_attrib(uint8_t *data, uint8_t maxLen, ZclHdr_ST *hdr, ZclAttrib_ST *attrib_list, uint8_t attrib_num)
{
    uint8_t i, attrib_len, *data_ptr;
    ZclAttrib_ST *attrib;

    // gen the zcl frame header
    data_ptr = zcl_gen_hdr(data, hdr);

    for (i = 0; i < attrib_num; i++)
    {
        attrib = &attrib_list[i];
        // get the len of the attribute
        attrib_len = zcl_get_attrib_size(attrib->type, attrib->data);
        if (attrib_len != 0xff)
        {
            if (data_ptr - data + attrib_len + 3 > maxLen)
            {
                LogError("attribute 0x%2x request buffer insuffcient\r\n", attrib->id);
                break;
            }
            else
            {
                // copy the attrib id and the attrib data type
                *(uint16_t *) data_ptr = attrib->id;
                data_ptr += sizeof(uint16_t);
                *data_ptr++ = attrib->type;

                // copy it into the data ptr
                memcpy(data_ptr, attrib->data, attrib_len);
                data_ptr += attrib_len;
            }
        }
    }

    return (data_ptr - data);
}

/**************************************************************************/
/*!
 Send the default response. This is the response when no other response
 is specified and will just mirror the command and the status. It can be
 suppressed by specifying to disable the default response in the incoming
 header.
 */
/**************************************************************************/
uint8_t zcl_gen_def_resp(uint8_t *resp, uint8_t status, ZclHdr_ST *hdr)
{
    uint8_t *resp_ptr;
    ZclHdr_ST resp_hdr;

    resp_ptr = resp;

    resp_hdr.frm_ctrl.dir = ZCL_FRM_DIR_TO_CLI;
    resp_hdr.frm_ctrl.dis_def_resp = TRUE;
    resp_hdr.frm_ctrl.frm_type = ZCL_FRM_TYPE_GENERAL;
    resp_hdr.frm_ctrl.manuf_spec = FALSE;
    resp_hdr.cmd = ZCL_CMD_DEFAULT_RESP;
    resp_hdr.seq_num = hdr->seq_num;

    resp_ptr = zcl_gen_hdr(resp_ptr, &resp_hdr);

    // payload is here. 1 byte for cmd, 1 byte for status
    *resp_ptr++ = hdr->cmd;
    *resp_ptr++ = status;

    return resp_ptr - resp;
}



/**************************************************************************/
/*!
 This function implements the ZCL read attribute foundation command.
 It can read either a single attribute or a list of attributes and returns
 a response of all the attribute values. The response can only be as long
 as the maximum application payload size.

 TODO: Rewrite this function to make it more concise
 */
/**************************************************************************/
static uint8_t zcl_cmd_read_attrib(ZclHandler_ST * handle, void *para, uint8_t *resp, uint8_t respMaxLen, ZclHdr_ST *hdr)
{
    uint8_t i, *resp_ptr, *data_ptr;
    ZclHdr_ST hdr_out;

    // save the original position so we can calculate the amount of data we wrote
    resp_ptr = resp;
    data_ptr = hdr->payload;

    // generate the header. most of the info is the same as the inbound hdr so do
    // a memcpy and just modify the parts that need to change
    memcpy(&hdr_out, hdr, sizeof(ZclHdr_ST));
    hdr_out.frm_ctrl.dir = ZCL_FRM_DIR_TO_CLI;
    hdr_out.cmd = ZCL_CMD_READ_ATTRIB_RESP;

    // this is where the header actually gets generated
    resp_ptr = zcl_gen_hdr(resp_ptr, &hdr_out);

    // now iterate over all the attributes that are requesting to be read
    for (i = 0; i < hdr->payload_len - 1; i += 2) /*-1是防止只剩单字节*/
    {
        uint8_t status, attrib_len;
        uint16_t attrib_id;
        ZclAttrib_ST *attrib;

        // init the status to an inital value of SUCCESS
        status = ZCL_STATUS_SUCCESS;

        // get the attrib id from the frame
        //attrib_id = *(uint16_t *) data_ptr;

		attrib_id = CHANGE16_LE(*(data_ptr),*(data_ptr+1));//soso
        data_ptr += sizeof(uint16_t);

        if ((resp_ptr - resp + 3) > respMaxLen)
        {
            LogError("attribute 0x%2x response buffer insuffcient\r\n", attrib_id);
            break;
        }
        else if ( (handle->findAttrib == NULL) || ((attrib = handle->findAttrib(para, attrib_id)) == NULL) )
        {
            status = ZCL_STATUS_UNSUP_ATTRIB;
            LogError("unsupported attribute 0x%2x\r\n", attrib_id);
            return 0;
        }
        else if ((attrib_len = zcl_get_attrib_size(attrib->type, attrib->data)) == 0xff)
        {
            LogError("unsupported type 0x%2x of attr 0x%2x\r\n", attrib->type, attrib_id);
            status = ZCL_STATUS_INVALID_TYPE;
        }
        // check to make sure there is enough space in the frame to include the attribute
        else if ((resp_ptr - resp + attrib_len + 4) > respMaxLen)
        {
            status = ZCL_STATUS_INSUFF_SPACE;
            LogError("attribute 0x%2x response buffer insuffcient for its data\r\n", attrib_id);
        }

        // attrib id
        //*(uint16_t *) resp_ptr = attrib_id;
		*resp_ptr = (uint8_t)attrib_id;
		*(resp_ptr+1) = (uint8_t)(attrib_id >> 8);
        resp_ptr += sizeof(uint16_t);
        *resp_ptr++ = status;

        // if we're out of room, then end the response frame here. otherwise, if there was a problem with the
        // attribute, then don't fill out the rest of the attrib fields and move on to the next one.
        if (status == ZCL_STATUS_INSUFF_SPACE)
        {
            break;
        }
        else if (status != ZCL_STATUS_SUCCESS)
        {
            continue;
        }


		//LogDebug("%s %d\n",__FUNCTION__,__LINE__);
        *resp_ptr++ = attrib->type;	
		//LogDebug("%s %d\n",__FUNCTION__,__LINE__);
		attrib_len = zcl_get_attrib_size(attrib->type, attrib->data);//soso		
        memcpy(resp_ptr, (uint8_t *) attrib->data, attrib_len);
        resp_ptr += attrib_len;
        LogDebug("read attribute 0x%2x type%x len=%d\r\n", attrib_id,attrib->type,attrib_len);
    }
	//LogDebug("%s %d\n",__FUNCTION__,__LINE__);
    // return the length of the response
    return (resp_ptr - resp);
}

static void zcl_cmd_read_attrib_resp(ZclHandler_ST * handle, void *para, uint8_t *resp, uint8_t respMaxLen, ZclHdr_ST *hdr)
{
    uint8_t *data_ptr;
	NodeNetworkStatus_ST *ptnode = NULL;
	
    // init the data
    data_ptr = hdr->payload;

    while ((data_ptr - hdr->payload + 3) <= hdr->payload_len) /*+3是确保有ID和status字段*/
    {
        uint8_t type, status, attrib_len;
        uint16_t attrib_id;

        //attrib_id = *(uint16_t *) data_ptr;

		//attrib_id = CHANGE16_LE(*data_ptr++,*data_ptr++);//soso
		attrib_id = CHANGE16_LE(*(data_ptr),*(data_ptr+1));//soso
		
        data_ptr += sizeof(uint16_t);
        status = *(uint8_t *) data_ptr++;

        if(status == ZCL_STATUS_SUCCESS)
        {
            if((data_ptr - hdr->payload + 2) <= hdr->payload_len) /*+2是确保有type和至少1字节的data字段*/
            {
                LogError("attribute 0x%2x type length overflow, reach %d, payload %d\r\n", attrib_id, data_ptr - hdr->payload, hdr->payload_len);
                break;
            }
            type = *(uint8_t *) data_ptr++;
            if ((attrib_len = zcl_get_attrib_size(type, data_ptr)) == 0xff)
            {
                LogError("unsupported type 0x%2x of attr 0x%2x\r\n", type, attrib_id);
                break;
            }

            if((data_ptr - hdr->payload + attrib_len) <= hdr->payload_len)
            {
                LogDebug("attrib 0x%2x,type 0x%x,len %d,value 0x%2x\r\n", attrib_id, type,attrib_len,*(uint16_t*)data_ptr);
                if (handle->recordAttrib)
                {
                    handle->recordAttrib(para, attrib_id, type, data_ptr,(void *)ptnode);
                }
                data_ptr += attrib_len;
            }
            else
            {
                LogError("attribute 0x%2x data length overflow, reach %d, payload %d\r\n", attrib_id, data_ptr - hdr->payload, hdr->payload_len);
                break;
            }
        }
    }
}

static void zcl_cmd_report_attrib(ZclHandler_ST * handle, void *para, uint8_t *resp, uint8_t respMaxLen, ZclHdr_ST *hdr)
{
	uint8_t err = 0;
	uint8_t *data_ptr;
	/////////////////////////////////////soso
	C4ZclPara_ST * c4Para = (C4ZclPara_ST *)para;
	NodeNetworkStatus_ST *ptnode = NULL;

	ptnode = seekNodeBuf(c4Para->remote_addr);
	if(ptnode == NULL)
	{
		ptnode = getEmptyNodeBuf();	
		if(ptnode == NULL)
		{
			LogDebug("%s err1\n",__FUNCTION__);
			//clear
			
			return;
		}			
		ptnode->userd = 1;
		ptnode->nodeId = c4Para->remote_addr;
		ptnode->ready = 0;

		ptnode->netType = NET_TYPE;
		ptnode->ready |= NETTYPE_OTTSET_BIT;
		ezspLookupEui64ByNodeId(ptnode->nodeId,ptnode->ieee);
		ptnode->ready |= IEEE_OTTSET_BIT;

		LogDebug("%s new NodeBuf\n",__FUNCTION__);
	}
	/////////////////////////////////////	
    data_ptr = hdr->payload;  	
    while ((data_ptr - hdr->payload + 4) <= hdr->payload_len) /*+4是确保有ID，type和至少1字节的data字段*/
    {
        uint8_t type, attrib_len;
        uint16_t attrib_id;

        LogHexDump(LOG_ERROR, "data_ptr", data_ptr, 4);
        //attrib_id = *(uint16_t *) data_ptr;
		attrib_id = CHANGE16_LE(*data_ptr,*(data_ptr+1));//soso
        data_ptr += sizeof(uint16_t);
        type = *(uint8_t *) data_ptr++;
        if ((attrib_len = zcl_get_attrib_size(type, data_ptr)) == 0xff)
        {
        	err = 1;
            LogError("unsupported type 0x%2x of attr 0x%2x\r\n", type, attrib_id);
            break;
        }
		//gError("attrib_id = 0x%04x  type = 0x%02x  attrib_len = %d\n",attrib_id,type,attrib_len);
        if((data_ptr - hdr->payload + attrib_len) <= hdr->payload_len)
        {		
			//LogDebug("attrib 0x%2x,type 0x%x,len %d,value 0x%2x\r\n", attrib_id, type,attrib_len,*(uint16_t*)(++data_ptr));//soso
            if (handle->recordAttrib)
            {
	            handle->recordAttrib(para, attrib_id, type, data_ptr,(void *)ptnode);
            }
            data_ptr += attrib_len;
        }
        else
        {
        	err = 1;
            LogError("attribute 0x%2x data length overflow, reach %d, payload %d\r\n", attrib_id, data_ptr - hdr->payload, hdr->payload_len);
            break;
        }
    }
	/////////////////////////////////////soso
	if(err == 0)
	{
		if((ptnode->ready & PDS_OTTSET_BIT) == PDS_OTTSET_BIT)
		{
			zapNodeNetworkStatusNotify(1,ptnode->ieee,ptnode->netType,ptnode->deviceType,ptnode->rssi,ptnode->lqi,
										ptnode->firmwareVer,ptnode->productString);
			LogDebug("%s zapNodeNetworkStatusNotify\n",__FUNCTION__);
		}
		if(ptnode->ready == ALL_READY)
		{
			//free
			ptnode->userd = 0;
			ptnode->ready = 0;
			ptnode->nodeId = 0;
			LogDebug("%s ptnode free\n",__FUNCTION__);
		}
	}
	/////////////////////////////////////
}

static uint8_t zcl_cmd_write_attrib(ZclHandler_ST * handle, void *para, uint8_t *resp, uint8_t respMaxLen, ZclHdr_ST *hdr)
{
    uint8_t *data_ptr, *resp_ptr;
    ZclHdr_ST hdr_out;

    // init the data and response pointers
    resp_ptr = resp;
    data_ptr = hdr->payload;

    // generate the header. most of the info is the same as the inbound hdr so do
    // a memcpy and just modify the parts that need to change
    memcpy(&hdr_out, hdr, sizeof(ZclHdr_ST));
    hdr_out.frm_ctrl.dir = ZCL_FRM_DIR_TO_CLI;
    hdr_out.cmd = ZCL_CMD_WRITE_ATTRIB_RESP;

    // this is where the header actually gets generated
    resp_ptr = zcl_gen_hdr(resp_ptr, &hdr_out);

    while ((data_ptr - hdr->payload + 3) <= hdr->payload_len)
    {
        uint8_t type, status, attrib_len, invalid_format = 0;
        uint16_t attrib_id;
        ZclAttrib_ST *attrib;

        // init the status by setting an invalid value
        status = ZCL_STATUS_SUCCESS;

        attrib_id = *(uint16_t *) data_ptr;
        data_ptr += sizeof(uint16_t);
        type = *(uint8_t *) data_ptr++;
        if ((attrib_len = zcl_get_attrib_size(type, data_ptr)) == 0xff)
        {
            LogError("unsupported type 0x%2x of attr 0x%2x\r\n", type, attrib_id);
            break;
        }
        if ((resp_ptr - resp + 3) > respMaxLen)
        {
            LogError("attribute 0x%2x response buffer insuffcient\r\n", attrib_id);
            break;
        }
        else if (attrib_len == 0xff)
        {
            status = ZCL_STATUS_INVALID_TYPE;
            invalid_format = 1;
        }
        else if ((data_ptr - hdr->payload + attrib_len) > hdr->payload_len)
        {
            status = ZCL_STATUS_INSUFF_SPACE;
            invalid_format = 1;
            LogError( "attribute 0x%2x length %d greater than actual data length\r\n", attrib_id, attrib_len);
        }
        else if ( (handle->findAttrib == NULL) || ((attrib = handle->findAttrib(para, attrib_id)) == NULL) )
        {
            status = ZCL_STATUS_UNSUP_ATTRIB;
            LogError("unsupported attribute 0x%2x\r\n", attrib_id);
            return 0;
        }
        else if (type != attrib->type)
        {
            status = ZCL_STATUS_INVALID_TYPE;
            LogError("type not match, attribute 0x%2x request type 0x%x local type 0x%x\r\n", attrib_id, type, attrib->type);
        }
        else if (attrib->access == ZCL_ACCESS_READ_ONLY)
        {
            status = ZCL_STATUS_READ_ONLY;
            LogError("attribute 0x%2x readonly\r\n", attrib_id);
        }

        // add status and attrib id to response
        *resp_ptr++ = status;
        *(uint16_t *) resp_ptr = attrib_id;
        resp_ptr += sizeof(uint16_t);

        // if we're out of room, then end the response frame here. otherwise if we ran into a problem
        // with the attrib, then update the status and move on to the next one.
        if (invalid_format)
        {
            break;
        }
        else if (status != ZCL_STATUS_SUCCESS)
        {
            data_ptr += attrib_len;
            continue;
        }

        memcpy(attrib->data, data_ptr, attrib_len);
        data_ptr += attrib_len;

        if (handle->saveAttrib)
        {
            handle->saveAttrib(para, attrib_id, type, data_ptr);
        }
    }

    // add the len to the response buffer and move the data pointer to the beginning of the data
    return (resp_ptr - resp);
}



uint8_t Zcl_ParseFrame(ZclHandler_ST *handle, void *para, uint8_t *resp, uint8_t respMaxLen, ZclHdr_ST *hdr)
{
	//ZclHdr_ST rsphdr;
    uint8_t resp_len = 0;
    uint8_t zcl_status;
    if (hdr->frm_ctrl.frm_type == ZCL_FRM_TYPE_GENERAL)
    {
        // decode the cmd and route it to the correct function
        switch (hdr->cmd)
        {
        case ZCL_CMD_READ_ATTRIB:
            resp_len = zcl_cmd_read_attrib(handle, para, resp, respMaxLen, hdr);	
            break;
        case ZCL_CMD_READ_ATTRIB_RESP:
            zcl_cmd_read_attrib_resp(handle, para, resp, respMaxLen, hdr);
            resp_len = 0;
            break;
        case ZCL_CMD_REPORT_ATTRIB:
            zcl_cmd_report_attrib(handle, para, resp, respMaxLen, hdr);
            resp_len = 0;
            break;
        case ZCL_CMD_WRITE_ATTRIB:
            resp_len = zcl_cmd_write_attrib(handle, para, resp, respMaxLen, hdr);
            break;
        case ZCL_CMD_WRITE_ATTRIB_NO_RESP:
            (void)zcl_cmd_write_attrib(handle, para, resp, respMaxLen, hdr);
            resp_len = 0;
            break;
        case ZCL_CMD_DEFAULT_RESP:
            resp_len = 0;
            break;
        default:
            LogError("unsupported general command 0x%x\r\n", hdr->cmd);
            resp_len = 0;
            break;
        }
    }
    else
    {
        if (handle->specCommand)
        {
            zcl_status = handle->specCommand(para, resp, respMaxLen, hdr);

            // send the default response
            if (!hdr->frm_ctrl.dis_def_resp)
            {
                if(zcl_status != ZCL_STATUS_CMD_HAS_RSP)
                {
                    resp_len = zcl_gen_def_resp(resp, zcl_status, hdr);
                }
                else
                {
                    resp_len = 0;
                }
            }
        }
    }
    return resp_len;
}