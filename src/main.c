#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "core/tcp/tcp_client/tcp_client.h"
#include "core/http/http.h"
#include "core/http/httpClient/httpClient.h"
#include "core/http/parser.h"
#include "core/locationiq/locationiq.h"

#define SERVER_ADDR "malmo.onvo.se"
#define TEST_PORT 10280

static void on_received_full_message(HTTPClient *client){
    printf("%s", client->inbuffer);

    // We don't want to parse HTTP here, it should be inside the httpclient so that it could be retreived here -- fix later
}


int main() 
{
    HTTPClient client;    
    
    HTTPClient_Initiate(&client, on_received_full_message);

    printf("\n\n===============================================================\n");
    printf("Hello & Welcome to Chas Malmö Utvecklingsbyrå's Weather Client!\n");
    printf("===============================================================\n");

    while(true){

        printf("\nPlease select one of the following two options:\n\nPress 1: to write the name of a location.\nPress 2: to input the latitude and longitude of the location.\nHowever, if you would like to exit the program just enter 'e' or 'q'\n");
        printf("\nPlease make your choice: \n");
        char input_buffer[64];
        memset(input_buffer, 0, sizeof(input_buffer));
        scanf("%63s", input_buffer);
        getchar();
        if(strcmp(input_buffer, "exit") == 0 || strcmp(input_buffer, "quit") == 0 || strcmp(input_buffer, "e") == 0 || strcmp(input_buffer, "q") == 0){
            break;
        }
        uint8_t selected_menu_choice = atoi(input_buffer);
        float latitude = 0.0f;
        float longitude = 0.0f;

        switch(selected_menu_choice){

            case 1:{
                // Contacts location IQ and gets latitude and longitude to work with by letting the user input the name of a location which provides the user with a list of matching location names.
                char input[128];
                Coordinates coords;
                memset(&coords, 0, sizeof(Coordinates));

                printf("Please enter city name:\n");
                fgets(input, sizeof(input), stdin);
                input[strcspn(input, "\n")] = 0;

                // Added a choice for the user to redo it's location search if they want to, without having them to choose one of the found cities. It takes them back to.
                if(strcmp(input, "0") == 0){
                    break;
                }

                if(strlen(input) == 0){
                    printf("City name cannot be empty. Try again.\n");
                    continue;
                }

                LOCATIONIQ_RESULT result = locationiq_get_coordinates(&coords, input);

                if(result == LOCATIONIQ_RESULT_USER_ABORTED){
                    continue;
                }

                if(result != LOCATIONIQ_RESULT_OK){
                    printf("No matching cities found.\n");
                    continue;
                }

                latitude = coords.lat;
                longitude = coords.lon;

                printf("Found city: %s\n", coords.location);
                printf("lat=%.6f, lon=%.6f\n\n", latitude, longitude);

                break;    
            }
            
            case 2:{

                while(true){
                    char latitude_str[32];
                    char longitude_str[32];
                    float lat, lon;

                    printf("Please enter latitude (-90 to 90):\n");
                    fgets(latitude_str, sizeof(latitude_str), stdin);
                    lat = atof(latitude_str);

                    printf("Please enter longitude (-180 to 180):\n");
                    fgets(longitude_str, sizeof(longitude_str), stdin);
                    lon = atof(longitude_str);

                    if(lat < -90 || lat > 90){
                        printf("Invalid latitude value. Try again.\n");
                        continue;
                    }
                    if(lon < -180 || lon > 180){
                        printf("Invalid longitude value. Try again.\n");
                        continue;
                    }

                    latitude = lat;
                    longitude = lon;                    
                    break;
                }

                break;
            }

            default:{
                printf("Invalid choice\n");
                continue;
            }
        }
               
        printf("Getting weather for latitude: %.4f longitude: %.4f\n", latitude, longitude);
        
        char route_buffer[512];
        memset(route_buffer, 0, sizeof(route_buffer));
        snprintf(route_buffer, sizeof(route_buffer), "/weather?latitude=%.4f&longitude=%.4f\n", latitude, longitude);

        HTTPClient_Reset(&client);

        HTTPClient_GET(&client, "155.4.19.176", route_buffer);

        while(HTTPClient_Work(&client) == false){

        }

        // Client cleanup, right now it contains crap -- fix that because we can't reuse the same client right now, which we should be able to. LOOKUP!
        // Fix inside tcp_client

        /* printf("Input buffer is: '%s'\n", input_buffer); */

        /* 
        TODO - example of program flow
            DONE -- 0. Welcome the user
            DONE -- 1. Prompt the user to input a city name (or lat and long) for which the weather information should be retrieved.
            DONE -- 2. Waiting for stdin
            DONE -- 3. Open menu - where do you want to get weather info from? 
                        (If entering a city name, our program should translate to lat and long, and get information) 
                        if there is conflicting information(ex. several different cities with the same name), the user will have to select which one they want to see the weather info for.
            DONE -- 4. Request to the server with this lat and long to get weather info
            DONE -- 5. Wait for response
            DONE -- 6. Print response
            DONE -- 7. Loop back to step 1

        */
        
        /* while(true){
            bool done = HTTPClient_Work(&client);
            if(done == true){
                break;
            }

        } */
    }

    HTTPClient_Dispose(&client);
    printf("Goodbye and thank you for using our Weather Client!\n");


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