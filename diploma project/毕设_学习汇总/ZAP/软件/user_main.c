/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */


/*

                   _ooOoo_
                  o8888888o
                  88" . "88
                  (| -_- |)
                  O\  =  /O
               ____/`---'\____
             .'  \\|     |//  `.
            /  \\|||  :  |||//  \
           /  _||||| -:- |||||-  \
           |   | \\\  -  /// |   |
           | \_|  ''\---/''  |   |
           \  .-\__  `-`  ___/-. /
         ___`. .'  /--.--\  `. . __
      ."" '<  `.___\_<|>_/___.'  >'"".
     | | :  `- \`.;`\ _ /`;.`/ - ` : | |
     \  \ `-.   \_ __\ /__ _/   .-` /  /
======`-.____`-.___\_____/___.-`____.-'======
                   `=---='
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
         佛祖保佑       永无BUG

*/
//          佛曰:   

//          写字楼里写字间，写字间里程序员；   

//          程序人员写程序，又拿程序换酒钱。   

//          酒醒只在网上坐，酒醉还来网下眠；   

//          酒醉酒醒日复日，网上网下年复年。   

//          但愿老死电脑间，不愿鞠躬老板前；   

//          奔驰宝马贵者趣，公交自行程序员。   

//          别人笑我忒疯癫，我笑自己命太贱；   

//          不见满街漂亮妹，哪个归得程序员？



#include "esp_common.h"
#include "assert.h"
#include "gpio.h"
#include "uart.h"
#include "flash.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "espressif/espconn.h"
#include "espressif/airkiss.h"
#include "lwip/mdns.h"

#include <stdarg.h>
#include "Logger.h"
#include "Sddp.h"
#include "Jsonrpc.h"

#include "freertos/timers.h"


#include "Ringbuf16.h"

DEFINE_LOGGER(MAIN, 128, LOG_DEBUG);

/**
*   log相关参数 
**/
void GetTime(char_t * s, int32_t maxlen);
uint32 halCommonGetInt32uMillisecondTick(void);
void resetZap(void);


static uint8_t gNetWorkStatus = 0;

static const LoggerConfig_ST s_myLoggerConfig = 
{
    NULL,os_putc,GetTime,NULL,10,0,4,TRUE
};


void GetTime(char_t * s, int32_t maxlen)
{
    static char_t timestr[32];
    uint32_t now;
    now = halCommonGetInt32uMillisecondTick();
    snprintf(timestr, sizeof(timestr), "%6d.%03d", now/1000, now%1000);
    strncpy(s, timestr, maxlen);

    // static char_t timestr[32];
    // uint16_t timelen = 0;
    // time_t timep; 
    // timep=time (&timep); 
    // strncpy(timestr,ctime(&timep),sizeof(timestr)-1);//除去ctime函数返回的字符串中的 “\n”
    // timestr[sizeof(timestr)-1] = 0;
    // timelen = strnlen(timestr, sizeof(timestr));
    // timestr[timelen-1] = 0;
    // return(timestr);
}

// char_t * GetThreadName(void)
// {
//     return pcTaskGetTaskName(NULL);
// }

/**
*   log相关参数 end
**/

/**
*   sddp相关参数 
**/
typedef struct
{
    const char * sddp_Type;
    const char * sddp_PrimaryProxy;
    const char * sddp_Proxies;
    const char * sddp_Manufacturer;
    const char * sddp_Model;
    const char * sddp_Driver;
    int sddp_MaxAge;
}SDDPConfig;

const SDDPConfig sddpConfig = 
{
    "inSona:Air_Quality_Sensor", 
    "thermostatV2", 
    "thermostatV2",
    "inSona", 
    "inSona_Air_Quality_Sensor", 
    "inSona_Air_Quality_Sensor.c4z", 
    1800
};


/**
*   sddp相关参数 end
**/





/**
 *  sleep(sec)休眠 x sec 
 *  if VTaskDelay()
 *  函数使能开关为0则sleep函数无效等价延时10000*sec个空指令执行时间
 *  函数使能开关为1则sleep函数生效                      
 **/






/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 user_rf_cal_sector_set(void)
{
    flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}
#if 0
typedef struct
{
	uint8_t flag;

	
	uint16_t pandid;
	int8_t txpower;
	uint8_t channel;	
}user_parameter;
static user_parameter my_par;
void write_parameter(void)
{
	u8 err = 0;
	err = flash_write(PARA_ADDR,(u8 *)&my_par,sizeof(my_par));
}

void read_parameter(void)
{
	u8 err = 0;
	err = flash_readx(PARA_ADDR,(u8 *)&my_par,sizeof(my_par));
}
#endif

void
_xassert(const char *file, int lineno)
{
  os_delay_us(10000);
  LogDebug("!!Assert failed:!! ");
  os_delay_us(10000);
  LogDebug("%s, line %d.\n", file, lineno);
  /*
   * loop for a while;
   * call _reset_vector__();
   */
}

void simulatedTimePasses(void)
{
    taskYIELD();
}

void ashSerialWriteByte(uint8 byte)
{
    uart_tx_one_char(UART0, byte);
}

uint8 ashSerialReadByte(uint8 *byte)
{
    int c = uart0_getchar();

if(c != -1){
    if((c == 0x11)||(c == 0x13))
    {
        return 0x27;
    }
    else
    {
        *byte = c;
        return 0;
    }
 } else {
    return 0x27;
}
}
uint8 ashSerialInit(void)
{
    LogDebug("ashSerialInit\n");
	uart0_init();
    return 0;
}

void ashSerialClose(void)
{
}
void ashSerialWriteFlush(void)
{
}

void ashSerialReadFlush(void)
{
    int c;
    LogDebug("ashSerialReadFlush\n");
    do
    {
     c = uart0_getchar();
    }while(c != -1);
} 

uint8 ashSerialWriteAvailable(void)
{
    uint8 ret = uart_tx_avaliable(UART0);
    if(ret == 0)
    {
        //LogDebug("ashSerialWriteAvailable OK\n");
    }
    else
    {
        LogDebug("ashSerialWriteAvailable FAIL\n");
    }
    return ret;
}

//Use stdlib random numbers.
uint16_t halCommonGetRandom(void)
{
  return (uint16_t)os_random();
}

uint32 halCommonGetInt32uMillisecondTick(void)
{
    static uint32 ts_hi = 0;
    static uint32 last_ts_lo = 0;
    uint32 ms = 0;
    uint32 us = system_get_time();
    
    if(us < last_ts_lo)
    {
        ts_hi++;
    }
    last_ts_lo = us;
    ms = (uint32)((((uint64)ts_hi<<32)+us)/1000);
    return ms;
}

int _gettimeofday_r(struct _reent *ptr,
		struct timeval *ptimeval,
		void *ptimezone)
{
  uint32 us = system_get_time();
  uint64 epoch = 1144473600000000ULL + us;
  if(ptimeval)
  {
  ptimeval->tv_sec = epoch/1000000;
  ptimeval->tv_usec = epoch%1000000;
  }
  if(ptimezone)
  {
  *(time_t*)ptimezone = 8*3600;
  }
  if(ptr)
  {
  ptr->_errno = 0;
  }
  return 0;
}

int32_t sleep(uint32_t sec)
{
    vTaskDelay(sec * configTICK_RATE_HZ);
    return 0;
}


void airkiss_init(void);


void ICACHE_FLASH_ATTR
server_task(void * para);
void ICACHE_FLASH_ATTR
client_task(void * para);
void user_mdns_config();
void ICACHE_FLASH_ATTR
jsonrpc_task(void * para);


void WaitConnected(void)
{
    struct ip_info ip_config;
    wifi_get_ip_info(STATION_IF, &ip_config);
    while(ip_config.ip.addr == 0){
        vTaskDelay(1000 / portTICK_RATE_MS);
        wifi_get_ip_info(STATION_IF, &ip_config);
        LogDebug("wait connected...\n");
    }
    LogDebug("connected %s\n",inet_ntoa(ip_config.ip.addr));
}

void PrintInfo(void)
{
    uint8 mac[6];
    char_t hostname[32];
    wifi_get_macaddr(STATION_IF, mac);

    snprintf(hostname,32,"ZAP-%02X%02X%02X", mac[3], mac[4], mac[5]);
    wifi_station_set_hostname(hostname);

    LogDebug("mac:\"%02x-%02x-%02x-%02x-%02x-%02x\"\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    LogDebug("hostname:\"%s\"\n", wifi_station_get_hostname());
}


#if 0
Ringbuf16_ST mysnd;
void zapNewPacketNotify (uint8_t ieee[8],
								uint16_t profileId,
								uint16_t clusterId,
								uint8_t sourceEndpoint,
								uint8_t destinationEndpoint,
								uint8_t messageLength,
								uint8_t* message)
{
	int recv_data = 0;
	LogInfo( "zapNewPacketNotify**********\n" );

	unsigned char	* ptr	= Ringbuf16_GetPutPtr( &mysnd);                                                          /* ***允许写入的最大长度**** // */
	int		maxlen	= Ringbuf16_GetPutLength( &mysnd, TRUE );

	if(messageLength < maxlen)
	{
		memset(ptr,'X',messageLength);
		Ringbuf16_PutPtrInc(&mysnd, messageLength); 
	}
	else
	{
		memset(ptr,'X',maxlen);
		Ringbuf16_PutPtrInc(&mysnd, maxlen); 
	}
	

}
#endif


void ICACHE_FLASH_ATTR
SDDP_task(void * para)
{
    SDDPHandle handle;
    int identify_signal = 0;

    LogInfo("sddptask\n");

	WaitConnected();
	PrintInfo();

    // Set up SDDP library
    SDDPInit(&handle);

    SDDPSetDevice(handle, 0, 
            sddpConfig.sddp_Type,
            sddpConfig.sddp_PrimaryProxy,
            sddpConfig.sddp_Proxies,
            sddpConfig.sddp_Manufacturer,
            sddpConfig.sddp_Model,
            sddpConfig.sddp_Driver,
            sddpConfig.sddp_MaxAge, 
            false);
    
    // Initialize the SDDP host name
    SDDPSetHost(handle, wifi_station_get_hostname());

    // Start up SDDP
    SDDPStart(handle);
    // Run SDDP loop
    while(1)
    {
        // Call SDDP tick to process incoming and outgoing SDDP messages
        SDDPTick(handle);

        if (identify_signal)
		{
			// Send identify
			LogInfo("Sending SDDP identify\n");

			SDDPIdentify(handle);
			SDDPIdentify(handle);

			identify_signal = 0;
		}
        //taskYIELD();
		vTaskDelay(100);
    }        
    // Shutdown SDDP
    SDDPShutdown(&handle);

    vTaskDelete(NULL);
} // End of main




void ICACHE_FLASH_ATTR
server_task(void * para)
{
    LogDebug("servertask\n");
    user_socket_config();
    //user_mdns_config();
    vTaskDelete(NULL);
}

struct mdns_info info;
void user_mdns_config()
{
    struct ip_info ipconfig;
    wifi_get_ip_info(STATION_IF, &ipconfig);
    memset(&info, 0, sizeof(info));
    info.host_name =	"espressif";
    info.ipAddr = ipconfig.ip.addr; //ESP8266	station	IP
    info.server_name =	"iot";
    info.server_port = 8080;
    info.txt_data[0] =	"version = now";
    info.txt_data[1] =	"user1 = data1";
    info.txt_data[2] =	"user2 = data2";
    mdns_init(&info);
}

int sc;
#define PORT 30001
#define BACKLOG 3
int user_socket_config()
{
    struct sockaddr_in server_addr; //存储服务器端socket地址结构    
    struct sockaddr_in client_addr; //存储客户端 socket地址结构    
	int ss;
	int err;
    
    
    /*****************socket()***************/    
    ss = socket(AF_INET,SOCK_STREAM,0); //建立一个序列化的，可靠的，双向连接的的字节流    
    if(ss<0)    
    {    
        LogError("server : server socket create error\n");    
        return -1;    
    }
        LogDebug("server : server socket create ok\n");    

    /******************bind()****************/    
    //初始化地址结构    
    memset(&server_addr,0,sizeof(server_addr));    
    server_addr.sin_family = AF_INET;           //协议族    
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);   //本地地址    
    server_addr.sin_port = htons(PORT);    
    
    err = bind(ss,(struct sockaddr *)&server_addr,sizeof(struct sockaddr));    
    if(err<0)    
    {    
        LogError("server : bind error\n");
        return -1;
    }
        LogDebug("server : bind ok\n");
    
    /*****************listen()***************/    
    err = listen(ss,BACKLOG);   //设置监听的队列大小    
    if(err < 0)    
    {    
        LogError("server : listen error\n");    
        return -1;    
    }
        LogDebug("server : listen ok\n");    

    /****************accept()***************/    
    /**  
    为类方便处理，我们使用两个进程分别管理两个处理：  
    1，服务器监听新的连接请求;2,以建立连接的C/S实现通信  
    这两个任务分别放在两个进程中处理，为了防止失误操作  
    在一个进程中关闭 侦听套接字描述符 另一进程中关闭  
    客户端连接套接字描述符。注只有当所有套接字全都关闭时  
    当前连接才能关闭，fork调用的时候父进程与子进程有相同的  
    套接字，总共两套，两套都关闭掉才能关闭这个套接字  
    */    
    
    for(;;)    
    {    
        socklen_t addrlen = sizeof(client_addr);    
        //accept返回客户端套接字描述符    
        sc = accept(ss,(struct sockaddr *)&client_addr,&addrlen);  //注，此处为了获取返回值使用 指针做参数    
        if(sc < 0)  //出错    
        {    
            continue;   //结束此次循环    
        }
		else
		{
			LogDebug("server : connected from %s\n", inet_ntoa(client_addr.sin_addr));
			//创建一个子线程，用于与客户端通信
			xTaskCreate(client_task, "client_task", 256, &sc, 1, NULL);
		}
	}    
    close(ss);
}
 
/**  
  服务器对客户端连接处理过程；先读取从客户端发送来的数据，  
  然后将接收到的数据的字节的个数发送到客户端  
  */    
    
//通过套接字 s 与客户端进行通信    
void process_conn_server(int s)    
{    
    ssize_t size = 0;    
    char buffer[128];  //定义数据缓冲区    
    for(;;)    
    {    
        //等待读    
        for(size = 0;size == 0 ;size = read(s,buffer,sizeof(buffer)));    
        //输出从客户端接收到的数据    
        LogDebug("%s",buffer);    
    
        //结束处理    
        if(strcmp(buffer,"quit") == 0)    
        {    
            close(s);   //成功返回0，失败返回-1    
            return ;    
        }    
        sprintf(buffer,"%d bytes altogether\n",size);    
        write(s,buffer,strlen(buffer)+1);    
    }    
} 

void ICACHE_FLASH_ATTR
client_task(void * para)    
{
	process_conn_server(*(int *)para);
    vTaskDelete(NULL);
}

jsonrpc_server_t * self = NULL;
//soso
char Num2HexChar(uint8_t num)
{
	num&=0xFu;

	return (num>9u)?('a'+num-0x0a):('0'+num);
	//return (num>9u)?('7'+num):('0'+num);
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
//soso 
char * Array2HexString(const uint8_t array[], int arrayLen, char hex[], int maxLen)
{
	int i;

	if((array == NULL) || (hex == NULL))
		return NULL;
	if(arrayLen < 0)
		return NULL;
	if(maxLen < 2 * arrayLen + 1)
		return NULL;

	for(i = 0; i < arrayLen; i++)
	{
		hex[2*i] = Num2HexChar(array[i]>>4);
		hex[2*i+1] = Num2HexChar(array[i]);
	}
	hex[2*i]='\0';
	return hex;
}
//soso
uint8_t * HexString2Array(const char hex[], uint8_t array[], int * arrayLen)
{
	int i;

	if( (array != NULL) && (hex != NULL))
	{
		int maxLen = *arrayLen;
		int hexLen = strnlen(hex, 2*maxLen);

		if((hexLen > 0) && ((hexLen & 1u) == 0u))
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
void Array2InvertedOrder(uint8_t scr[],uint8_t des[],uint8_t length)
{
	uint8_t icnt = 0;
	for(icnt = 0;icnt < length;icnt++)
	{
		des[icnt] = scr[length-1-icnt];
	}
}

//#define USE_RPC_ZAP_PORT
#ifdef USE_RPC_ZAP_PORT
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
    LogDebug("%s \n", __FUNCTION__);

	LogDebug("netType = %s \n", netType);
	LogDebug("panId = %d \n", panId);
	LogDebug("radioTxPower = %d \n", radioTxPower);
	LogDebug("radioChannel = %d \n", radioChannel);
	return 0;
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
	LogDebug("%s \n", __FUNCTION__);
	return 0;
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
	LogDebug("%s \n", __FUNCTION__);
	return 0;
}

/******************************************************************************
 * FunctionName : zapRemoveDevice
 * Description  : 删除远端设备
 * Parameters   : 参数待定
 * Returns      : 1=success\0=fail
 * uint8_t zapRemoveDevice(char *ieee);
*******************************************************************************/
uint8_t zapRemoveDevice(uint8_t ieee[8])
{
	LogDebug("%s \n", __FUNCTION__);
	return 0;
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
	LogDebug("%s \n", __FUNCTION__);
	return 0;
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
	LogDebug("%s \n", __FUNCTION__);
	return 0;
}
#endif



#define USE_RPC_NOTIFY
#ifdef USE_RPC_NOTIFY
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
						uint8_t* message,
						uint8_t result)
{
	char ieeeStr[17];
	char * messageStr = (char *)malloc(messageLength*2+1);
	uint8_t ieeeInverted[8];
	
	LogDebug("%s \n", __FUNCTION__);
	LogDebug("%s result = %d\n",__FUNCTION__,result);
	
	if(self == NULL)
	{
		LogDebug("%s self == NULL\n", __FUNCTION__);
		return;
	}

	
	
	Array2InvertedOrder(ieee,ieeeInverted,8);	
	if(messageStr != NULL)
	{
	    Array2HexString(ieeeInverted, 8, ieeeStr, sizeof(ieeeStr));
	    Array2HexString(message, messageLength, messageStr, messageLength*2+1);

	    //****报文发送动作**//
	    jsonrpc_notify_client(self,
	            "zapSentNotify",
	            "ieee:s,profileId:i,clusterId:i,sourceEndpoint:i,destinationEndpoint:i,message:s,result:i", 
	            ieeeStr,
	            (double)profileId,
	            (double)clusterId,
	            (double)sourceEndpoint,
	            (double)destinationEndpoint,
	            messageStr,
	            (double)result);
		
		free(messageStr);
	}
	else
	{
		//LogError("%s \n", __FUNCTION__);
	}
	//**功能函数注册**//
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
	uint8_t ieeeInverted[8];
		
	LogDebug("%s \n", __FUNCTION__);

	if(self == NULL)
	{
		LogDebug("%s self == NULL\n", __FUNCTION__);
		return;
	}
	Array2InvertedOrder(ieee,ieeeInverted,8);

	if(messageStr != NULL)
	{
	    Array2HexString(ieeeInverted, 8, ieeeStr, sizeof(ieeeStr));
	    Array2HexString(message, messageLength, messageStr, messageLength*2+1);

	    //****报文发送动作**//
	    jsonrpc_notify_client(self,
	            "zapNewPacketNotify",
	            "ieee:s,profileId:i,clusterId:i,sourceEndpoint:i,destinationEndpoint:i,message:s", 
	            ieeeStr,
	            (double)profileId,
	            (double)clusterId,
	            (double)sourceEndpoint,
	            (double)destinationEndpoint,
	            messageStr);
		free(messageStr);

	}
	else
	{
		//LogError("%s \n", __FUNCTION__);
	}
	
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
											uint8_t ieee[8],
											const char *netType,
											uint8_t deviceType,
											int8_t rssi,
											uint8_t lqi,
											char *firmwareVer,
											char *productString)
{
	char ieeeStr[18];
	uint8_t ieeeInverted[8];
	LogDebug("%s \n", __FUNCTION__);

	if(self == NULL)
	{
		LogDebug("%s self == NULL\n", __FUNCTION__);
		return;
	}
	
	Array2InvertedOrder(ieee,ieeeInverted,8);
	Array2HexString(ieeeInverted, 8, ieeeStr, sizeof(ieeeStr));
	if(firmwareVer == NULL)
	{
		firmwareVer = "\0";
	}
	if(productString == NULL)
	{
		productString = "\0";
	}
	
	jsonrpc_notify_client(self,
						"zapNodeNetworkStatusNotify",
						"status:i,ieee:s,netType:s,deviceType:i,rssi:i,lqi:i,firmwareVer:s,productString:s",
						(double)status,
						ieeeStr,
						netType,
						(double)deviceType,
						(double)rssi,
						(double)lqi,
						firmwareVer,
						productString);	
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
	LogDebug("%s \n", __FUNCTION__);
	if(self == NULL)
	{
		LogDebug("%s self == NULL\n", __FUNCTION__);
		return;
	}
	jsonrpc_notify_client(self,
						"zapNetworkStatusNotify",
						"status:i",
						(double)status
						);
	gNetWorkStatus = status;
}
#endif

/*
struct jsonrpc_mstreamx
{
	char	*stream;
	size_t	alloc;
	size_t	length;
};
typedef struct jsonrpc_mstreamx	jsonrpc_mstream_tx;
*/
jsonrpc_error_t server_zapFormNetwork(int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{
	int8_t ret = 0;
	
	uint8_t netType[8];
	int netTypeLen = sizeof(netType);
	double panId;
	double radioTxPower;
	double radioChannel;

	LogDebug("%s \n", __FUNCTION__);

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
	LogDebug("server_zapFormNetwork pandid = %d,txpower = %d,channel = %d\n",(uint16_t)panId,(int8_t)radioTxPower,(uint8_t)radioChannel);
	
	ret = zapFormNetwork(netType,
						(uint16_t)panId,
						(int8_t)radioTxPower,
						(uint8_t)radioChannel
						);
	
	print_result(ctx, "%d", ret);
	
	return JSONRPC_ERROR_OK;
}


jsonrpc_error_t server_zapLeaveNetwork (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{
	int8_t ret = 0;
	
	uint8_t netType[8];
	int netTypeLen = sizeof(netType);

	LogDebug("%s \n", __FUNCTION__);
	
	if( (argc != 1) || (argv[0].json.type != JSONRPC_TYPE_STRING) )
	{
		return JSONRPC_ERROR_INVALID_PARAMS;
	}

	HexString2Array(argv[0].json.u.string, netType, &netTypeLen);
	if(netTypeLen != 8)
	{
		return JSONRPC_ERROR_INVALID_PARAMS;
	}
	ret = zapLeaveNetwork(netType);
	print_result(ctx, "%d", ret);
	return JSONRPC_ERROR_OK;
}

jsonrpc_error_t server_zapPermitJion (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{
	int8_t ret = 0;
	
	double seconds;

	LogDebug("%s \n", __FUNCTION__);
	
	if( (argc != 1) ||
	(argv[0].json.type != JSONRPC_TYPE_NUMBER) )
	{
		return JSONRPC_ERROR_INVALID_PARAMS;
	}
	seconds = argv[0].json.u.number;

	ret = zapPermitJion(
	            (uint16_t)seconds
	              );

	print_result(ctx, "%d", ret);
	return JSONRPC_ERROR_OK;
}

jsonrpc_error_t server_zapRemoveDevice (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{
	int8_t ret = 0;
	
	uint8_t ieee[8];
	uint8_t ieeeInverted[8];
	
	
	LogDebug("%s \n", __FUNCTION__);
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
	LogDebug("%s ieeeLen = %d\n",__FUNCTION__,ieeeLen);
	LogHexDump(LOG_INFO, "server_zapRemoveDevice ieee\n", ieee, 8);

	Array2InvertedOrder(ieee,ieeeInverted,8);
	ret = zapRemoveDevice(ieeeInverted);

	
	print_result(ctx, "%d", ret);
	return JSONRPC_ERROR_OK;
}


jsonrpc_error_t server_zapSetChannel (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{
	int8_t ret = 0;
	
	double channel;

	LogDebug("%s \n", __FUNCTION__);
	if( (argc != 1) ||
	(argv[0].json.type != JSONRPC_TYPE_NUMBER) )
	{
	return JSONRPC_ERROR_INVALID_PARAMS;
	}
	channel = argv[0].json.u.number;

	ret = zapSetChannel(
	            (uint16_t)channel
	              );

	print_result(ctx, "%d", ret);
	return JSONRPC_ERROR_OK;
}

jsonrpc_error_t server_zapSendUnicast (int argc, const jsonrpc_param_t *argv, void (* print_result)(void *ctx, const char *fmt, ...), void *ctx)
{
	uint8_t ret = 0;
	uint8_t ieeeInverted[8];
	uint8_t ieee[8];
	int ieeeLen = sizeof(ieee);
	double profileId;
	double clusterId;
	double sourceEndpoint;
	double destinationEndpoint;
	uint8_t *message = (uint8_t *)malloc(100);/*一条zigbee报文不超过100字节*/
	int messageLen = 100;
	jsonrpc_bool_t needConfirm;

	LogDebug("%s \n", __FUNCTION__);

	if(message == NULL)
	{
		return JSONRPC_ERROR_SERVER_INTERNAL;
    }
    
    LogDebug("argc=%d, argv=%d\n",argc,(int)argv);

	if( (argc != 7) || (argv == NULL) ||
			(argv[0].json.type != JSONRPC_TYPE_STRING) ||
			(argv[1].json.type != JSONRPC_TYPE_NUMBER) ||
			(argv[2].json.type != JSONRPC_TYPE_NUMBER) ||
			(argv[3].json.type != JSONRPC_TYPE_NUMBER) ||
			(argv[4].json.type != JSONRPC_TYPE_NUMBER) ||
			(argv[5].json.type != JSONRPC_TYPE_STRING) ||
			(argv[6].json.type != JSONRPC_TYPE_NUMBER) )
	{
		free(message);
		return JSONRPC_ERROR_INVALID_PARAMS;
	}

	HexString2Array(argv[0].json.u.string, ieee, &ieeeLen);
	if(ieeeLen != 8)
	{
		free(message);
		return JSONRPC_ERROR_INVALID_PARAMS;
	}

	profileId = argv[1].json.u.number;
	clusterId = argv[2].json.u.number;
	sourceEndpoint = argv[3].json.u.number;
	destinationEndpoint = argv[4].json.u.number;


	HexString2Array(argv[5].json.u.string, message, &messageLen);
	if((messageLen <= 0) || (messageLen > 100))
	{
		free(message);
		return JSONRPC_ERROR_INVALID_PARAMS;
	}

	needConfirm = argv[6].json.u.boolean;

	LogHexDump(LOG_INFO, "ieee", ieee, 8);
	Array2InvertedOrder(ieee,ieeeInverted,8);
	ret = zapSendUnicast(ieeeInverted,
						(uint16_t)profileId,
                  		(uint16_t)clusterId,
                  		(uint8_t)sourceEndpoint,
                  		(uint8_t)destinationEndpoint,
                  		(uint8_t)messageLen,
                  		message,
                  		(bool_t)needConfirm);

	
	print_result(ctx, "%d", ret);

	free(message);
	return JSONRPC_ERROR_OK;
}

void ICACHE_FLASH_ATTR
jsonrpc_task(void * para)
{
	uint32_t nowTime,lastTime,mtorrIntervalTime;
	jsonrpc_error_t error = JSONRPC_ERROR_OK;
	
	LogDebug("create birpc server used port %d\n", 8212);
	self = jsonrpc_server_open(0,8212);
	//LogDebug("create birpc server used port %d\n", 8219);
	//self = jsonrpc_server_open(0,8219);
	

	error  = jsonrpc_server_register_method(self, JSONRPC_TRUE, server_zapFormNetwork, "zapFormNetwork",  "netType:s,panId:i,radioTxPower:i,radioChannel:i");
	error  = jsonrpc_server_register_method(self, JSONRPC_TRUE,  server_zapLeaveNetwork, "zapLeaveNetwork", "netType:s");
	error  = jsonrpc_server_register_method(self, JSONRPC_TRUE,  server_zapPermitJion, "zapPermitJion", "seconds:i");
	error  = jsonrpc_server_register_method(self, JSONRPC_TRUE,  server_zapRemoveDevice, "zapRemoveDevice", "ieee:s");
	error  = jsonrpc_server_register_method(self, JSONRPC_TRUE,  server_zapSetChannel, "zapSetChannel", "channel:i");
	error  = jsonrpc_server_register_method(self, JSONRPC_TRUE, server_zapSendUnicast, "zapSendUnicast", 
										"ieee:s,profileId:i,clusterId:i,sourceEndpoint:i,destinationEndpoint:i,message:s,needConfirm:i");
	
	//resetZap();	
    zapInit();
	if(zapNetworkInit() == 1)
	{
		LogInfo("zapNetworkInit ok\n");
	}	
	
	lastTime = system_get_time();
	mtorrIntervalTime = 5*60*1000*1000;	

	while(1)
	{
		zapTick();
		if(self != NULL)
		{
			error = jsonrpc_server_run(self, 0);
		}
		else
		{
			LogError("%s %d err\n",__FUNCTION__,__LINE__);
		}

		nowTime = system_get_time();
		if(nowTime - lastTime > mtorrIntervalTime)
		{
			LogDebug("nowTime = %d, lastTime = %d\n",nowTime,lastTime);
			lastTime = nowTime;
			if(gNetWorkStatus == 1)
				zapSendMTORR();
		}		
	}
	LogError("%s return err\n",__FUNCTION__);
	jsonrpc_server_close(self);
	
	vTaskDelete(NULL);
}


void ICACHE_FLASH_ATTR
resetZap(void)
{
	int i;
    GPIO_OUTPUT_SET(GPIO_ID_PIN(4), 0);
    vTaskDelay(60);
    GPIO_OUTPUT_SET(GPIO_ID_PIN(4), 1);
    for(i=0;i<20;i++)
    {
       vTaskDelay(10);
		 //vTaskDelay(60);
    }
}
void ICACHE_FLASH_ATTR
zap_task(void * para)    
{
	resetZap();	
    zapInit();
	zapFormNetwork("soso",0xffff,0xff,0xff);
    zapPermitJion(60);
	
	LogDebug("%s resetZap zapInit zapFormNetwork zapPermitJion ok\n", __FUNCTION__);
    while(1)
    {
        zapTick();
        vTaskDelay(5);
    }
    vTaskDelete(NULL);
}
#if 1
u8 writeflag = 0;
u8 *wrietbuf = NULL;
u8 readflag = 0;
u8 *readbuf = NULL;

Ringbuf16_ST mysnd;
Ringbuf16_ST myrev;

static int myDoSelect( int server_socket, int *new_socket )
{
	/* *****select init******* // */
	struct sockaddr_in client_addr;

	int		sock_len	= sizeof(struct sockaddr);
	int		client_socket	= -1;
	int		result		= 0;
	int		nfds;
	struct fd_set	readfds;
	struct fd_set	writefds;
	struct fd_set	exceptfds;
	struct timeval	timeout = { 0, 0 };             /* select等待3秒，3秒轮询，要非阻塞就置0 */

	FD_ZERO( &readfds );                            /* ****用之前先将其清零**** // */
	FD_ZERO( &writefds );
	FD_ZERO( &exceptfds );                          /* ****用之前先将其清零**** // */

	if ( server_socket != -1 )                      /* ****socket是否成功产生*** // */
	{
		FD_SET( server_socket, &readfds );      /* 是，则添加描述符 */
		FD_SET( server_socket, &exceptfds );    /* 添加描述符 */
	}
	if ( *new_socket != -1 )                        /* ****看是否有客户端连接进来*** // */
	{
		FD_SET( *new_socket, &readfds );        /* 如果有，添加描述符 */
		if(Ringbuf16_Elements(&mysnd)) 
			FD_SET( *new_socket, &writefds );
		FD_SET( *new_socket, &exceptfds );      /* 添加描述符 */
	}

	nfds = (server_socket > *new_socket) ? server_socket + 1 : *new_socket + 1;

	if ( nfds > 0 )
	{
		/* ***循环检测描述符合集的变化****** // */
		result = select( nfds, &readfds, &writefds, &exceptfds, &timeout ); /* ****监听检查结果*** // */
		if ( result < 0 )
		{
			return(-1);
		}
		else if ( result == 0 )
		{
			return(0);
		}
		
		if ( FD_ISSET( server_socket, &readfds ) )                                                              /* ****服务器端已经成功打开socket，之后的连接与断开不再影响该值的变化*** // */
		{
			/* ****socket成功打开信息显示*** // */
			{
				client_socket = accept( server_socket, (struct sockaddr *) &client_addr, &sock_len );   /* ***进行接收*** // */
				if ( client_socket > 0 )
				{
					*new_socket = client_socket;
					LogInfo( "client %s connected socket = %d\n", inet_ntoa( client_addr.sin_addr ), client_socket );

					LogError( "server_socket =%d\n",server_socket);
					if ( 1 )
					{
						LogError( "ioctlsocket FIONBIO\n" );
						int	imode	= 1;
						int	retVal	= ioctlsocket( *new_socket, FIONBIO, &imode );
						if ( retVal == -1 )
						{
							LogError( "ioctlsocket failed!\n" );
							close( client_socket );
							/*
							 * WSACleanup();
							 * return -1;
							 */
						}
					}
				}else  {
					LogError( "accept error %d\n", 111 );
				}
			}
		}

		if ( FD_ISSET( *new_socket, &writefds ) )                                                                 /* ****判断是否可写**** // */
		{
			LogInfo( "client_socket writeable***********\n" );
			unsigned char	* ptr	= Ringbuf16_GetGetPtr(&mysnd);                                                             /* **得到读指针** // */
			int		remain	= Ringbuf16_GetGetLength(&mysnd,TRUE);  
			
			int send_data = send(*new_socket,ptr,remain,0);
			
			LogDebug( "send_data = %d\n", send_data );
			if ( send_data < 0 )
			{
				LogDebug("new_socket = %d\n",*new_socket);
				LogDebug("server_socket = %d\n",server_socket);
				LogDebug( "send failed %d %s\n", errno, strerror( errno ) );
				while(1);
			}
			else  
			{
				
				Ringbuf16_GetPtrInc(&mysnd, send_data);
			}
		}		
		if ( FD_ISSET( *new_socket, &readfds ) )                                                                                /* **连接字描述符处于可读状态，表示处于可以连接客户端的状态****** // */
		{
			int recv_data = 0;
			LogInfo( "client_socket readable**********\n" );

			unsigned char	* ptr	= Ringbuf16_GetPutPtr( &mysnd);                                                          /* ***允许写入的最大长度**** // */
			int		maxlen	= Ringbuf16_GetPutLength( &mysnd, TRUE );
			
			recv_data = recv( *new_socket, ptr, maxlen, 0 );
			LogInfo( "recv_data = %d\n",recv_data);
			if ( recv_data <= 0 )
			{
				LogInfo( "client_socket closed\n" );
			}
			else
			{
				Ringbuf16_PutPtrInc(&mysnd, recv_data);  
			}
		}

		if ( FD_ISSET( server_socket, &exceptfds ) )                                                                            /* ******服务器打开socket异常*** // */
		{
			LogInfo( "server_socket exception\n" );                                                                         /* ***** // */
		}

		if ( FD_ISSET( *new_socket, &exceptfds ) )                                                                              /* ******服务器打开socket异常*** // */
		{
			LogInfo( "client_socket exception\n" );                                                                         /* ***** // */
		}
		
		if ( FD_ISSET( *new_socket, &exceptfds ) )                                                                /* ****客户端连接异常*** // */
		{
			LogInfo( "client_socket exception\n" );
		}
		return(1);
	}
	else 
	{
		return(0);	
	}
}


void ICACHE_FLASH_ATTR
tcptest_task(void * para)    
{
	struct sockaddr_in servaddr;
	int server_socket = 0;
	int client_socket = 0;

	server_socket = socket(AF_INET, SOCK_STREAM, 0);

	wrietbuf = malloc(2048);
	readbuf = malloc(2048);
	
	Ringbuf16_Init(&mysnd, wrietbuf, 2048);
	Ringbuf16_Init(&myrev, readbuf, 2048);
	
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(12345);
	if (bind(server_socket, (struct sockaddr*) &servaddr, sizeof(servaddr))== -1)
	{
		LogDebug("bind error\n");
		close(server_socket);
	}

	if (listen(server_socket, 2) == -1) 
	{
		LogDebug("server_socket listen error\n");
		close(server_socket);
	}
	
	LogDebug("%s init ok\n", __FUNCTION__);

	resetZap();	
    zapInit();
	zapFormNetwork("soso",0xffff,0xff,0xff);
    zapPermitJion(60);
    while(1)
    {
    	zapTick();
    	myDoSelect(server_socket,&client_socket);
        //vTaskDelay(1);
    }
    vTaskDelete(NULL);
}
#endif


#define myfun(x)	LogDebug("the relat is"#x"%d\n",x+x)
void mytestsoso(void)
{
	int soso = 9;
	myfun(soso);
	myfun(3);
}


xTaskHandle jsonrpc_task_handle;

void ICACHE_FLASH_ATTR WifiTask(void * para)
{
	u32 jsonrpc_uxHighWaterMark = 0;
	
	//mytestsoso();

	
	jsonrpc_uxHighWaterMark =uxTaskGetStackHighWaterMark(jsonrpc_task_handle); 
	LogDebug("%s \n", __FUNCTION__);
	/* need to set opmode before you set config */
	WaitConnected();
    PrintInfo();	
	
	//xTaskCreate(tcptest_task, "tcptest_task", 512, NULL, 3, &jsonrpc_task_handle);
	
	xTaskCreate(jsonrpc_task, "jsonrpc_task", 512, NULL, 3, &jsonrpc_task_handle);
	//xTaskCreate(jsonrpc_task, "jsonrpc_task", 512, NULL, 4, NULL);
	//xTaskCreate(zap_task, "zap_task", 512, NULL, 4, NULL);

	while(1)
	{	
		LogDebug("%s jsonrpc_uxHighWaterMark = %d\n", __FUNCTION__,jsonrpc_uxHighWaterMark);
		vTaskDelay(500);
		jsonrpc_uxHighWaterMark =uxTaskGetStackHighWaterMark(jsonrpc_task_handle); 
		
	}
	vTaskDelete(NULL);
}

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{
	static const char ssid[] = "inSona";
	static const char password[]  = "Control4";
	struct station_config stationConf; 

	
	uart1_init();
    Logger_Init(&s_myLoggerConfig);
	Logger_SetGlobalLevel(LOG_DEBUG);
	LogInfo("***************** user_init %d %d*****************\n",os_random(),os_random());
    LogInfo("SDK version:%s\n", system_get_sdk_version());
    /* need to set opmode before you set config */
    wifi_set_opmode(STATION_MODE);

	stationConf.bssid_set = 0; 
	strcpy(stationConf.ssid, ssid); 
	strcpy(stationConf.password, password); 
	wifi_station_set_config(&stationConf); 

    xTaskCreate(WifiTask, "WifiTask", 384, NULL, 5, NULL);
}


