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

#include "../plugins/jsonrpc_plugin_cjson.h"
#include "../plugins/jsonrpc_plugin_tcp.h"

/******************************************************************************
 * FunctionName : zapFormNetwork
 * Description  : 组建zigbee网络
 * Parameters   : netType=网络类型,字符举类型 如 "control4"
 				: panId
 				: radioTxPower
 				: radioChannel
 * Returns      : 1=success\0=fail
 * uint8_t zapFormNetwork(const char *netType,
							int16u panId,
							int8s radioTxPower,
							int8u radioChannel);
*******************************************************************************/
jsonrpc_error_t zapFormNetwork(int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{
	printf("%s(argc:%d)\n", __FUNCTION__, argc);
	print_result(ctx, "hello，this is the function zapFormNetwork\n");
	return JSONRPC_ERROR_OK;
}

/******************************************************************************
 * FunctionName : zapLeaveNetwork
 * Description  : 删除zigbee网络
 * Parameters   : netType=网络类型,字符举类型 如 "control4"
 * Returns      : 1=success\0=fail
 * uint8_t zapLeaveNetwork(const char *netType);
*******************************************************************************/
jsonrpc_error_t zapLeaveNetwork (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{
	printf("%s(argc:%d)\n", __FUNCTION__, argc);
	print_result(ctx, "hello，this is the function zapLeaveNetwork\n");
	return JSONRPC_ERROR_OK;
}
/******************************************************************************
 * FunctionName : zapPermitJion
 * Description  : 在窗口时间内允许入网
 * Parameters   : seconds=窗口时间
 * Returns      : 1=success\0=fail
 * uint8_t zapPermitJion(uint8_t seconds);
*******************************************************************************/

jsonrpc_error_t zapPermitJion (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{
	printf("%s(argc:%d)\n", __FUNCTION__, argc);
	print_result(ctx, "hello，this is the function zapPermitJion\n");
	return JSONRPC_ERROR_OK;
}
/******************************************************************************
 * FunctionName : zapRemoveDevice
 * Description  : 删除远端设备
 * Parameters   : 参数待定
 * Returns      : 1=success\0=fail
 * uint8_t zapRemoveDevice(char *ieee);
*******************************************************************************/
jsonrpc_error_t zapRemoveDevice (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{
	printf("%s(argc:%d)\n", __FUNCTION__, argc);
	print_result(ctx, "hello，this is the function zapRemoveDevice\n");
	return JSONRPC_ERROR_OK;
}
/******************************************************************************
 * FunctionName : zapSetChannel
 * Description  : 设置通道参数
 * Parameters   : channel 通道参数
 * Returns      : 1=success\0=fail
 * uint8_t zapSetChannel(uint8_t channel);
*******************************************************************************/
jsonrpc_error_t zapSetChannel (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{
	printf("%s(argc:%d)\n", __FUNCTION__, argc);
	print_result(ctx, "hello，this is the function zapSetChannel\n");
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

	if( (argc != 7) &&
			(argv[0].json.type != JSONRPC_TYPE_STRING) &&
			(argv[1].json.type != JSONRPC_TYPE_NUMBER) &&
			(argv[2].json.type != JSONRPC_TYPE_NUMBER) &&
			(argv[3].json.type != JSONRPC_TYPE_NUMBER) &&
			(argv[4].json.type != JSONRPC_TYPE_NUMBER) &&
			(argv[5].json.type != JSONRPC_TYPE_STRING) &&
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

#define NUM_TO_HEX_CHAR(num)	(num)>9

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
/******************************************************************************
 * FunctionName : zapSentCallback
 * Description  : 报文发送确认回调函数
 * Parameters   : 同zapSendUnicast
 * Returns      : 1=success\0=fail
 * void zapSentNotify(char *ieee,
						uint16_t profileId,
                  		uint16_t clusterId,
                  		uint8_t sourceEndpoint,
                  		uint8_t destinationEndpoint,
                  		uint8_t messageLength,
                  		uint8_t* message);
*******************************************************************************/
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
void zapNewPacketNotify(char *ieee,
							uint16_t profileId,
                      		uint16_t clusterId,
                      		uint8_t sourceEndpoint,
                      		uint8_t destinationEndpoint,
                      		uint8_t messageLength,
                      		uint8_t* message)
{


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
											int8u deviceType,
											int8s rssi,
											int8u lqi,
											int8u firmwareVer,
											int8u productString);
*******************************************************************************/
jsonrpc_error_t zapNodeNetworkStatusNotify (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{
	printf("%s(argc:%d)\n", __FUNCTION__, argc);
	print_result(ctx, "hello，this is the function zapNodeNetworkStatusNotify\n");
	return JSONRPC_ERROR_OK;
}
/******************************************************************************
 * FunctionName : zapNetworkStatusNotify
 * Description  :
 * Parameters   : status=1 NETWORK_UP/status = 0 NETWORK_DOWN
 * Returns      : 1=success\0=fail
 * void zapNetworkStatusNotify(uint8_t status);
*******************************************************************************/
jsonrpc_error_t zapNetworkStatusNotify (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{
	printf("%s(argc:%d)\n", __FUNCTION__, argc);
	print_result(ctx, "hello，this is the function zapNetworkStatusNotify\n");
	return JSONRPC_ERROR_OK;
}
/******************************************************************************
 * FunctionName :
 * Description  :
 * Parameters   :
 * Returns      : 1=success\0=fail
 * void zapTick(void);
*******************************************************************************/
jsonrpc_error_t zapTick (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{
	printf("%s(argc:%d)\n", __FUNCTION__, argc);
	print_result(ctx, "hello，this is the function zapTick\n");
	return JSONRPC_ERROR_OK;
}
/******************************************************************************
 * FunctionName :
 * Description  :
 * Parameters   :
 * Returns      : 1=success\0=fail
 * void zapInit(void);
*******************************************************************************/
jsonrpc_error_t zapInit (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{
	printf("%s(argc:%d)\n", __FUNCTION__, argc);
	print_result(ctx, "hello，this is the function zapInit\n");
	return JSONRPC_ERROR_OK;

}

jsonrpc_error_t subtract (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{
	double r, f;

	printf("%s(argc:%d)\n", __FUNCTION__, argc);

	r = argv[0].json.u.number - argv[1].json.u.number;
	f = fmod(r, 1.0);
	if (f == 0.0)
		print_result(ctx, "%.0lf", r);
	else
		print_result(ctx, "%lf", r);
	return JSONRPC_ERROR_OK;
}

jsonrpc_error_t sum (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{
	double r = 0.0, f;

	printf("%s(argc:%d)\n", __FUNCTION__, argc);

	while (argc--)
	{
		r += argv[argc].json.u.number;
	}

	f = fmod(r, 1.0);
	if (f == 0.0)
		print_result(ctx, "%.0lf", r);
	else
		print_result(ctx, "%lf", r);
	return JSONRPC_ERROR_OK;
}

jsonrpc_error_t update (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{
	printf("%s(argc:%d)\n", __FUNCTION__, argc);

	return JSONRPC_ERROR_OK;
}




jsonrpc_error_t foobar (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{
	printf("%s(argc:%d)\n", __FUNCTION__, argc);

	return JSONRPC_ERROR_OK;
}


jsonrpc_error_t test_param (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{
	printf("%s(argc:%d)\n", __FUNCTION__, argc);

	printf("\t%c:%s\n", (char)argv->json.type, argv->json.u.string);
	argv++;
	printf("\t%c:%d\n", (char)argv->json.type, argv->json.u.boolean);
	argv++;
	printf("\t%c:0x%X\n", (char)argv->json.type, argv->json.u.object);
	argv++;

	argv++;
	printf("\t%c:%lf\n", (char)argv->json.type, argv->json.u.number);
	argv++;

	return JSONRPC_ERROR_OK;
}

jsonrpc_error_t and (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{
	printf(" function name:\n%s(argc:%d)\n", __FUNCTION__,argc);			//*****自动获取函数名并将其输出*****//

	int a = 0;

	a = argv[0].json.u.number + argv[1].json.u.number;

	print_result(ctx, "%d", a);


	return JSONRPC_ERROR_OK;

}


jsonrpc_error_t multiply (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{
	double r, f;

	printf("%s(argc:%d)\n", __FUNCTION__, argc);

	r = argv[0].json.u.number * argv[1].json.u.number;
	f = fmod(r, 1.0);
	if (f == 0.0)
		print_result(ctx, "%.0lf", r);
	else
		print_result(ctx, "%lf", r);
	return JSONRPC_ERROR_OK;
}



jsonrpc_error_t get_data (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{
	printf("%s(argc:%d)\n", __FUNCTION__, argc);

	print_result(ctx, "{\"data\":\"abcde\"}");
	return JSONRPC_ERROR_OK;
}

static const LoggerConfig_ST s_myLoggerConfig =
{
		NULL,putchar,NULL,NULL,0,0,8,TRUE
};

int main (int argc, const char * argv[])
{
	jsonrpc_handle_t birpc;
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
	printf("enter the main\n");
	if(client != 0)
	{
		printf("create birpc client used port %d connect server %s\n", port, serverAddr);
		birpc = jsonrpc_server_open(
				jsonrpc_plugin_cjson(),
				jsonrpc_plugin_tcp(),
				client,
				port,
				serverAddr
		);
	}
	else
	{
		printf("create birpc server used port %d\n", port);
		birpc = jsonrpc_server_open(
				jsonrpc_plugin_cjson(),
				jsonrpc_plugin_tcp(),
				client,
				port
		);
		//******当调用函数需要有回调值时，注册为：JSONRPC_TRUE，当不需要返回调用值时：JSONRPC_FALSE,并且请求中也不需要带id编号*******//
		//******没有参数的就直接省去“params”：“**”这一部分****//
		//******当参数部分有多个数字是，需要用到【】来记录具体的数字****//
		//*****当批量调用是，批量的请求需要用【】记录****//
		error  = jsonrpc_server_register_method(birpc, JSONRPC_TRUE, subtract, "subtract", "minuend:i, subtrahend:i");
		error  = jsonrpc_server_register_method(birpc, JSONRPC_TRUE, sum, "sum", "iii");
		error  = jsonrpc_server_register_method(birpc, JSONRPC_FALSE, update, "update", "iiiii");	//***//***{"jsonrpc": "2.0", "method": "update", "params": [1,2,3,4,5]}**//
		error  = jsonrpc_server_register_method(birpc, JSONRPC_FALSE, foobar, "foobar", NULL);	//****{"jsonrpc": "2.0", "method": "foobar"}**//
		error  = jsonrpc_server_register_method(birpc, JSONRPC_TRUE, get_data, "get_data", NULL);//**{"jsonrpc": "2.0", "method": "get_data", "id": "9"}***//
		error  = jsonrpc_server_register_method(birpc, JSONRPC_FALSE, test_param, "test_param", "sboai");
		//***and函数：不需要返回，只是服务器端打印字符即可****//
		error  = jsonrpc_server_register_method(birpc, JSONRPC_TRUE, and, "and", "ii");
		error  = jsonrpc_server_register_method(birpc, JSONRPC_TRUE, multiply, "multiply", "ii");
		//**ZAP功能注册函数**//
		//error  = jsonrpc_server_register_method(birpc, JSONRPC_TRUE, zapFormNetwork, "zapFormNetwork",  "(char *netType):i,   panId:i，  radioTxPower:i,  radioChannel:i  ");
		//error  = jsonrpc_server_register_method(birpc, JSONRPC_TRUE, zapLeaveNetwork, "zapLeaveNetwork", "(*netType)");
		error  = jsonrpc_server_register_method(birpc, JSONRPC_TRUE, zapPermitJion, "zapPermitJion", "i");
		//error  = jsonrpc_server_register_method(birpc, JSONRPC_TRUE, zapRemoveDevice, "zapRemoveDevice", "(char *ieee)");
		error  = jsonrpc_server_register_method(birpc, JSONRPC_TRUE, zapSetChannel, "zapSetChannel", "i");
		//error  = jsonrpc_server_register_method(birpc, JSONRPC_TRUE, zapSendUnicast, "zapSendUnicast",   "(char *ieee):i,   profileId:i，  clusterId:i,  sourceEndpoint:i,  destinationEndpoint:i,  messageLength:i,  (uint8_t* message):i,  needConfirm:b  ");
		//error  = jsonrpc_server_register_method(birpc, JSONRPC_TRUE, zapSentNotify, "zapSentNotify",   "(char *ieee):i,   profileId:i，  clusterId:i,  sourceEndpoint:i,  destinationEndpoint:i,  messageLength:i,  (uint8_t* message):i ");
		//error  = jsonrpc_server_register_method(birpc, JSONRPC_TRUE, zapNewPacketNotify, "zapNewPacketNotify",   "(char *ieee):i,   profileId:i，  clusterId:i,  sourceEndpoint:i,  destinationEndpoint:i,  messageLength:i,  (uint8_t* message):i ");
		//error  = jsonrpc_server_register_method(birpc, JSONRPC_TRUE, zapNodeNetworkStatusNotify, "zapNodeNetworkStatusNotify",   "status:i,   (char *ieee):i，  (const char *netType):i,  deviceType:i,  rssi:i,  lqi:i, firmwareVer:i  productString:i ");
		error  = jsonrpc_server_register_method(birpc, JSONRPC_TRUE, zapNetworkStatusNotify, "zapNetworkStatusNotify", "i");
		error  = jsonrpc_server_register_method(birpc, JSONRPC_TRUE, zapTick, "zapTick", NULL);
		error  = jsonrpc_server_register_method(birpc, JSONRPC_TRUE, zapInit, "zapInit", NULL);
	}

	for (;;)
	{
		jsonrpc_server_run(birpc, 0);
		//sleep(1);
	}
	jsonrpc_server_close(birpc);

	return 0;
}

