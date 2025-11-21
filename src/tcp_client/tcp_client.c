#ifndef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 200809L
#endif

#include "tcp_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "../include/weather-app-shared/core/utils/min.h"
#include "../include/weather-app-shared/core/utils/clock_monotonic.h"

/* Private functions */

static TCP_Client_Result tcp_client_read(TCP_Client *client);
static TCP_Client_Result tcp_client_send_queued(TCP_Client *client);

/*-------------------*/

TCP_Client_Result tcp_client_init(TCP_Client *client, TCP_Client_Callback_On_Received_Bytes_From_Server on_received){
    
    if(client == NULL){
        return TCP_Client_Result_Error;
    }

    memset(client, 0, sizeof(TCP_Client));
    client->connected = false;
    client->outgoing_buffer_bytes = 0;
    client->on_received_callback = on_received;
    client->socket.file_descriptor = -1;
    client->last_activity_timestamp = SystemMonotonicMS();

    return TCP_Client_Result_OK;
}

TCP_Client_Result tcp_client_connect(TCP_Client *client, const char *ip, int port){
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);

    if(inet_pton(AF_INET, ip, &addr.sin_addr) <= 0){
        return TCP_Client_Result_Connection_Failure;
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        return TCP_Client_Result_Connection_Failure;
    }

    int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0){
		return TCP_Client_Result_Connection_Failure;
    }
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    int res = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
    if(res < 0){ 
        if(errno != EINPROGRESS){
            close(fd);
            return TCP_Client_Result_Connection_Failure;
        }
    }

    client->socket.file_descriptor = (uint32_t)fd;
    client->connected = true;
    client->outgoing_buffer_bytes = client->outgoing_buffer_bytes;
    client->last_activity_timestamp = SystemMonotonicMS();
    
    return TCP_Client_Result_OK;
}

TCP_Client_Result tcp_client_read(TCP_Client *client){
    
    if(!client->connected){
        return TCP_Client_Result_Disconnected;
    }

    int totalBytesRead = 0;

    Socket_Result result = socket_read(&client->socket, client->incoming_buffer, TCP_CLIENT_RECEIVE_BUFFER_SIZE, &totalBytesRead);

    if(result == socket_result_connection_closed){
        return TCP_Client_Result_Disconnected;
    }

    if(result != Socket_Result_OK){
        return TCP_Client_Result_OK;
    }

    if(totalBytesRead > 0 && client->on_received_callback){
        client->last_activity_timestamp = SystemMonotonicMS();
        client->on_received_callback(client, client->incoming_buffer, (uint32_t)totalBytesRead);
    }

    return TCP_Client_Result_OK;
}

TCP_Client_Result tcp_client_send_queued(TCP_Client *client){

    if(client->outgoing_buffer_bytes == 0){
        return TCP_Client_Result_OK;
    }

    uint32_t sent = 0;

    Socket_Result write_result = socket_write(&client->socket, client->outgoing_buffer, client->outgoing_buffer_bytes, &sent);

    if(write_result == socket_result_connection_closed){
        return TCP_Client_Result_Disconnected;
    }

    if(sent > 0){
        client->last_activity_timestamp = SystemMonotonicMS();

        if(sent < client->outgoing_buffer_bytes){
            uint32_t remaining = client->outgoing_buffer_bytes - sent;
            memmove(client->outgoing_buffer, client->outgoing_buffer + sent, remaining);
            client->outgoing_buffer_bytes = remaining;
        } else {
            client->outgoing_buffer_bytes = 0;
        }
    }

    return TCP_Client_Result_OK;
}


TCP_Client_Result tcp_client_work(TCP_Client *client){

    if(!client->connected){
        return TCP_Client_Result_Disconnected;
    }

    TCP_Client_Result reading = tcp_client_read(client);
    if(reading != TCP_Client_Result_OK){
        return TCP_Client_Result_Error_Reading;
    }

    TCP_Client_Result send_queue = tcp_client_send_queued(client);
    if(send_queue != TCP_Client_Result_OK){
        return TCP_Client_Result_Error_Sending_Queued;
    }

    if(client->last_activity_timestamp + 15000 < SystemMonotonicMS()){
        return TCP_Client_Result_Disconnected;
    }

    return TCP_Client_Result_OK;
}

TCP_Client_Result tcp_client_send(TCP_Client *client, const uint8_t *buffer, uint32_t size){

    uint32_t space_left = TCP_CLIENT_OUTGOING_BUFFER_SIZE - client->outgoing_buffer_bytes;

    if(space_left < size){
        return TCP_Client_Result_Not_Enough_Space;
    }

    uint32_t copy = min_uint32(space_left, size);
    memcpy(&client->outgoing_buffer[client->outgoing_buffer_bytes], buffer, copy);
    client->outgoing_buffer_bytes += copy;

    return TCP_Client_Result_OK;
}

TCP_Client_Result tcp_client_disconnect(TCP_Client *client){

    if(client->socket.file_descriptor > 0){
        socket_close(&client->socket);
        client->socket.file_descriptor = 0;
    }

    client->connected = false;

    return TCP_Client_Result_OK;    
}

