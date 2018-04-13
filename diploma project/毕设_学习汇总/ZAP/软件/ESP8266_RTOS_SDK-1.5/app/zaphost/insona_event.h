#ifndef __INSONA_EVENT__
#define __INSONA_EVENT__
#include "app/framework/include/af.h"
#include "app/framework/include/af-types.h"
void sendEventPro(int fd);
void newNodeNet(int16u shortid,EmberEUI64 ieee);
void nodeLeftNet(int16u shortid,EmberEUI64 ieee);
void formResultInf(EmberStatus status);
void pjoinResultInf(EmberStatus status);
void sendResultInf(EmberStatus status);
boolean revCmdInfo(EmberAfClusterCommand* cmd);
EmberNodeId getShortId(EmberEUI64 nodeEui64);

typedef struct
{
	EmberNodeId nodeId;
    EmberEUI64 nodeEui64;
}NodeAddr;



#endif