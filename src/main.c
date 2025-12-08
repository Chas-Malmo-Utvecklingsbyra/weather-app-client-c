#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "core/tcp/tcp_client/tcp_client.h"
#include "core/http/http.h"
#include "core/http/parser.h"

#define SERVER_ADDR "127.0.0.1"
#define TEST_PORT 8080

static void on_client_received(TCP_Client *client, const uint8_t *data, uint32_t size){
    
    (void)client;
    if(size > 0){
        printf("[CLIENT] Received: %u bytes\n", size);
        printf("%.*s\n", size, data);
    }
}

static void on_full_request(TCP_Client *client, Http_Request *request){
    
    (void)client;
    if(request->start_line.method == GET || request->start_line.method == POST || request->start_line.method == OPTIONS){
        printf("Received full HTTP request: %s %s\n", Http_Request_Get_Method_String(request), request->start_line.path);
    } else {
        printf("Received HTTP response, ignoring for now.\n");
    }
}

static void on_client_connect(TCP_Client *client){
    (void)client;

    printf("[CLIENT] I successfully connected!\n");
}

static void on_client_disconnect(TCP_Client *client){
    (void)client;

    printf("[CLIENT] Disconnected!\n");
}

static void on_client_error(TCP_Client *client, TCP_Client_Result error){
    (void)client;
    printf("[CLIENT] ERROR!\nI encountered the following error: %d\n", error);
}



int main() {
    /* printf("Hello, world! I am the (C) client.\n"); */

    
    TCP_Client client;
    if(tcp_client_init(&client, NULL, on_client_received, on_full_request, on_client_connect, on_client_disconnect, on_client_error) != TCP_Client_Result_OK){
        fprintf(stderr, "Failed to initialize Client\n");
        return 1;
    }

    printf("Connecting client to %s:%d\n", SERVER_ADDR, TEST_PORT);
    if(tcp_client_connect(&client, SERVER_ADDR, TEST_PORT) != TCP_Client_Result_OK){
        fprintf(stderr, "Client failed to connect to server.\n");
        return 1;
    }

    const char *msg =
        "GET /weather?latitude=59.85&longitude=17.6389 HTTP/1.1\r\n"
        "Host: 127.0.0.1\r\n"
        "User-Agent: curl/8.6.0\r\n"
        "Accept: */*\r\n"
        "Connection: close\r\n\r\n";

    bool request_sent = false;
    

    while(client.server.connection_state != TCP_Client_Connection_State_Disconnected){
        
        TCP_Client_Result client_result = tcp_client_work(&client);
        /* printf("[DEBUG] Connection state: %d | Incoming bytes: %u | Outgoing bytes: %u\n", client.server.connection_state, client.server.incoming_buffer_bytes, client.server.outgoing_buffer_bytes); */

        if(!request_sent && client.server.connection_state == TCP_Client_Connection_State_Connected && client.server.outgoing_buffer_bytes == 0){
            if(tcp_client_send(&client, (const uint8_t *)msg, (uint32_t)strlen(msg)) != TCP_Client_Result_OK){
                fprintf(stderr, "Client failed to queue initial message (buffer full?)\n");
                tcp_client_disconnect(&client);
                return 1;
            }
            printf("Sending message: \n%s\n", msg);
            request_sent = true;
        }

        if (client_result != TCP_Client_Result_OK && client_result != TCP_Client_Result_Nothing_Read_Yet && client_result != TCP_Client_Result_Nothing_Sent_Yet) {
            if(client_result == TCP_Client_Result_Disconnected){
                break;
            }
            fprintf(stderr, "Client error: %d\n", (int)client_result);
            printf("Shutting down!\n");
            break;
        }          
        
    }
    
    printf("Shutting down!\n");
    tcp_client_disconnect(&client);
    printf("Clean exit.\n");


    /*

    Create an input-buffer.

    Verify that we have an internet-connection and/or a network-card.

    Loop
        Wait for input from stdin.
        Fill buffer with data from stdin.

        Check whether or not the program should close by comparing the input-buffer against "q", "exit", etc.

        If we're here then the client wants to get weather data.
        Try connecting to the weather-app server:
            - Success:
                Send input-buffer to the server and wait for a Network Message.
                When the Network Message has come we need to check the type of the Network Message:
                    - Specify_Location
                    - Weather_Report
                        We got a reportPrint the response when we get it.
            - Fail:
                We probably don't have internet, or something is wrong with the server. Check the return-value(?).

    */



    /*

    Allting nedan kan faila och bör i de flesta fallen hanteras gracefully.
    Det finns åtminstonde ett ställe vi kan cachea och det är longitud och latitud för städer man sökt efter.

    1. Fyll input-buffern.
    2. Jämfört input-bufferten med quit-kommandon för att kunna ha ett sätt att avsluta programmet.
    3. Koppla upp dig till servern specifierad i args.
    4. Skicka input-bufferten, t.ex. "Malmö". (Request_Coordinates)
    5. Få respons från servern med alla locations för "Malmö". Varje location har sina koordinater. (Kanske vi kan ski)
    6. Skicka sedan koordinaterna du är intresserad av till servern, t.ex. location 2.
    7. Få en Weather_Report från servern.
    8. Cachea reporten. Nyckeln kan vara koordinaterna så om någon ber om de koordinaterna och de inte är "för gamla", använd den!

    */




    return 0;
}