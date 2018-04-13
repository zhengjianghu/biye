#ifndef __INSONA_UTIL__
#define __INSONA_UTIL__
#include "app/framework/include/af.h"
#include "app/framework/include/af-types.h"
extern int32s string2SignedInt(char* data);
extern int32u string2UnsignedInt(char* data);
extern void sendDataParse(int8u* dstp,char* srdp);
#endif