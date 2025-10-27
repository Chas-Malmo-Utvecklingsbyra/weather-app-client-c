#include <stdio.h>

int main() {
    printf("Hello, world! I am the (C) client.\n");


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