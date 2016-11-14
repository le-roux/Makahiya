#ifndef WEB_H
#define WEB_H

#include "lwip/api.h"
#include "lwipthread.h"

#define WEB_THREAD_STACK_SIZE 2048

#define WEB_THREAD_PRIORITY (LOWPRIO + 2)
#define WEB_THREAD_PORT 80

/** @brief Master switch to enable DHCP in user code.
 *
 * - 0: DHCP is not used, addresses are hard-coded.
 * - 1: DHCP is used, adresses are automatically allocated by the network.
 */
#define USE_DHCP 1

extern THD_WORKING_AREA(wa_http_server, WEB_THREAD_STACK_SIZE);

/** @brief Control the execution of webpage requests.
 *
 * The internet thread waits on this binary semaphore until the user ask for a
 * page through the shell.
 * This semaphore is signaled in the __web__ function (shell_user.c).
 */
extern binary_semaphore_t web_bsem;

/** @brief Buffer to store the address of the requested page.
 *
 * This buffer is filled by __web__ (shell_user.c) before it signals web_bsem and
 * used by __http_function__ to create the HTTP request.
 */
extern char hostname[];

/** @brief The maximum size of the requested webpage address.
 *
 * This value defines the memory space allocated to the __hostname__ buffer.
 */
#define HOSTNAME_MAX_LENGTH 70

/** @brief Initialize all the stuff required to use the ethernet connction.
 *
 * Calling this function is __MANDATORY__ before any other call to an
 * ethernet-related function.
 */
void web_init(void);

/** @brief This function takes care of handling the webpages requests.
 *
 * It's the function executed by the thread dedicated to handle the web
 * connections.
 */
THD_FUNCTION(http_function, p);

#endif // WEB_H
