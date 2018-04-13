#include "insona_event.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
//#include <arpa/inet.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
#include "lwip/sockets.h"
#include <stdio.h>
#include "cJSON.h"
#define MAX_EVENT_BUFF_NUM 20
#define MAX_CONNECTED_NODE_NUM 10
void saveNodeIAddr(EmberNodeId newNodeId,EmberEUI64 newNodeEui64);
void rmNodeAddr(EmberEUI64 nodeEui64);


NodeAddr  nodeAddrTable[MAX_CONNECTED_NODE_NUM];
int8u p_save = 0;
int8u p_send = 0;
char event_buffer[20][200];

boolean cmpEUI64(EmberEUI64 n1,EmberEUI64 n2){
	int r = memcmp(n1,n2,EUI64_SIZE);
	if(r==0){
		return TRUE;
	}
	return FALSE;
}

void resetEUI64(EmberEUI64 nodeEui64){
	memset(nodeEui64,0,EUI64_SIZE);
}

void setEUI64(EmberEUI64 n1,EmberEUI64 n2){
	memcpy(n1,n2,EUI64_SIZE);
}

boolean isEUI64_EMPTY(EmberEUI64 nodeEui64){
	int  i = 0;
	for(i=0;i<EUI64_SIZE;i++){
		if(nodeEui64[i]!=0x00){
			return FALSE;
		}
	}
	return TRUE;
}


void addEvent(char* et){
	if(et==NULL){
		return;
	}
	if(NULL==strcpy(event_buffer[p_save],et)){
		return;
	}
	p_save>=(MAX_EVENT_BUFF_NUM-1)?(p_save=0):(p_save++);
}

void sendEventPro(int fd){
	while(p_save!=p_send){
		write(fd, &event_buffer[p_send][0], strlen(&event_buffer[p_send][0]));
		p_send>=(MAX_EVENT_BUFF_NUM-1)?(p_send=0):(p_send++);
	}
}


/*
typedef struct {
  int16u profileId;
  int16u clusterId;
  int8u sourceEndpoint;
  int8u destinationEndpoint;
  EmberApsOption options;
  int16u groupId;
  int8u sequence;
} EmberApsFrame;


typedef struct {
  EmberApsFrame            *apsFrame;
  EmberIncomingMessageType  type;
  EmberNodeId               source;
  int8u                    *buffer;
  int16u                    bufLen;
  boolean                   clusterSpecific;
  boolean                   mfgSpecific;
  int16u                    mfgCode;
  int8u                     seqNum;
  int8u                     commandId;
  int8u                     payloadStartIndex;
  int8u                     direction;
  EmberAfInterpanHeader    *interPanHeader;
  int8u                     networkIndex;
} EmberAfClusterCommand;
*/
boolean revCmdInfo(EmberAfClusterCommand* cmd)
{
	cJSON *root; 
	cJSON *array;
	int i=0;
	char* et_str = NULL; 
	char c_profileId[7];
	char c_clusterId[7];
	char c_shortid[7];
	char c_bufferlen[7];
	char c_sourceEndpoint[5];
	char c_destinationEndpoint[5];
	char c_commandId[5];
	char c_buffer[3];

	printf("revCmdInfo 0x%2x\n", cmd->source);
	snprintf(c_shortid,sizeof(c_shortid),"0x%02x",cmd->source);
	snprintf(c_profileId,sizeof(c_profileId),"0x%04x",cmd->apsFrame->profileId);
	snprintf(c_clusterId,sizeof(c_clusterId),"0x%04x",cmd->apsFrame->clusterId);
	snprintf(c_bufferlen,sizeof(c_bufferlen),"0x%02x",cmd->bufLen);
	snprintf(c_sourceEndpoint,sizeof(c_sourceEndpoint),"0x%02x",cmd->apsFrame->sourceEndpoint);
	snprintf(c_destinationEndpoint,sizeof(c_destinationEndpoint),"0x%02x",cmd->apsFrame->destinationEndpoint);
	snprintf(c_commandId,sizeof(c_commandId),"0x%02x",cmd->commandId);
	//snprintf(c_buffer,sizeof(c_buffer),"0x%2x",cmd->buffer);

	root=cJSON_CreateObject();
	cJSON_AddStringToObject(root, "name", "revNewCmd");
	cJSON_AddStringToObject(root, "profileId", c_profileId);
	cJSON_AddStringToObject(root, "clusterId", c_clusterId);
	cJSON_AddStringToObject(root, "sourceEndpoint", c_sourceEndpoint);
	cJSON_AddStringToObject(root, "destinationEndpoint", c_destinationEndpoint);
	cJSON_AddStringToObject(root, "shortid", c_shortid);
	cJSON_AddStringToObject(root, "commandId", c_commandId);
	cJSON_AddStringToObject(root, "bufLen", c_bufferlen);

	array = cJSON_CreateArray();
	for(i=0;i<cmd->bufLen;i++){
		snprintf(c_buffer,sizeof(c_buffer),"%02x",cmd->buffer[i]);
		cJSON_AddItemToArray(array, cJSON_CreateString(c_buffer));
	}
	cJSON_AddItemToObject(root, "buffer", array);
	et_str =  cJSON_Print(root);  
	addEvent(et_str);
  	return FALSE;
}


void newNodeNet(int16u shortid,EmberEUI64 ieee){
	cJSON *root; 
	char* et_str = NULL; 
	char c_shortid[7];
	char c_ieee[19];
	snprintf(c_shortid,sizeof(c_shortid),"0x%2x",shortid);
	snprintf(c_ieee,sizeof(c_ieee),"0x%2x",ieee);
	root=cJSON_CreateObject();    
	cJSON_AddStringToObject(root, "name", "newNodeNet");
	cJSON_AddStringToObject(root, "shotid", c_shortid);
	cJSON_AddStringToObject(root, "ieee", c_ieee);
	et_str =  cJSON_Print(root);  
	addEvent(et_str);
	saveNodeIAddr(shortid,ieee);
}

void nodeLeftNet(int16u shortid,EmberEUI64 ieee){
	cJSON *root; 
	char* et_str = NULL; 
	char c_shortid[7];
	char c_ieee[19];
	snprintf(c_shortid,sizeof(c_shortid),"0x%2x",shortid);
	snprintf(c_ieee,sizeof(c_ieee),"0x%2x",ieee);
	root=cJSON_CreateObject();    
	cJSON_AddStringToObject(root, "name", "nodeLeftNet");
	cJSON_AddStringToObject(root, "shotid", c_shortid);
	cJSON_AddStringToObject(root, "ieee", c_ieee);
	et_str =  cJSON_Print(root);  
	addEvent(et_str);
	rmNodeAddr(ieee);
}

void formResultInf(EmberStatus status){
	cJSON *root; 
	char ca[20];
	char* et_str = NULL; 
	snprintf(ca,sizeof(ca),"0x%x",status);
	root=cJSON_CreateObject(); 
	cJSON_AddStringToObject(root, "name", "form");   
	//if(status == EMBER_SUCCESS){
		cJSON_AddStringToObject(root,"result",ca);
	//}else{
	//	cJSON_AddStringToObject(root,"form","fail");
	//}
	et_str =  cJSON_Print(root);  
	addEvent(et_str);
}

void pjoinResultInf(EmberStatus status){
	cJSON *root; 
	//char ca[50];
	char* et_str = NULL; 
	//snprintf(ca,sizeof(ca),"pJoin for %d sec: 0x%x",duration,status);
	root=cJSON_CreateObject();   
	cJSON_AddStringToObject(root, "name", "pJoin");    
	if(status == EMBER_SUCCESS){
		cJSON_AddStringToObject(root,"pJoin","success");
	}else{
		cJSON_AddStringToObject(root,"pJoin","fail");
	}
	
	et_str =  cJSON_Print(root);  
	addEvent(et_str);
}


void sendResultInf(EmberStatus status){
	cJSON *root; 
	//char ca[50];
	char* et_str = NULL; 
	//snprintf(ca,sizeof(ca),"pJoin for %d sec: 0x%x",duration,status);
	root=cJSON_CreateObject(); 
	cJSON_AddStringToObject(root, "name", "send");  
	if(status == EMBER_SUCCESS){
		cJSON_AddStringToObject(root,"result","success");
	}else{
		cJSON_AddStringToObject(root,"result","fail");
	}
	
	et_str =  cJSON_Print(root);  
	addEvent(et_str);	
}


void saveNodeIAddr(EmberNodeId newNodeId,EmberEUI64 newNodeEui64){
	int8u p=0;
	for(;p<MAX_CONNECTED_NODE_NUM;p++){
		if(isEUI64_EMPTY(nodeAddrTable[p].nodeEui64)){
			NodeAddr addr;
			addr.nodeId = newNodeId;
			setEUI64(addr.nodeEui64,newNodeEui64);
			nodeAddrTable[p] = addr;
			printf("saveNodeIAddr:saveNode,newNodeEui64:%2x,newNodeId:%2x\n",newNodeEui64,newNodeId);
			return;
		}
	}
	printf("saveNodeIAddr:%s\n", "connected node full");
}

void rmNodeAddr(EmberEUI64 nodeEui64){
	int8u p=0;
	for(;p<MAX_CONNECTED_NODE_NUM;p++){
		if(cmpEUI64(nodeAddrTable[p].nodeEui64,nodeEui64)){
			resetEUI64(nodeAddrTable[p].nodeEui64);
			nodeAddrTable[p].nodeId = 0;
			printf("rmNodeAddr:saveNode,nodeEui64:%2x\n",nodeEui64);
		}
	}
}


EmberNodeId getShortId(EmberEUI64 nodeEui64){
	int8u p=0;
	for(;p<MAX_CONNECTED_NODE_NUM;p++){
		if(cmpEUI64(nodeAddrTable[p].nodeEui64,nodeEui64)){
		//if(nodeAddrTable[p].nodeEui64==nodeEui64){
			return nodeAddrTable[p].nodeId ;
		}
	}
	return 0;
}



