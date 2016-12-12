#ifndef WIFI_H
#define WIFI_H

/***********************/
/*        Defines      */
/***********************/
typedef struct wifi_response {
    int error;
    int error_code;
    int length;
    char channel_id[3];
} wifi_response;

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
wifi_response parse_response_code(void);
void get_channel_id(char* response_body, char* out);

#endif
