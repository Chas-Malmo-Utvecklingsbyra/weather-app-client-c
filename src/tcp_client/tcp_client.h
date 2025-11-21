#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <stdbool.h>
#include "../include/weather-app-shared/core/tcp/shared/tcp_shared.h"

#ifndef TCP_CLIENT_RECEIVE_BUFFER_SIZE
    #define TCP_CLIENT_RECEIVE_BUFFER_SIZE 1024
#endif

#ifndef TCP_CLIENT_OUTGOING_BUFFER_SIZE
    #define TCP_CLIENT_OUTGOING_BUFFER_SIZE 1024
#endif

typedef struct TCP_Client TCP_Client;

typedef enum {
    
    TCP_Client_Result_OK,
    TCP_Client_Result_Error,
    TCP_Client_Result_Error_Reading,
    TCP_Client_Result_Error_Sending_Queued,
    TCP_Client_Result_Connection_Failure,
    TCP_Client_Result_Not_Enough_Space,
    TCP_Client_Result_Disconnected

    /* ... */
} TCP_Client_Result;

typedef void(*TCP_Client_Callback_On_Received_Bytes_From_Server)(TCP_Client *client, const uint8_t *buffer, const uint32_t buffer_size);


 struct TCP_Client {
    Socket socket;
    bool connected;
    
    uint8_t incoming_buffer[TCP_CLIENT_RECEIVE_BUFFER_SIZE];
    uint8_t outgoing_buffer[TCP_CLIENT_OUTGOING_BUFFER_SIZE];
    uint32_t outgoing_buffer_bytes;
    uint64_t last_activity_timestamp;

    TCP_Client_Callback_On_Received_Bytes_From_Server on_received_callback;
};


TCP_Client_Result tcp_client_init(TCP_Client *client, TCP_Client_Callback_On_Received_Bytes_From_Server on_received);

TCP_Client_Result tcp_client_connect(TCP_Client *client, const char *ip, int port);

TCP_Client_Result tcp_client_work(TCP_Client *client);

TCP_Client_Result tcp_client_send(TCP_Client *client, const uint8_t *buffer, uint32_t size);

TCP_Client_Result tcp_client_disconnect(TCP_Client *client);


#endif