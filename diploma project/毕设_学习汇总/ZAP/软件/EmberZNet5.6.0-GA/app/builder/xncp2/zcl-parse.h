/**
 * \brief 适用于不同模块的共通的日志输出模块对外接口头文件
 * \copyright inSona Co.,Ltd.
 * \author  
 * \file  Logger.h
 *
 * Dependcy:
 *       Platform.stdint
 *       Platform.macrodefine
 *       Platform.string
 *       Platform.stdio
 * 
 * \date 2013-10-17   zhouyong 新建文件
**/

#ifndef ZCL_PARSE_H
#define ZCL_PARSE_H


/*standard type define*/
#include "Platform.h"


#define ZCL_MAX_PAYLOAD_SIZE    (73)


#define CHANGE16_LE(a1,a2)						((((a2)&0x00ff)<<8)+(a1))
#define CHANGE16_BE(a1,a2)						((((a1)&0x00ff)<<8)+(a2))
#define CHANGE64_LE(a0,a1,a2,a3,a4,a5,a6,a7)	(((u64)a0<<56)|((u64)a1<<48)|((u64)a2<<40)|((u64)a3<<32)|((u64)a4<<24)|((u64)a5<<16)|((u64)a6<<8)|((u64)a7))


/**************************************************************************/
/*!
 Enumeration of the ZCL frame fields.
 */
/**************************************************************************/
typedef enum
{
    ZCL_FRM_TYPE_OFF = 0,                        ///< Frame type offset
    ZCL_MANUF_SPEC_OFF = 2,                        ///< Manuf specification offset
    ZCL_DIR_OFF = 3,                        ///< Direction offset
    ZCL_DIS_DEF_OFF = 4,                        ///< Disable default response offset

    ZCL_FRM_TYPE_MASK = (3 << ZCL_FRM_TYPE_OFF),    ///< Frame type mask
    ZCL_MANUF_SPEC_MASK = (1 << ZCL_MANUF_SPEC_OFF),  ///< Manufacturing spec mask
    ZCL_DIR_MASK = (1 << ZCL_DIR_OFF),         ///< Direction mask
    ZCL_DIS_DEF_MASK = (1 << ZCL_DIS_DEF_OFF),     ///< Disable default response mask

    ZCL_FRM_TYPE_GENERAL = 0,                    ///< Frame type - general
    ZCL_FRM_TYPE_CLUST_SPEC = 1,                    ///< Frame type - cluster specific

    ZCL_FRM_DIR_TO_SRVR = 0,                    ///< Direction - To server
    ZCL_FRM_DIR_TO_CLI = 1,                    ///< Direction - To client

    ZCL_RPT_DIR_SEND_RPT = 0                     ///< Report Direction - Send report
} ZclFrame_ENUM;


/**************************************************************************/
/*!
 Enumeration of the ZCL foundation command IDs.
 */
/**************************************************************************/
typedef enum
{
    ZCL_CMD_READ_ATTRIB = 0x00,      ///< Read attributes foundation command ID
    ZCL_CMD_READ_ATTRIB_RESP = 0x01,      ///< Read attributes response foundation command ID
    ZCL_CMD_WRITE_ATTRIB = 0x02,      ///< Write attributes foundation command ID
    ZCL_CMD_WRITE_ATTRIB_RESP = 0x03,      ///< Write attributes response foundation command ID
    ZCL_CMD_WRITE_ATTRIB_UNDIV = 0x04,      ///< Write attributes undivided foundation command ID
    ZCL_CMD_WRITE_ATTRIB_NO_RESP = 0x05,      ///< Write attributes no response foundation command ID
    ZCL_CMD_CONFIG_REPORT = 0x06,      ///< Configure reporting foundation command ID
    ZCL_CMD_CONFIG_REPORT_RESP = 0x07,      ///< Configure reporting response foundation command ID
    ZCL_CMD_READ_REPORT_CFG = 0x08,      ///< Read reporting config foundation command ID
    ZCL_CMD_READ_REPORT_CFG_RESP = 0x09,      ///< Read reporting config response foundation command ID
    ZCL_CMD_REPORT_ATTRIB = 0x0a,      ///< Report attribute foundation command ID
    ZCL_CMD_DEFAULT_RESP = 0x0b,      ///< Default response foundation command ID
    ZCL_CMD_DISC_ATTRIB = 0x0c,      ///< Discover attributes foundation command ID
    ZCL_CMD_DISC_ATTRIB_RESP = 0x0d       ///< Discover attributes response foundation command ID
} ZclGeneralCommand_ENUM;

/**************************************************************************/
/*!
 Enumeration of the ZCL attribute data type values.
 */
/**************************************************************************/
typedef enum
{
    ZCL_TYPE_NULL = 0x00,     ///< Null data type
    ZCL_TYPE_8BIT = 0x08,     ///< 8-bit value data type
    ZCL_TYPE_16BIT = 0x09,     ///< 16-bit value data type
    ZCL_TYPE_24BIT = 0x0a,     ///< 24-bit value data type
    ZCL_TYPE_32BIT = 0x0b,     ///< 32-bit value data type
    ZCL_TYPE_40BIT = 0x0c,     ///< 40-bit value data type
    ZCL_TYPE_48BIT = 0x0d,     ///< 48-bit value data type
    ZCL_TYPE_56BIT = 0x0e,     ///< 56-bit value data type
    ZCL_TYPE_64BIT = 0x0f,     ///< 64-bit value data type
    ZCL_TYPE_BOOL = 0x10,     ///< Boolean data type
    ZCL_TYPE_8BITMAP = 0x18,     ///< 8-bit bitmap data type
    ZCL_TYPE_16BITMAP = 0x19,     ///< 16-bit bitmap data type
    ZCL_TYPE_24BITMAP = 0x1a,     ///< 24-bit bitmap data type
    ZCL_TYPE_32BITMAP = 0x1b,     ///< 32-bit bitmap data type
    ZCL_TYPE_40BITMAP = 0x1c,     ///< 40-bit bitmap data type
    ZCL_TYPE_48BITMAP = 0x1d,     ///< 48-bit bitmap data type
    ZCL_TYPE_56BITMAP = 0x1e,     ///< 56-bit bitmap data type
    ZCL_TYPE_64BITMAP = 0x1f,     ///< 64-bit bitmap data type
    ZCL_TYPE_U8 = 0x20,     ///< Unsigned 8-bit value data type
    ZCL_TYPE_U16 = 0x21,     ///< Unsigned 16-bit value data type
    ZCL_TYPE_U24 = 0x22,     ///< Unsigned 16-bit value data type
    ZCL_TYPE_U32 = 0x23,     ///< Unsigned 32-bit value data type
    ZCL_TYPE_U40 = 0x24,     ///< Unsigned 40-bit value data type
    ZCL_TYPE_U48 = 0x25,     ///< Unsigned 48-bit value data type
    ZCL_TYPE_U56 = 0x26,     ///< Unsigned 56-bit value data type
    ZCL_TYPE_U64 = 0x27,     ///< Unsigned 64-bit value data type
    ZCL_TYPE_S8 = 0x28,     ///< signed 8-bit value data type
    ZCL_TYPE_S16 = 0x29,     ///< signed 16-bit value data type
    ZCL_TYPE_S24 = 0x2a,     ///< signed 16-bit value data type
    ZCL_TYPE_S32 = 0x2b,     ///< signed 32-bit value data type
    ZCL_TYPE_S40 = 0x2c,     ///< signed 40-bit value data type
    ZCL_TYPE_S48 = 0x2d,     ///< signed 48-bit value data type
    ZCL_TYPE_S56 = 0x2e,     ///< signed 56-bit value data type
    ZCL_TYPE_S64 = 0x2f,     ///< signed 64-bit value data type
    ZCL_TYPE_ENUM8 = 0x30,     ///
    ZCL_TYPE_ENUM16 = 0x31,     ///
    ZCL_TYPE_BYTE_ARRAY = 0x41,     ///< Byte array data type
    ZCL_TYPE_CHAR_STRING = 0x42,     ///< Charactery string (array) data type
    ZCL_TYPE_LONG_BYTE_ARRAY = 0x43,     ///< Long Byte array data type
    ZCL_TYPE_LONG_CHAR_STRING = 0x44,     ///< Long Charactery string (array) data type
    ZCL_TYPE_TIME_OF_DAY = 0xe0,     ///
    ZCL_TYPE_DATE = 0xe1,     ///
    ZCL_TYPE_UTCTIME = 0xe2,     ///
    ZCL_TYPE_CLUST_ID = 0xe8,     ///
    ZCL_TYPE_ATTRIB_ID = 0xe9,     ///
    ZCL_TYPE_IEEE_ADDR = 0xf0,     ///< IEEE address (U64) type
    ZCL_TYPE_SEC_KEY = 0xf1,     ///
    ZCL_TYPE_INVALID = 0xff      ///< Invalid data type
} ZclAttribType_ENUM;

/**************************************************************************/
/*!
 Enumeration of the attribute access values.
 */
/**************************************************************************/
typedef enum
{
    ZCL_ACCESS_READ_ONLY = 0x00,     ///< Attribute is read only
    ZCL_ACCESS_READ_WRITE = 0x01      ///< Attribute is read/write
} ZclAttribAccess_ENUM;

/**************************************************************************/
/*!
 Enumeration of the ZCL status values.
 */
/**************************************************************************/
typedef enum
{
    ZCL_STATUS_SUCCESS = 0x00,     ///< ZCL Success
    ZCL_STATUS_FAILURE = 0x01,     ///< ZCL Fail
    ZCL_STATUS_NOT_AUTHORIZED = 0x7e,     ///<
    ZCL_STATUS_RESERVED_FIELD_NOT_ZERO = 0x7f,     ///<
    ZCL_STATUS_MALFORMED_CMD = 0x80,     ///< Malformed command
    ZCL_STATUS_UNSUP_CLUST_CMD = 0x81,     ///< Unsupported cluster command
    ZCL_STATUS_UNSUP_GEN_CMD = 0x82,     ///< Unsupported general command
    ZCL_STATUS_UNSUP_MANUF_CLUST_CMD = 0x83,     ///< Unsupported manuf-specific clust command
    ZCL_STATUS_UNSUP_MANUF_GEN_CMD = 0x84,     ///< Unsupported manuf-specific general command
    ZCL_STATUS_INVALID_FIELD = 0x85,     ///< Invalid field
    ZCL_STATUS_UNSUP_ATTRIB = 0x86,     ///< Unsupported attribute
    ZCL_STATUS_INVALID_VALUE = 0x87,     ///< Invalid value
    ZCL_STATUS_READ_ONLY = 0x88,     ///< Read only
    ZCL_STATUS_INSUFF_SPACE = 0x89,     ///< Insufficient space
    ZCL_STATUS_DUPE_EXISTS = 0x8a,     ///< Duplicate exists
    ZCL_STATUS_NOT_FOUND = 0x8b,     ///< Not found
    ZCL_STATUS_UNREPORTABLE_ATTRIB = 0x8c,     ///< Unreportable attribute
    ZCL_STATUS_INVALID_TYPE = 0x8d,     ///< Invalid type
    ZCL_STATUS_INVALID_SELECTOR = 0x8e,     ///<
    ZCL_STATUS_WRITE_ONLY = 0x8f,     ///<
    ZCL_STATUS_INCONSISTENT_STARTUP_STATE = 0x90,     ///<
    ZCL_STATUS_DEFINED_OUT_OF_BAND = 0x91,     ///<
    ZCL_STATUS_INCONSISTENT = 0x92,     ///<
    ZCL_STATUS_ACTION_DENIED = 0x93,     ///<
    ZCL_STATUS_TIMEOUT = 0x94,     ///<
    ZCL_STATUS_ABORT = 0x95,     ///<
    ZCL_STATUS_INVALID_IMAGE = 0x96,     ///<
    ZCL_STATUS_WAIT_FOR_DATA = 0x97,     ///<
    ZCL_STATUS_NO_IMAGE_AVAILABLE = 0x98,     ///<
    ZCL_STATUS_REQUIRE_MORE_IMAGE = 0x99,     ///<
    ZCL_STATUS_HW_FAIL = 0xc0,     ///< Hardware failure
    ZCL_STATUS_SW_FAIL = 0xc1,     ///< Software failure
    ZCL_STATUS_CALIB_ERR = 0xc2,     ///< Calibration error
    ZCL_STATUS_DISC_COMPLETE = 0x01,     ///< Discovery complete
    ZCL_STATUS_DISC_INCOMPLETE = 0x00,      ///< Discovery incomplete
    ZCL_STATUS_CMD_HAS_RSP = 0xFF ///< Non-standard status (used for Default Rsp)
} ZclStatus_ENUM;




typedef struct
{
    uint16_t clust_id;
    uint16_t remote_addr;
    uint8_t remote_ep;
    uint8_t local_ep;
} C4ZclPara_ST;

/**************************************************************************/
/*!
 ZCL attribute structure. Contains all fields to address the attribute.
 Also, contains an optional field in case the attribute is set to
 report.
 */
/**************************************************************************/
typedef struct
{
    uint16_t id;             ///< Attrib ID
    uint8_t type;           ///< Attrib data type
    uint8_t access;         ///< Attrib data access privileges
    void *data;          ///< Ptr to data
} ZclAttrib_ST;

/**************************************************************************/
/*!
 ZCL frame control field.
 */
/**************************************************************************/
typedef struct
{
    uint8_t frm_type;       ///< ZCL Frame type
    bool_t manuf_spec;     ///< Manufacturer specific frame
    bool_t dir;            ///< Direction
    bool_t dis_def_resp;   ///< Disable default response
} ZclFrmCtrl_ST;

/**************************************************************************/
/*!
 ZCL frame header structure.
 */
/**************************************************************************/
typedef struct
{
    ZclFrmCtrl_ST frm_ctrl;    ///< Frame control field structure
    uint8_t fcf;                    ///< Frame control field - condensed into single byte
    uint16_t manuf_code;             ///< Manufacturer code
    uint8_t seq_num;                ///< Sequence number - used to identify response frame
    uint8_t cmd;                    ///< Command ID
    uint8_t *payload;               ///< Payload
    uint8_t payload_len;            ///< Payload length
} ZclHdr_ST;

/**************************************************************************/
/*!
 ZCL handles structure.
 */
/**************************************************************************/
typedef struct
{
    ZclAttrib_ST * (*findAttrib)(void * para, uint16_t id);    ///< 找到自己的attrib结构体
    void (*saveAttrib)(void * para, uint16_t id, uint8_t type, uint8_t *data);    ///< 存储自己的attrib
    void (*recordAttrib)(void * para, uint16_t id, uint8_t type, uint8_t *data,void *stjoin);    ///< 记录别人的attrib
    uint8_t (*specCommand)(void * para, uint8_t *resp, uint8_t respMaxLen, ZclHdr_ST *hdr);    ///< 记录别人的attrib
} ZclHandler_ST;

typedef void (*attributesIdentifierHandle)(void * para,uint8_t type,uint8_t *data,void *stjoin);



#define NET_TYPE	("control4")

#define IEEE_OTTSET_BIT				(0x01<<0)
#define NETTYPE_OTTSET_BIT			(0x01<<1)
#define DVTYPE_OTTSET_BIT			(0x01<<2)
#define RSSI_OTTSET_BIT				(0x01<<3)
#define LQI_OTTSET_BIT				(0x01<<4)
#define FVER_OTTSET_BIT				(0x01<<5)
#define PDS_OTTSET_BIT				(0x01<<6)
#define ALL_READY					(0x7f)

#define NODE_STATUS_BUF_SIZE		(5)
typedef struct 
{
	uint8_t userd;	
	uint16_t nodeId;
	uint8_t ready;

	uint8_t ieee[8];
	char *netType;
	uint8_t deviceType;
	int8_t rssi;
	uint8_t lqi;
	char firmwareVer[16];
	char productString[24];
}NodeNetworkStatus_ST;
	
#define ZCL_MAX_TX_PAYLOAD_SIZE	(128)

int32_t ZclParseHeader(uint8_t *data, uint8_t len, ZclHdr_ST *hdr);
uint8_t Zcl_ParseFrame(ZclHandler_ST *handle, void *para, uint8_t *resp, uint8_t respMaxLen, ZclHdr_ST *hdr);

void NodeStatusInit(void);
void clearNodeBuf(uint16_t shortid);
NodeNetworkStatus_ST *getEmptyNodeBuf(void);
NodeNetworkStatus_ST *seekNodeBuf(uint16_t shortid);


#endif
