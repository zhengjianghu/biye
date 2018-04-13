/**
 * \date 2017-12-5 soso 新建
**/

#ifndef NCP_H
#define NCP_H
/*standard type define*/
#include "Platform.h"

/******************************************************************************
 * FunctionName : zapFormNetwork
 * Description  : 组建zigbee网络
 * Parameters   : netType=网络类型,字符举类型 如 "control4"
 				: panId
 				: radioTxPower
 				: radioChannel
 * Returns      : 1=success\0=fail
*******************************************************************************/
uint8_t zapFormNetwork(const char *netType,
							uint16_t panId,
							int8_t radioTxPower,
							uint8_t radioChannel);
/******************************************************************************
 * FunctionName : zapLeaveNetwork
 * Description  : 删除zigbee网络
 * Parameters   : netType=网络类型,字符举类型 如 "control4"
 * Returns      : 1=success\0=fail
*******************************************************************************/
uint8_t zapLeaveNetwork(const char *netType);
/******************************************************************************
 * FunctionName : zapPermitJion
 * Description  : 在窗口时间内允许入网
 * Parameters   : seconds=窗口时间
 * Returns      : 1=success\0=fail
*******************************************************************************/
uint8_t zapPermitJion(uint8_t seconds);
/******************************************************************************
 * FunctionName : zapRemoveDevice
 * Description  : 删除远端设备
 * Parameters   : 参数待定
 * Returns      : 1=success\0=fail
*******************************************************************************/
uint8_t zapRemoveDevice(uint8_t ieee[8]);
/******************************************************************************
 * FunctionName : zapSetChannel
 * Description  : 设置通道参数
 * Parameters   : channel 通道参数
 * Returns      : 1=success\0=fail
*******************************************************************************/
uint8_t zapSetChannel(uint8_t channel);
/******************************************************************************
 * FunctionName : zapSendUnicast
 * Description  : 单播发送
 * Parameters   : ieee=64位地址
 				: profileId=配置文件ID
 				: clusterId=集群ID
 				: sourceEndpoint=源端点号
 				: destinationEndpoint=目的端点号
 				: messageLength=消息长度
 				: message=消息缓存
 				: needConfirm=是否需要回复应答标识
 * Returns      : messageTag=报文序列号
*******************************************************************************/
uint8_t zapSendUnicast(uint8_t ieee[8],
							uint16_t profileId,
                      		uint16_t clusterId,
                      		uint8_t sourceEndpoint,
                      		uint8_t destinationEndpoint,
                      		uint8_t messageLength,
                      		uint8_t* message,
                      		uint8_t needConfirm);
/******************************************************************************
 * FunctionName : zapSentCallback
 * Description  : 报文发送确认回调函数
 					---ezspMessageSentHandler
 * Parameters   : 同zapSendUnicast
 * Returns      : 1=success\0=fail
*******************************************************************************/
void zapSentNotify(uint8_t ieee[8],
						uint16_t profileId,
						uint16_t clusterId,
						uint8_t sourceEndpoint,
						uint8_t destinationEndpoint,
						uint8_t messageLength,
						uint8_t* message,
						uint8_t result);

/******************************************************************************
 * FunctionName : zapNewPacketNotify
 * Description  : 接受报文回调函数
 					---ezspIncomingMessageHandler
 * Parameters   : ieee=64位地址
 				: profileId=配置文件ID
 				: clusterId=集群ID
 				: sourceEndpoint=源端点号
 				: destinationEndpoint=目的端点号
 				: messageLength=消息长度
 				: message=消息缓存
 * Returns      : 1=success\0=fail
*******************************************************************************/
void zapNewPacketNotify(uint8_t ieee[8],
							uint16_t profileId,
                      		uint16_t clusterId,
                      		uint8_t sourceEndpoint,
                      		uint8_t destinationEndpoint,
                      		uint8_t messageLength,
                      		uint8_t* message);
/******************************************************************************
 * FunctionName : zapNodeNetworkStatusNotify
 * Description  : 节点入网状态变化通知
 					---ezspTrustCenterJoinHandler 
 					---C4IncomingMessageHandler 
 * Parameters   ：status 0 = off line/1 = on line
 				: ieee=64位地址
 				: netType=网络类型        如 "control4"
 				: deviceType=设备类型
 				: rssi=
 				: lqi=
 				: firmwareVer=固件版本号
 				: productString= 
 * Returns      : 1=success\0=fail
*******************************************************************************/
void zapNodeNetworkStatusNotify(uint8_t status,
											uint8_t ieee[8],
											const char *netType,
											uint8_t deviceType,
											int8_t rssi,
											uint8_t lqi,
											char *firmwareVer,
											char *productString);
/******************************************************************************
 * FunctionName : zapNetworkStatusNotify
 * Description  : 
 					---ezspStackStatusHandler
 * Parameters   : status=1 NETWORK_UP/status = 0 NETWORK_DOWN
 * Returns      : 1=success\0=fail
*******************************************************************************/
void zapNetworkStatusNotify(uint8_t status);


/******************************************************************************
 * FunctionName : 
 * Description  : 
 * Parameters   : 
 * Returns      : 1=success\0=fail
*******************************************************************************/
void zapTick(void);
/******************************************************************************
 * FunctionName : 
 * Description  : 
 * Parameters   : 
 * Returns      : 1=success\0=fail
*******************************************************************************/
void zapInit(void);
/******************************************************************************
 * FunctionName : 
 * Description  : 
 * Parameters   : 
 * Returns      : 1=success\0=fail
*******************************************************************************/
void zapSendMTORR(void);
uint8_t zapNetworkInit(void);



#if 0//待定
/******************************************************************************
 * FunctionName : 
 * Description  : 
 * Parameters   : 
 * Returns      : 1=success\0=fail
*******************************************************************************/
uint8_t zapNetworkStatusNotify(uint8_t status);
/******************************************************************************
 * FunctionName : 
 * Description  : 
 * Parameters   : 
 * Returns      : 1=success\0=fail
*******************************************************************************/
uint8_t zapRemoveDeviceNotify(uint8_t status);//ok?
/******************************************************************************
 * FunctionName : 
 * Description  : 
 * Parameters   : 
 * Returns      : 1=success\0=fail
*******************************************************************************/
uint8_t zapSetChannelNotify(uint8_t status);//ok?
#endif


#endif
