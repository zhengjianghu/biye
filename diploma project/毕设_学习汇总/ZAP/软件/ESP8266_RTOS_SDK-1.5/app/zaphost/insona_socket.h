#ifndef __INSONA_SOCKET_H__
#define __INSONA_SOCKET_H__
#include "app/framework/include/af.h"
#include "app/framework/include/af-types.h"

 EmberStatus readByteFromSocket(int8u *dataByte);
 void startSocketServer();
 void addSendBuffer(char* data);
void insonaAfPrintln(char * formatString) ;
void socketCmdPro();
void startTask_emberAfMain_8266();
#endif