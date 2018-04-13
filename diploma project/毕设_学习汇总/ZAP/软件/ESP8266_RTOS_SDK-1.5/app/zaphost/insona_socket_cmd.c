#include "insona_socket_cmd.h"
#include "insona_util.h"
#include "stdio.h"
#include "string.h"
char* cmd_par[] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL};
int cmd_par_num = 0;
typedef struct {
  char* name;
  CommandAction action;
  int par_num;
  char* des;
} SocketCommandEntry;

 #define socketCommandEntryActionWithDetails(name,action,par_num,des) \
    { (name), (action), (par_num), (des) }
void socketForm(){
	EmberStatus status;
	EmberNetworkParameters networkParams;
	EmberNetworkParameters* np = &networkParams;
	printf("insona_socket_cmd:%s\n", "invoke socketForm");
	memset(np, 0, sizeof(EmberNetworkParameters));
  	//emberAfGetFormAndJoinExtendedPanIdCallback(np->extendedPanId);
  	np->radioChannel = 15;
  	np->radioTxPower = -3;
 	np->panId = (int16u)0x1234;
 	printf("insona_socket_cmd:%s\n", "Tag5");
	status = emberAfFormNetwork(&networkParams);
	printf("insona_socket_cmd:%s\n", "Tag6");
	emberAfAppPrintln("%p 0x%x", "form", status);
	emberAfAppFlush();
}

void socketFormWithChn(){
	printf("insona_socket_cmd:%s\n", "invoke socketFormWithChn");
}

void socketFormWithChnAndPanId(){
	EmberStatus status;
	EmberNetworkParameters networkParams;
	EmberNetworkParameters* np = &networkParams;
	printf("insona_socket_cmd:%s\n", "invoke socketFormWithChnAndPanId");
	memset(&np, 0, sizeof(EmberNetworkParameters));
  	//emberAfGetFormAndJoinExtendedPanIdCallback(np->extendedPanId);
  	np->radioChannel = (int8u)string2UnsignedInt(cmd_par[1]);
  	printf("insona_socket_cmd:%s\n", "Tag3");
  	np->radioTxPower = (int8s)string2SignedInt(cmd_par[2]);
  	printf("insona_socket_cmd:%s\n", "Tag4");
 	np->panId = (int16u)string2UnsignedInt(cmd_par[3]);
 	printf("insona_socket_cmd:%s\n", "Tag5");
	status = emberAfFormNetwork(&networkParams);
	printf("insona_socket_cmd:%s\n", "Tag6");
	emberAfAppPrintln("%p 0x%x", "form", status);
	emberAfAppFlush();
}
void socketNetworkPjoin(){
  emAfPermitJoin(60,FALSE_C);
}
void socketNetworkPjoinWithSeconds(){
  int8u duration = (int8u)string2UnsignedInt(cmd_par[1]);
  printf("socketNetworkPjoinWithSeconds:duration %d\n",duration );
  emAfPermitJoin(duration,FALSE_C);
}
/*
void relayControl(int16u indexOrDestination,boolean onoff){//0x26b9
  EmberStatus status;

  if(onoff){
     emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND \
                             | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER), \
                            ZCL_ON_OFF_CLUSTER_ID, \
                            ZCL_ON_COMMAND_ID, \
                            "");
     printf("ZCL_ON_COMMAND_ID:%x\r\n",ZCL_ON_COMMAND_ID);
   }else{
    emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND \
                             | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER), \
                            ZCL_ON_OFF_CLUSTER_ID, \
                            ZCL_OFF_COMMAND_ID, \
                            "");
    printf("ZCL_OFF_COMMAND_ID:%x\r\n",ZCL_OFF_COMMAND_ID);
   }
  emberAfSetCommandEndpoints(11,11);
  emberAfGetCommandApsFrame()->options |= EMBER_APS_OPTION_SOURCE_EUI64;
  status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, indexOrDestination);
  if(status != EMBER_SUCCESS) {
    printf("Cannot send status:%x\r\n",status);
  }else{
    printf("send command success\r\n");
  }
}
*/
void socketSendDataWithPar(int16u destination,
							int16u cluster_id ,
							int8u srcEndpoint,
							int8u dstEndpoint,
							int8u bufferlen,
							int8u* buffer
							){
	EmberStatus status;
	int8u label;
	printf("destination:0x%2x,srcEndpoint:%d,dstEndpoint:%d\n",destination,srcEndpoint,dstEndpoint);
	emAfApsFrameClusterIdSetup(cluster_id);
	emAfApsFrameEndpointSetup(srcEndpoint, dstEndpoint);
	if (emberAfPreCliSendCallback(&globalApsFrame,
	                            emberAfGetNodeId(),
	                            destination,
	                            buffer,
	                            bufferlen)) {
    	return;
  	}
   	label = 'U';
    status = emberAfSendUnicast(EMBER_OUTGOING_DIRECT,
                                destination,
                                &globalApsFrame,
                                bufferlen,
                                buffer);

	printf("label:%c,\n",label);
	if (status != EMBER_SUCCESS) {
		emberAfCorePrintln("Error: CLI Send failed, status: 0x%X", status);
	}
	sendResultInf(status);
	UNUSED_VAR(label);
	emberAfDebugPrintln("T%4x:TX (%p) %ccast 0x%x%p",
	              emberAfGetCurrentTime(),
	              "CLI",
	              label,
	              status,
	              ((globalApsFrame.options & EMBER_APS_OPTION_ENCRYPTION)
	               ? " w/ link key" : ""));
	emberAfDebugPrint("TX buffer: [");
	emberAfDebugFlush();
	emberAfDebugPrintBuffer(buffer, bufferlen, TRUE_C);
	emberAfDebugPrintln("]");
	emberAfDebugFlush();

	mfgSpecificId = EMBER_AF_NULL_MANUFACTURER_CODE;
	disableDefaultResponse = 0;
}

void socketSendData(){// send shortid clusterid srcEndpoint dstEndpoint bufferlen buffer[]

	int16u destination =string2UnsignedInt(cmd_par[1]);
	int16u cluster_id = string2UnsignedInt(cmd_par[2]);
	int8u srcEndpoint =(int8u)string2UnsignedInt(cmd_par[3]);
	int8u dstEndpoint =(int8u)string2UnsignedInt(cmd_par[4]);
	int8u bufferlen=(int8u)string2UnsignedInt(cmd_par[5]);
	int8u buffer[50];
	sendDataParse(buffer,cmd_par[6]);
	if(buffer==NULL){
		printf("socketSendData:%s\n","data format err");
	}
	socketSendDataWithPar(destination,
							 cluster_id ,
							 srcEndpoint,
							 dstEndpoint,
							 bufferlen,
							 buffer
							);
	/*EmberStatus status;
	int8u label;
	int16u socketSendDataBuffLen =3;
	int8u socketSendDataBuff[] ={0x01,0x00,0x01};
	int8u on_off = (int8u)string2UnsignedInt(cmd_par[1]);
	if(on_off==0){
		socketSendDataBuff[2]=0;
	}
	socketSendDataBuff[1]=emberAfNextSequence();*/

  
}

void ONOFF(){//switch destination dev on-off
	int16u destination =string2UnsignedInt(cmd_par[1]);
	int16u cluster_id = ZCL_ON_OFF_CLUSTER_ID;
	int8u srcEndpoint =1;
	int8u dstEndpoint =(int8u)string2UnsignedInt(cmd_par[2])+10;
	int8u bufferlen=3;
	int8u buffer[]={0x01,0x00,0x00};
	buffer[1]=emberAfNextSequence();
	if(strcmp(cmd_par[3],"on")==0){
		buffer[2]=ZCL_ON_COMMAND_ID;
	}else if(strcmp(cmd_par[3],"off")==0){
		buffer[2]=ZCL_OFF_COMMAND_ID;
	}else{
		printf("socketSendData:%s\n","command err");
		return;
	}
	if(buffer==NULL){
		printf("socketSendData:%s\n","data format err");
		return;
	}
	socketSendDataWithPar(destination,
							 cluster_id ,
							 srcEndpoint,
							 dstEndpoint,
							 bufferlen,
							 buffer
							);
}

SocketCommandEntry socketCommandTable[]={
	socketCommandEntryActionWithDetails("form", socketForm,0, ""),
	socketCommandEntryActionWithDetails("form", socketFormWithChn,1, ""),
	socketCommandEntryActionWithDetails("form", socketFormWithChnAndPanId,3, ""),
	socketCommandEntryActionWithDetails("pjoin",socketNetworkPjoin,0, ""),
	socketCommandEntryActionWithDetails("pjoin",socketNetworkPjoinWithSeconds,1, ""),
	socketCommandEntryActionWithDetails("send", socketSendData,6, ""),
	socketCommandEntryActionWithDetails("switch",ONOFF,3, ""),
	socketCommandEntryActionWithDetails(NULL,NULL,0,NULL)
};

void clrCmdPar(){
	memset(cmd_par,0,sizeof(cmd_par));
	cmd_par_num = 0;
}
void parParse(char *cmd){
	printf("parParse:%s\n", cmd);
	int p = 0;
	char delim[] = " ";
	char *result = NULL;
	clrCmdPar();
	result = strtok(cmd,delim);
	while(result !=NULL){
		cmd_par[p]=result;
		p++;
		result = strtok(NULL,delim);
	}
	cmd_par_num = p-1;
	printf("parParse result:%s,%s,%s,%s,%s,%s,%s\n", cmd_par[0],cmd_par[1],cmd_par[2],cmd_par[3]
												,cmd_par[4],cmd_par[5],cmd_par[6]);
}

int cmdParse(){
	if(cmd_par[0]==NULL){
		return SOCKET_CMDNAME_ERR;
		printf("cmdParse:%s\n", "cmdname err");
	}
	SocketCommandEntry *commandFinger = &socketCommandTable[0];
  	for (; commandFinger->name != NULL; commandFinger++) {
    	if(memcmp(commandFinger->name,cmd_par[0],strlen(cmd_par[0])) == 0
    		&& commandFinger->par_num ==cmd_par_num
    		){
    		(*commandFinger->action)();
    		return 0;
    	}
  	}
  	printf("cmdParse:%s\n", "no cmd match");
  	return -1;
}