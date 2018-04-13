#include "insona_socket.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
//#include <arpa/inet.h>
//#include <sys/socket.h>
#include "lwip/sockets.h"
//#include <netinet/in.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#include "espressif/c_types.h"
#include "lwip/sockets.h"
//#include <pthread.h>
#include "insona_socket_cmd.h"
#define MAXLINE 2000 
static int SERVER_PORT = 1234;
boolean socket_start_flag = FALSE;
int serv_sock = -1;
int clnt_sock = -1;
//pthread_t accept_thread_id;
//pthread_t trans_thread_id;
//pthread_t send_thread_id;
char buff[200];  
int  len_buff;  
int rp=0;

void closerLink();
void closeServer();

char send_buff[10][200];
boolean send_flag = FALSE;
int p_send1 = 0;
int p_send2 = 0;
boolean sync_flag =FALSE;

boolean rev_cmd_flag = FALSE;

void insonaAfPrintln(char * formatString) 
{
  //va_list p;  
  //int arg;  
  //va_start(p, formatString);  
 // while ((arg = va_arg(p,int)) != 0) {  
   //     total += arg;  
  //}  
  //printf(msg, total); 
  //sprintf(formatString,va_arg); 
  //va_end(p);  
 
  addSendBuffer(formatString);
}
void addSendBuffer(char* data){
  printf("addSendBuffer %s\n",data);
	memcpy(&send_buff[p_send2][0],data,strlen(data));
	sync_flag = FALSE;
	p_send2++;
	if(p_send2>=10){
		p_send2=0;
	}
	sync_flag = TRUE;
  	printf("p_send1:%d,p_send2:%d\n",p_send1,p_send2 );
}


/*
int getchar_socket(){
  int r  =-1;
  if(rp<n){
     r = buff[rp];
     rp++;
  }
  return r;
}*/
EmberStatus readByteFromSocket(int8u *dataByte){
  int ch=-1;
  
  if(rp<len_buff){
	printf("rp:%d,len_buff:%d\n",rp,len_buff );
	ch= buff[rp];
	rp++;
	*dataByte = (int8u)ch;
    return EMBER_SUCCESS;
  }
  //if(ch<0) {
    return EMBER_SERIAL_RX_EMPTY;
  //}
  
}
void sendThread(){
	printf("Start send pthread \n");
	while(1){
    vTaskDelay(1);
		//sleep(1);
		if(clnt_sock==-1){
      printf(" send pthread  exit\n");
			break;
		}
    sendEventPro(clnt_sock);
    //printf(" send pthread  alive :%d,%d,%d\n",sync_flag,p_send1,p_send2);
    #if 0
		if( (sync_flag==TRUE) && (p_send1!=p_send2) ){
       		//write(clnt_sock, "yyyy", strlen("yyyy"));
      		printf(" send data  %d,%d\n",p_send1,p_send2);
			    write(clnt_sock, &send_buff[p_send1][0], strlen(&send_buff[p_send1][0]));
			p_send1++;
			if(p_send1>=10){
				p_send1=0;
			}
		}
    #endif
	}
}

void socketCmdPro(){
  if(rev_cmd_flag==TRUE){
    parParse(buff);
    cmdParse();
    rev_cmd_flag=FALSE;
  }
}

void transThread(){
    printf("Start trans pthread \n");
	while(1){
      vTaskDelay(1);
     // sleep(1);
      int n = recv(clnt_sock, buff, MAXLINE, 0);
      if(n<=0){
        printf("closerLink:%d\n",n);  
        closerLink();
        break;
      }else{
        /*
		    len_buff = n;
		    buff[len_buff]='\r';
		    buff[len_buff+1]='\n';
        buff[len_buff+2]='\0';
        len_buff+=2;
        rp=0;
        */
        
        len_buff = n;
        buff[len_buff]='\0';
        rp=0;
        printf("recv msg from client:%s,len_buff:%d\n",&buff[0],len_buff);  
       rev_cmd_flag = TRUE;
       // write(clnt_sock, "xxxx", strlen("xxxx"));
      }
      //n=n1;
      //buff[n] = '\0';  
      //rp=0;  
      //printf("recv msg from client:%d\n",n);  
    }
}
void acceptThread(){
	struct sockaddr_in clnt_addr;
    //接收客户端请求
    while(1){
   		if(serv_sock==-1){
    		break;
    	} 
      if(clnt_sock!=-1){
        //sleep(1);
        vTaskDelay(1);
        continue;
      }
    	socklen_t clnt_addr_size = sizeof(clnt_addr);
      printf("start accept\n");
    	clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
    	printf("server: got connection from %s\n", inet_ntoa(clnt_addr.sin_addr)); 
      int ret = xTaskCreate(transThread, "acceptThread", 100, NULL, 2, NULL);
    	//int ret=pthread_create(&trans_thread_id,NULL,(void*)transThread,NULL); // 成功返回0，错误返回错误编号
    	if (ret != pdPASS)  {
      		printf("Create trans pthread error\n");
    	}else{
      		printf("Create trans pthread success\n");
    	}
      ret = xTaskCreate(sendThread, "acceptThread", 100, NULL, 3, NULL);
    	//ret=pthread_create(&send_thread_id,NULL,(void*)sendThread,NULL); // 成功返回0，错误返回错误编号
    	if (ret != pdPASS)  {
      		printf("Create send pthread error\n");
    	}else{
      		printf("Create send pthread success\n");
    	}
    	
    }
}
void emberAfMain_8266_local(){
  emberAfMain_8266_socket();
}
void  startTask_emberAfMain_8266(){
  // int ret = xTaskCreate(acceptThread, "af_main_task", 2000, NULL, 4, NULL);
   // if (ret != pdPASS)  {
    int ret = xTaskCreate(emberAfMain_8266_local, "af_main_task", 2000, NULL, 4, NULL);
   if (ret != pdPASS)  {
      printf("Create Af Main pthread error\n");
    }else{
      printf("Create Af Main pthread success\n");
    }
}

void startSocketServer(){
  struct sockaddr_in serv_addr;
  if(socket_start_flag==FALSE){
    serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //将套接字和IP、端口绑定
    memset(&serv_addr, 0, sizeof(serv_addr));  //每个字节都用0填充
    serv_addr.sin_family = AF_INET;  //使用IPv4地址
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);///inet_addr("127.0.0.1");  //具体的IP地址
    serv_addr.sin_port = htons(SERVER_PORT);  //端口
    bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    //进入监听状态，等待用户发起请求
    listen(serv_sock, 20);
    int ret = xTaskCreate(acceptThread, "acceptThread", 200, NULL, 5, NULL);
    //int ret=pthread_create(&accept_thread_id,NULL,(void*)acceptThread,NULL); // 成功返回0，错误返回错误编号
    if (ret != pdPASS)  {
  //  if(ret!=0){
      printf("Create accept pthread error\n");
    }else{
      printf("Create accept pthread success\n");
    }
    socket_start_flag = TRUE;
    //向客户端发送数据
    //char str[] = "Hello World!";
    //write(clnt_sock, str, sizeof(str));
   
    //关闭套接字
    //close(clnt_sock);
    //close(serv_sock);
    return;
  }
}

void closerLink(){
	close(clnt_sock);
	clnt_sock = -1;
}
void closeServer(){
	close(serv_sock);
	serv_sock=-1;
}

