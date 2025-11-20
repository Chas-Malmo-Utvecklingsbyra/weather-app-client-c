#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "core/tcp/tcp_client/tcp_client.h"
#include "core/tcp/server/tcp_server.h"

/* Just a temporary test to see if the tcp_client will work (PL)*/
#define TEST_PORT 9000
#define SERVER_ADDR "127.0.0.1"

static void on_server_received(TCP_Server *server, TCP_Server_Client *client, const uint8_t *buffer, const uint32_t size){
    
    if(!server || !client || !buffer || size == 0){
        return;
    }

    printf("[SERVER] Received from client FD %d: %.*s\n", client->socket.file_descriptor, (int)size, buffer);

    (void)tcp_server_send_to_client(server, client, buffer, size);
}

static void on_client_received(TCP_Client *client, const uint8_t *data, uint32_t size){
    
    (void)client;
    if(!data || size == 0){
        return;
    }
    printf("[CLIENT] Received: %.*s\n", (int)size, data);

}



int main() {
    /* printf("Hello, world! I am the (C) client.\n"); */

    TCP_Server server;
    if(tcp_server_init(&server, TEST_PORT, on_server_received) != TCP_Server_Result_OK){
        printf("Failed to initialize server.\n");
        return 1;
    }

    if(tcp_server_start(&server) != TCP_Server_Result_OK){
        fprintf(stderr, "Failed to start server.\n");
        tcp_server_dispose(&server);
        return 1;
    }

    printf("Server running on port %d\n", TEST_PORT);


    TCP_Client client;
    if(tcp_client_init(&client, on_client_received) != TCP_Client_Result_OK){
        fprintf(stderr, "Failed to initialize Client\n");
        tcp_server_dispose(&server);
        return 1;
    }

    printf("Connecting client to %s:%d\n", SERVER_ADDR, TEST_PORT);
    if(tcp_client_connect(&client, SERVER_ADDR, TEST_PORT) != TCP_Client_Result_OK){
        fprintf(stderr, "Client failed to connect to server.\n");
        tcp_client_disconnect(&client);
        tcp_server_dispose(&server);
        return 1;
    }
    printf("Client connected.\n");

    const char *msg = "Hello from the Client!";
    if(tcp_client_send(&client, (const uint8_t *)msg, (uint32_t)strlen(msg)) != TCP_Client_Result_OK){
        fprintf(stderr, "Client failed to queue initial message (buffer full?)\n");
    }

    tcp_client_work(&client);

    while(1){
        
        TCP_Server_Result sr = tcp_server_work(&server);
        if(sr != TCP_Server_Result_OK){
            fprintf(stderr, "Server reported error (%d), shutting down!\n", (int)sr);
            break;
        }

        TCP_Client_Result cr = tcp_client_work(&client);
        if(cr == TCP_Client_Result_Disconnected){
            fprintf(stderr, "Client disconnected. Shutting down loop.\n");
            break;
        } else if (cr != TCP_Client_Result_OK) {
            fprintf(stderr, "Client reported error (%d), shutting down!\n", (int)cr);
            break;
        }
        
        sleep(1);
    }
    
    printf("Shutting down!\n");

    if(tcp_client_disconnect(&client) != TCP_Client_Result_OK){
        fprintf(stderr, "Client disconnect failed, forcing close.\n");
        socket_close(&client.socket);
    }

    if(tcp_server_dispose(&server) != TCP_Server_Result_OK){
        fprintf(stderr, "Server dipose failed, forcing close.\n");
        socket_close(&server.socket);
    }
    
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