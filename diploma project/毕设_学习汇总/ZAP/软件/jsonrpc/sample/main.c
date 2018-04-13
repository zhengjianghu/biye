/*
 * Copyright (c) 2012 Jonghyeok Lee <jhlee4bb@gmail.com>
 *
 * jsonrpC is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <stdio.h>
#include <math.h>
#include "jsonrpc.h"

#include "Logger.h"

jsonrpc_server_t * self = NULL;

char Num2HexChar(uint8_t num)
{
	num&=0xFu;
	return (num>9u)?('7'+num):('0'+num);
}
uint8_t HexChar2Num(char c)
{
	if( (c >= '0') && (c <= '9') )
	{
		return c - '0';
	}
	else if( (c >= 'a') && (c <= 'f') )
	{
		return c - 'a' + 0xA;
	}
	else if( (c >= 'A') && (c <= 'F') )
	{
		return c - 'A' + 0xA;
	}
	else
	{
		return 0xFF;
	}
}

char * Array2HexString(const uint8_t array[], int arrayLen, char hex[], int maxLen)
{
	int i;
	if( (array == NULL) || (hex == NULL) || (arrayLen > 0) || (maxLen >= 2 * arrayLen + 1) )
	{
		for(i = 0; i < arrayLen; i++)
		{
			hex[2*i] = Num2HexChar(array[i]>>4);
			hex[2*i+1] = Num2HexChar(array[i]);
		}
		hex[2*i]='\0';
		return hex;
	}
	else
	{
		return NULL;
	}
}
uint8_t * HexString2Array(const char hex[], uint8_t array[], int * arrayLen)
{
	int i;

	if( (array == NULL) || (hex == NULL))
	{
		int maxLen = *arrayLen;
		int hexLen = strnlen(hex, 2*maxLen);

		if( (hexLen > 0) || ((hexLen & 1u) == 0u) )
		{
			for(i = 0; i < hexLen/2; i++)
			{
				uint8_t hi, lo;
				hi = HexChar2Num(hex[2*i]);
				lo = HexChar2Num(hex[2*i+1]);
				if((hi == 0xFF) || (lo == 0xFF))
				{
					return NULL;
				}
				array[i] = (hi<<4)+lo;
			}
			*arrayLen = i;
			return array;
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		return NULL;
	}
}


/******************************************************************************
 * FunctionName : zapFormNetwork
 * Description  : 组建zigbee网络
 * Parameters   : netType=网络类型,字符举类型 如 "control4"
 				: panId
 				: radioTxPower
 				: radioChannel
 * Returns      : 1=success\0=fail
 * uint8_t zapFormNetwork(const char *netType,
							uint16_t panId,
							uint8_t radioTxPower,
							uint8_t radioChannel);
*******************************************************************************/
uint8_t zapFormNetwork(const char *netType,
    uint16_t panId,
    uint8_t radioTxPower,
    uint8_t radioChannel)
{
    printf("%s \n", __FUNCTION__);
	return 0;
}
jsonrpc_error_t server_zapFormNetwork(int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{

uint8_t netType[8];
int netTypeLen = sizeof(netType);
double panId;
double radioTxPower;
double radioChannel;

if( (argc != 4) ||
(argv[0].json.type != JSONRPC_TYPE_STRING) ||
(argv[1].json.type != JSONRPC_TYPE_NUMBER) ||
(argv[2].json.type != JSONRPC_TYPE_NUMBER) ||
(argv[3].json.type != JSONRPC_TYPE_NUMBER) )
{
return JSONRPC_ERROR_INVALID_PARAMS;
}

HexString2Array(argv[0].json.u.string, netType, &netTypeLen);
if(netTypeLen != 8)
{
return JSONRPC_ERROR_INVALID_PARAMS;
}

panId = argv[1].json.u.number;
radioTxPower = argv[2].json.u.number;
radioChannel = argv[3].json.u.number;
zapFormNetwork(netType,
            (uint16_t)panId,
              (uint16_t)radioTxPower,
              (uint8_t)radioChannel
              );
return JSONRPC_ERROR_OK;
}



/******************************************************************************
 * FunctionName : zapLeaveNetwork
 * Description  : 删除zigbee网络
 * Parameters   : netType=网络类型,字符举类型 如 "control4"
 * Returns      : 1=success\0=fail
 * uint8_t zapLeaveNetwork(const char *netType);
*******************************************************************************/
uint8_t zapLeaveNetwork(const char *netType)
{
	printf("%s \n", __FUNCTION__);
	return 0;
}
jsonrpc_error_t server_zapLeaveNetwork (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{
	uint8_t netType[8];
			int netTypeLen = sizeof(netType);

			if( (argc != 1) ||
					(argv[0].json.type != JSONRPC_TYPE_STRING) )
			{
				return JSONRPC_ERROR_INVALID_PARAMS;
			}

			HexString2Array(argv[0].json.u.string, netType, &netTypeLen);
			if(netTypeLen != 8)
			{
				return JSONRPC_ERROR_INVALID_PARAMS;
			}
			zapLeaveNetwork(netType);

			return JSONRPC_ERROR_OK;
}



/******************************************************************************
 * FunctionName : zapPermitJion
 * Description  : 在窗口时间内允许入网
 * Parameters   : seconds=窗口时间
 * Returns      : 1=success\0=fail
 * uint8_t zapPermitJion(uint8_t seconds);
*******************************************************************************/
uint8_t zapPermitJion(uint8_t seconds)
{
	printf("%s \n", __FUNCTION__);
	return 0;
}

jsonrpc_error_t server_zapPermitJion (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{

	double seconds;


	if( (argc != 1) ||
	(argv[0].json.type != JSONRPC_TYPE_NUMBER) )
	{
	return JSONRPC_ERROR_INVALID_PARAMS;
	}
	seconds = argv[0].json.u.number;

	zapPermitJion(
	            (uint16_t)seconds
	              );
	return JSONRPC_ERROR_OK;
}



/******************************************************************************
 * FunctionName : zapRemoveDevice
 * Description  : 删除远端设备
 * Parameters   : 参数待定
 * Returns      : 1=success\0=fail
 * uint8_t zapRemoveDevice(char *ieee);
*******************************************************************************/
uint8_t zapRemoveDevice(char *ieee)
{
	printf("%s \n", __FUNCTION__);
	return 0;
}
jsonrpc_error_t server_zapRemoveDevice (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{
	uint8_t ieee[8];
			int ieeeLen = sizeof(ieee);

			if( (argc != 1) ||
					(argv[0].json.type != JSONRPC_TYPE_STRING) )
			{
				return JSONRPC_ERROR_INVALID_PARAMS;
			}

			HexString2Array(argv[0].json.u.string, ieee, &ieeeLen);
			if(ieeeLen != 8)
			{
				return JSONRPC_ERROR_INVALID_PARAMS;
			}
			zapRemoveDevice(ieee);

			return JSONRPC_ERROR_OK;
}




/******************************************************************************
 * FunctionName : zapSetChannel
 * Description  : 设置通道参数
 * Parameters   : channel 通道参数
 * Returns      : 1=success\0=fail
 * uint8_t zapSetChannel(uint8_t channel);
*******************************************************************************/
uint8_t zapSetChannel(uint8_t channel)
{
	printf("%s \n", __FUNCTION__);
	return 0;
}

jsonrpc_error_t server_zapSetChannel (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{

	double channel;


	if( (argc != 1) ||
	(argv[0].json.type != JSONRPC_TYPE_NUMBER) )
	{
	return JSONRPC_ERROR_INVALID_PARAMS;
	}
	channel = argv[0].json.u.number;

	zapSetChannel(
	            (uint16_t)channel
	              );
	return JSONRPC_ERROR_OK;
}


/******************************************************************************
 * FunctionName : zapSentCallback
 * Description  : 报文发送确认回调函数
 * Parameters   : 同zapSendUnicast
 * Returns      : 1=success\0=fail
*******************************************************************************/
//**功能函数具体定义**//
void zapSentNotify (uint8_t ieee[8],
    uint16_t profileId,
      uint16_t clusterId,
      uint8_t sourceEndpoint,
      uint8_t destinationEndpoint,
      uint8_t messageLength,
      uint8_t* message)
{
char ieeeStr[17];
char * messageStr = (char *)malloc(messageLength*2+1);

if(messageStr != NULL)
{
    Array2HexString(ieee, 8, ieeeStr, sizeof(ieeeStr));
    Array2HexString(message, messageLength, messageStr, messageLength*2+1);

    //****报文发送动作**//
    jsonrpc_notify_client(self,
            "zapSentNotify",
            "siiiis", ieeeStr,
            (double)profileId,
            (double)clusterId,
            (double)sourceEndpoint,
            (double)destinationEndpoint,
            messageStr);
}
//**功能函数注册**//
}
jsonrpc_error_t  server_zapSentNotify (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{
uint8_t ieee[8];
    int ieeeLen = sizeof(ieee);
    double profileId;
    double clusterId;
    double sourceEndpoint;
    double destinationEndpoint;
    uint8_t *message = (uint8_t *)malloc(100);/*一条zigbee报文不超过100字节*/
    int messageLen = 100;
    if(message == NULL)
        {
            return JSONRPC_ERROR_SERVER_INTERNAL;
        }
    if( (argc != 6) ||
                (argv[0].json.type != JSONRPC_TYPE_STRING) ||
                (argv[1].json.type != JSONRPC_TYPE_NUMBER) ||
                (argv[2].json.type != JSONRPC_TYPE_NUMBER) ||
                (argv[3].json.type != JSONRPC_TYPE_NUMBER) ||
                (argv[4].json.type != JSONRPC_TYPE_NUMBER) ||
                (argv[5].json.type != JSONRPC_TYPE_STRING)  )
        {
            return JSONRPC_ERROR_INVALID_PARAMS;
        }
    HexString2Array(argv[0].json.u.string, ieee, &ieeeLen);
        if(ieeeLen != 8)
        {
            return JSONRPC_ERROR_INVALID_PARAMS;
        }
        profileId = argv[1].json.u.number;
        clusterId = argv[2].json.u.number;
        sourceEndpoint = argv[3].json.u.number;
        destinationEndpoint = argv[4].json.u.number;
        HexString2Array(argv[5].json.u.string, message, &messageLen);
        if((messageLen <= 0) || (messageLen > 100))
            {
                return JSONRPC_ERROR_INVALID_PARAMS;
            }
        zapSentNotify(ieee,
                            (uint16_t)profileId,
                            (uint16_t)clusterId,
                            (uint8_t)sourceEndpoint,
                            (uint8_t)destinationEndpoint,
                            (uint8_t)messageLen,
                            message
                       );
return JSONRPC_ERROR_OK;
}






/******************************************************************************
 * FunctionName : zapSendUnicast
 * Description  : 单播发送
 * Parameters   : ieee=64位地址 hex string
 				: profileId=配置文件ID
 				: clusterId=集群ID
 				: sourceEndpoint=源端点号
 				: destinationEndpoint=目的端点号
 				: messageLength=消息长度
 				: message=消息缓存
 				: needConfirm=是否需要回复应答标识
 * Returns      : messageTag=报文序列号
 * uint8_t zapSendUnicast(uint8_t ieee[8],
							uint16_t profileId,
                      		uint16_t clusterId,
                      		uint8_t sourceEndpoint,
                      		uint8_t destinationEndpoint,
                      		uint8_t messageLength,
                      		uint8_t* message,
                      		bool_t needConfirm);
*******************************************************************************/
uint8_t zapSendUnicast(uint8_t ieee[8],
	uint16_t profileId,
	  uint16_t clusterId,
	  uint8_t sourceEndpoint,
	  uint8_t destinationEndpoint,
	  uint8_t messageLength,
	  uint8_t* message,
	  bool_t needConfirm)
{
printf("%s \n", __FUNCTION__);
return 0;
}
jsonrpc_error_t server_zapSendUnicast (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{

	uint8_t r;
	uint8_t ieee[8];
	int ieeeLen = sizeof(ieee);
	double profileId;
	double clusterId;
	double sourceEndpoint;
	double destinationEndpoint;
	uint8_t *message = (uint8_t *)malloc(100);/*一条zigbee报文不超过100字节*/
	int messageLen = 100;
	jsonrpc_bool_t needConfirm;

	if(message == NULL)
	{
		return JSONRPC_ERROR_SERVER_INTERNAL;
	}

	if( (argc != 7) ||
			(argv[0].json.type != JSONRPC_TYPE_STRING) ||
			(argv[1].json.type != JSONRPC_TYPE_NUMBER) ||
			(argv[2].json.type != JSONRPC_TYPE_NUMBER) ||
			(argv[3].json.type != JSONRPC_TYPE_NUMBER) ||
			(argv[4].json.type != JSONRPC_TYPE_NUMBER) ||
			(argv[5].json.type != JSONRPC_TYPE_STRING) ||
			(argv[6].json.type != JSONRPC_TYPE_BOOLEAN) )
	{
		return JSONRPC_ERROR_INVALID_PARAMS;
	}

	HexString2Array(argv[0].json.u.string, ieee, &ieeeLen);
	if(ieeeLen != 8)
	{
		return JSONRPC_ERROR_INVALID_PARAMS;
	}

	profileId = argv[1].json.u.number;
	clusterId = argv[2].json.u.number;
	sourceEndpoint = argv[3].json.u.number;
	destinationEndpoint = argv[4].json.u.number;


	HexString2Array(argv[5].json.u.string, message, &messageLen);
	if((messageLen <= 0) || (messageLen > 100))
	{
		return JSONRPC_ERROR_INVALID_PARAMS;
	}

	needConfirm = argv[6].json.u.boolean;

	r = zapSendUnicast(ieee,
								(uint16_t)profileId,
	                      		(uint16_t)clusterId,
	                      		(uint8_t)sourceEndpoint,
	                      		(uint8_t)destinationEndpoint,
	                      		(uint8_t)messageLen,
	                      		message,
	                      		(bool_t)needConfirm);


	print_result(ctx, "%d", r);
	return JSONRPC_ERROR_OK;
}




/******************************************************************************
 * FunctionName : zapNewPacketNotify
 * Description  : 接受报文回调函数
 * Parameters   : ieee=64位地址 hex string
 				: profileId=配置文件ID
 				: clusterId=集群ID
 				: sourceEndpoint=源端点号
 				: destinationEndpoint=目的端点号
 				: messageLength=消息长度
 				: message=消息缓存
 * Returns      : 1=success\0=fail
 * void zapNewPacketNotify(char *ieee,
							uint16_t profileId,
                      		uint16_t clusterId,
                      		uint8_t sourceEndpoint,
                      		uint8_t destinationEndpoint,
                      		uint8_t messageLength,
                      		uint8_t* message);
*******************************************************************************/
void zapNewPacketNotify (uint8_t ieee[8],
    uint16_t profileId,
      uint16_t clusterId,
      uint8_t sourceEndpoint,
      uint8_t destinationEndpoint,
      uint8_t messageLength,
      uint8_t* message)
{
char ieeeStr[17];
char * messageStr = (char *)malloc(messageLength*2+1);

if(messageStr != NULL)
{
    Array2HexString(ieee, 8, ieeeStr, sizeof(ieeeStr));
    Array2HexString(message, messageLength, messageStr, messageLength*2+1);

    //****报文发送动作**//
    jsonrpc_notify_client(self,
            "zapNewPacketNotify",
            "siiiis", ieeeStr,
            (double)profileId,
            (double)clusterId,
            (double)sourceEndpoint,
            (double)destinationEndpoint,
            messageStr);

}

}
jsonrpc_error_t server_zapNewPacketNotify (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{
uint8_t ieee[8];
    int ieeeLen = sizeof(ieee);
    double profileId;
    double clusterId;
    double sourceEndpoint;
    double destinationEndpoint;
    uint8_t *message = (uint8_t *)malloc(100);/*一条zigbee报文不超过100字节*/
    int messageLen = 100;
    if(message == NULL)
        {
            return JSONRPC_ERROR_SERVER_INTERNAL;
        }
    if( (argc != 6) ||
                (argv[0].json.type != JSONRPC_TYPE_STRING) ||
                (argv[1].json.type != JSONRPC_TYPE_NUMBER) ||
                (argv[2].json.type != JSONRPC_TYPE_NUMBER) ||
                (argv[3].json.type != JSONRPC_TYPE_NUMBER) ||
                (argv[4].json.type != JSONRPC_TYPE_NUMBER) ||
                (argv[5].json.type != JSONRPC_TYPE_STRING)  )
        {
            return JSONRPC_ERROR_INVALID_PARAMS;
        }
    HexString2Array(argv[0].json.u.string, ieee, &ieeeLen);
        if(ieeeLen != 8)
        {
            return JSONRPC_ERROR_INVALID_PARAMS;
        }
        profileId = argv[1].json.u.number;
        clusterId = argv[2].json.u.number;
        sourceEndpoint = argv[3].json.u.number;
        destinationEndpoint = argv[4].json.u.number;
        HexString2Array(argv[5].json.u.string, message, &messageLen);
        if((messageLen <= 0) || (messageLen > 100))
            {
                return JSONRPC_ERROR_INVALID_PARAMS;
            }
            zapNewPacketNotify(ieee,
                            (uint16_t)profileId,
                            (uint16_t)clusterId,
                            (uint8_t)sourceEndpoint,
                            (uint8_t)destinationEndpoint,
                            (uint8_t)messageLen,
                            message
                       );
return JSONRPC_ERROR_OK;
}


/******************************************************************************
 * FunctionName : zapNodeNetworkStatusNotify
 * Description  : 节点入网状态变化通知
 * Parameters   ：status 0 = off line/1 = on line
 				: ieee=64位地址hex string
 				: netType=网络类型        如 "control4"
 				: deviceType=设备类型
 				: rssi=
 				: lqi=
 				: firmwareVer=固件版本号
 				: productString=
 * Returns      : 1=success\0=fail
 * void zapNodeNetworkStatusNotify(uint8_t status,
											char *ieee,
											const char *netType,
											uint8_t deviceType,
											uint8_t rssi,
											uint8_t lqi,
											uint8_t firmwareVer,
											uint8_t productString);
*******************************************************************************/
void zapNodeNetworkStatusNotify(uint8_t status,
    char *ieee,
    const char *netType,
    uint8_t deviceType,
    uint8_t rssi,
    uint8_t lqi,
    uint8_t firmwareVer,
    uint8_t productString)
{
char ieeeStr[17];
char netTypeStr[17];
Array2HexString(ieee, 8, ieeeStr, sizeof(ieeeStr));
Array2HexString(netType, 8, netTypeStr, sizeof(netTypeStr));

jsonrpc_notify_client(self,
"zapNodeNetworkStatusNotify",
"issiiiii",
(double)status,
ieeeStr,
netTypeStr,
(double)deviceType,
(double)rssi,
(double)lqi,
(double)firmwareVer,
(double)productString
);
}
jsonrpc_error_t server_zapNodeNetworkStatusNotify (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{
uint8_t ieee[8];
uint8_t netType[8];
int ieeeLen = sizeof(ieee);
int netTypeLen = sizeof(netType);
double status;
double deviceType;
double rssi;
double lqi;
double firmwareVer;
double productString;

if( (argc != 8) ||
(argv[0].json.type != JSONRPC_TYPE_NUMBER) ||
(argv[1].json.type != JSONRPC_TYPE_STRING) ||
(argv[2].json.type != JSONRPC_TYPE_STRING) ||
(argv[3].json.type != JSONRPC_TYPE_NUMBER) ||
(argv[4].json.type != JSONRPC_TYPE_NUMBER) ||
(argv[5].json.type != JSONRPC_TYPE_NUMBER) ||
(argv[6].json.type != JSONRPC_TYPE_NUMBER) ||
(argv[7].json.type != JSONRPC_TYPE_NUMBER) )
{
return JSONRPC_ERROR_INVALID_PARAMS;
}
HexString2Array(argv[1].json.u.string, ieee, &ieeeLen);
HexString2Array(argv[2].json.u.string, netType, &netTypeLen);
if(ieeeLen != 8)
{
return JSONRPC_ERROR_INVALID_PARAMS;
}
if(netTypeLen != 8)
{
return JSONRPC_ERROR_INVALID_PARAMS;
}
status = argv[0].json.u.number;
deviceType = argv[3].json.u.number;
rssi = argv[4].json.u.number;
lqi = argv[5].json.u.number;
firmwareVer = argv[6].json.u.number;
productString = argv[7].json.u.number;

zapNodeNetworkStatusNotify((uint8_t) status,
                     ieee,
                     netType,
                     (uint8_t) deviceType,
                     (uint8_t) rssi,
                     (uint8_t) lqi,
                     (uint8_t) firmwareVer,
                     (uint8_t) productString
                     );
return JSONRPC_ERROR_OK;
}


/******************************************************************************
 * FunctionName : zapNetworkStatusNotify
 * Description  :
 * Parameters   : status=1 NETWORK_UP/status = 0 NETWORK_DOWN
 * Returns      : 1=success\0=fail
 * void zapNetworkStatusNotify(uint8_t status);
*******************************************************************************/
void zapNetworkStatusNotify(uint8_t status)
{
	jsonrpc_notify_client(self,
						"zapNodeNetworkStatusNotify",
						"i",
						(double)status
						);
}
jsonrpc_error_t server_zapNetworkStatusNotify (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{


			double status;

			if( (argc != 1) ||
						(argv[0].json.type != JSONRPC_TYPE_NUMBER)  )
				{
					return JSONRPC_ERROR_INVALID_PARAMS;
				}
				 status = argv[0].json.u.number;
				 zapNetworkStatusNotify((uint8_t) status);
				 											
		return JSONRPC_ERROR_OK;
}


DEFINE_LOGGER(jsonrpc,100,LOG_TRACE);

void myputchar(char_t c)
{
	putchar(c);
}


static const LoggerConfig_ST s_myLoggerConfig =
{
		NULL,myputchar,NULL,NULL,0,0,8,TRUE
};

int main (int argc, const char * argv[])
{

	Logger_Init(&s_myLoggerConfig);
	Logger_SetGlobalLevel(LOG_DEBUG);



	jsonrpc_error_t error;
	int client;
	int port;
	const char * serverAddr;

	setbuf(stdout,NULL);
	Logger_Init(&s_myLoggerConfig);
	Logger_SetGlobalLevel(LOG_DEBUG);

	if(argc <= 1)
	{
		client = 0;
		port = 8212;
	}
	else
	{
		client = (argv[1][0]=='0')?0:1;
		if(argc > 2)
		{
			port = atoi(argv[2]);
			if(argc > 3)
			{
				serverAddr = argv[3];
			}
		}
	}

	setbuf(stdout,NULL);
	LogDebug("enter the main\n");
	if(client != 0)
	{
		printf("create birpc client used port %d connect server %s\n", port, serverAddr);
		self = jsonrpc_server_open(
				client,
				port,
				serverAddr
		);
	}
	else
	{
		printf("create birpc server used port %d\n", port);
		self = jsonrpc_server_open(
				client,
				port
		);
		
		error  = jsonrpc_server_register_method(self, JSONRPC_TRUE, server_zapFormNetwork, "zapFormNetwork",  "siii ");
		error  = jsonrpc_server_register_method(self, JSONRPC_TRUE,  server_zapLeaveNetwork, "zapLeaveNetwork", "s");
		error  = jsonrpc_server_register_method(self, JSONRPC_TRUE,  server_zapPermitJion, "zapPermitJion", "i");
		error  = jsonrpc_server_register_method(self, JSONRPC_TRUE,  server_zapRemoveDevice, "zapRemoveDevice", "s");
		error  = jsonrpc_server_register_method(self, JSONRPC_TRUE,  server_zapSetChannel, "zapSetChannel", "i");
		error  = jsonrpc_server_register_method(self, JSONRPC_TRUE, server_zapSendUnicast, "zapSendUnicast", "siiiisb");
		error  = jsonrpc_server_register_method(self, JSONRPC_TRUE,  server_zapSentNotify, "zapSentNotify",   "siiiis ");
		error  = jsonrpc_server_register_method(self, JSONRPC_TRUE,  server_zapNewPacketNotify, "zapNewPacketNotify",   "siiiis ");
		error  = jsonrpc_server_register_method(self, JSONRPC_TRUE,  server_zapNodeNetworkStatusNotify, "zapNodeNetworkStatusNotify",   "issiiiii ");
		error  = jsonrpc_server_register_method(self, JSONRPC_TRUE,  server_zapNetworkStatusNotify, "zapNetworkStatusNotify", "i");
		//error  = jsonrpc_server_register_method(self, JSONRPC_TRUE,  server_zapTick, "zapTick", NULL);
		//error  = jsonrpc_server_register_method(self, JSONRPC_TRUE,  server_zapInit, "zapInit", NULL);
            
		
	}


	for (;;)
	{
		error = jsonrpc_server_run(self, 0);
		if(error == JSONRPC_ERROR_OK)
		{
			zapSentNotify ("abcd",
				  4,
				  5,
				  6,
				  7,
				  4,
				  "ehgh");
		}
		//sleep(1);
	}
	jsonrpc_server_close(self);

	return 0;
}

