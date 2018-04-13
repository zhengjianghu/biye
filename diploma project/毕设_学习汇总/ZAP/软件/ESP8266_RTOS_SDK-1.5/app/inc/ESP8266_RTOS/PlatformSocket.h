/**    
 *

 */

#ifndef PLATFORM_SOCKET_H
#define PLATFORM_SOCKET_H

/**   
 * PlatformSocket.h function
 * 1. socket : socket bind accept connect listen send sendto recv recvfrom close
 * 2. sleep : sleep
 * 3. time : time
 * 4. errno : errno strerr
 */

#include <time.h>           /* time */
#include "lwip/inet.h"      /* in_addr ... */
#include "lwip/sockets.h"
#include "esp_common.h"     /* size_t */
#include "ssl_compat-1.0.h"
#include "c_types.h"        /* c_type */
#include "errno.h"          /* errno */

#ifdef __cplusplus
extern "C" {
#endif

int32_t sleep(uint32);

#ifdef __cplusplus
}
#endif

#endif