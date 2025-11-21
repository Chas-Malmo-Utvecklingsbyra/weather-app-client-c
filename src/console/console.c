#include <stdbool.h>
#include <string.h>
#include "console.h"

/* Kontrollerar om användaren vill avsluta programmet */
bool console_is_quit_command(const char *input) {
    if (!input) return false;
    
    /* exit-kommando */
    return (strcmp(input, "exit") == 0);
}