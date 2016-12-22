#ifndef WEBSOCKET_H
#define WEBSOCKET_H

/***********************/
/*        Defines      */
/***********************/

#define WA_WEBSOCKET_SIZE 256
#define EMPTY '0'
#define DATA_AVAILABLE '1'
#define CONN_CLOSED '2'

/***********************/
/*       Variables     */
/***********************/

extern THD_WORKING_AREA(wa_websocket, WA_WEBSOCKET_SIZE);

/***********************/
/*       Functions     */
/***********************/

extern THD_FUNCTION(websocket, arg);

#endif
