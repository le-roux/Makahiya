#include "ch.h"
#include "utils.h"
#include "chprintf.h"
#include "string.h"

#include "web.h"
#include "usbcfg.h"

THD_WORKING_AREA(wa_http_server, WEB_THREAD_STACK_SIZE);

#if !USE_DHCP
static ip_addr_t address, gateway, netmask;
#endif // !USE_DHCP

static uint8_t macaddress[6] = {0xC2, 0xAF, 0x51, 0x13, 0xCF, 0x46};
static lwipthread_opts_t internet_config = {macaddress, 0, 0, 0};
static ip_addr_t target_ip;

binary_semaphore_t web_bsem;
char hostname[HOSTNAME_MAX_LENGTH];

// Error return code
static err_t ret;
static msg_t msg;

static const char request_prolog[] = "GET / HTTP/1.0\r\nHost: ";
static const char request_epilog[] = "\r\n\r\n";
static char request[100];

/**
 * Set all the required parameters and start the web module.
 */
void web_init(void) {
    chBSemObjectInit(&web_bsem, true);
    #if !USE_DHCP
    IP4_ADDR(&address, 137, 194, 66, 51);
    IP4_ADDR(&gateway, 137, 194, 64, 254);
    IP4_ADDR(&netmask, 255, 255, 252, 0);

    internet_config.address = address.addr;
    internet_config.gateway = gateway.addr;
    internet_config.netmask = netmask.addr;
    #endif //!USE_DHCP

    lwipInit(&internet_config);
}

THD_FUNCTION(http_function, p) {
    UNUSED(p);
    struct netconn* conn;
    struct netbuf* response;
    char* data;
    u16_t data_length;
    while (TRUE) {
        msg = chBSemWait(&web_bsem);
        if (msg != MSG_OK)
            continue;

        strcpy(request, request_prolog);
        strcat(request, hostname);
        strcat(request, request_epilog);
        chprintf((BaseSequentialStream*)&SDU1, "Request: %s\r\n", request);
        ret = netconn_gethostbyname(hostname, &target_ip);
        if (ERR_OK == ret)
            chprintf((BaseSequentialStream*)&SDU1, "Host found %d\n\r", target_ip.addr);
        else {
            chprintf((BaseSequentialStream*)&SDU1, "Error finding host %i\n\r", ret);
            continue;
        }

        while(ret != -12) {
            chprintf((BaseSequentialStream*)&SDU1, "New attempt\r\n");
            conn = netconn_new(NETCONN_TCP);
            ret = netconn_connect(conn, &target_ip, WEB_THREAD_PORT);
            if (ret != ERR_OK) {
                chprintf((BaseSequentialStream*)&SDU1, "Error connecting %i\n\r", ret);
                netconn_delete(conn);
                continue;
            }
            else
                chprintf((BaseSequentialStream*)&SDU1, "Success connecting\n\r");

            ret = netconn_write(conn, request, strlen(request), NETCONN_NOCOPY);
            if (ret != ERR_OK) {
                chprintf((BaseSequentialStream*)&SDU1, "Error writing %i\n\r", ret);
                netconn_delete(conn);
                continue;
            } else
                chprintf((BaseSequentialStream*)&SDU1, "Request sent\n\r");

            for (u16_t i = 0; i < sizeof(request); i++)
                chSequentialStreamPut((BaseSequentialStream*)&SDU1, request[i]);

            ret = netconn_recv(conn, &response);
            while (ret == ERR_OK) {
                netbuf_data(response, (void**)&data, &data_length);
                for (u16_t i = 0; i < data_length; i++) {
                    chSequentialStreamPut((BaseSequentialStream*)&SDU1, data[i]);
                    if (data[i] == '\n')
                        chSequentialStreamPut((BaseSequentialStream*)&SDU1, '\r');
                }
                netbuf_delete(response);
                ret = netconn_recv(conn, &response);
            }

            if (ret != -12)
                chprintf((BaseSequentialStream*)&SDU1, "\n\rEnd with error %i\n\r\n\r\n\r\n\r", ret);
            else
                chprintf((BaseSequentialStream*)&SDU1, "\r\nPage sucessfully loaded\n\r\n\r\n\r\n\r");

            netconn_delete(conn);
        }
    }
}
