#ifndef WIFI_H
#define WIFI_H

/***********************/
/*        Defines      */
/***********************/
typedef struct wifi_response_header {
    int error;
    int error_code;
    int length;
} wifi_response_header;

typedef struct wifi_connection {
    char channel_id[3];
} wifi_connection;

#define WIFI_HEADER_SIZE 9
#define WIFI_BUFFER_SIZE 500

/***********************/
/*       Variables     */
/***********************/
extern char response_code[WIFI_HEADER_SIZE];
extern char response_body[WIFI_BUFFER_SIZE];

/***********************/
/*       Functions     */
/***********************/
void get_channel_id(char* response_body, wifi_connection* conn);
wifi_response_header get_response(void);
void read(wifi_connection conn);

#endif
