#ifndef __INSONA_SOCKET_CMD__
#define __INSONA_SOCKET_CMD__
#include "app/framework/include/af.h"
#include "app/framework/include/af-types.h"
#include "app/framework/util/af-main.h"
#include "app/framework/cli/zcl-cli.h"
#define SOCKET_CMDNAME_ERR  1
#define SOCKET_CMDPAR_ERR	2
void parParse(char *cmd);
int cmdParse();
#endif