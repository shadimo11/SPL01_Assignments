#include <stdio.h>  // For printf(), fgets(), strcmp()
#include <string.h> // For strlen(), strtok()

int femtoshell_main(int argc, char *argv[]) {
    char command[256];  // Buffer to store user input

    while (1) {
        printf("Fento shell prompt > ");  // Display prompt
        fflush(stdout);  // Ensure prompt is displayed immediately

        if (fgets(command, sizeof(command), stdin) == NULL) {
            printf("Error reading input\n");
            break;  // Exit on input error
        }

        // Remove trailing newline
        size_t len = strlen(command);
        if (len > 0 && command[len - 1] == '\n') {
            command[len - 1] = '\0';
        }

        // Skip empty input
        if (strlen(command) == 0) {
            continue;
        }

        // Tokenize the command
        char *cmd = strtok(command, " ");
        if (cmd == NULL) {
            continue;
        }

        // Handle exit command
        if (strcmp(cmd, "exit") == 0) {
            printf("Good Bye :)\n");
            break;  // Exit the shell
        }

        // Handle echo command
        if (strcmp(cmd, "echo") == 0) {
            char *text = strtok(NULL, "\n");  // Get the rest of the line
            if (text != NULL) {
                while (*text == ' ') text++;  // Skip leading spaces
                if (*text != '\0') {
                    printf("%s\n", text);  // Print the text with a newline
                } else {
                    printf("\n");  // Newline for echo with no text
                }
            } else {
                printf("\n");  // Newline for echo with no arguments
            }
        } else {
            printf("Invalid command\n");  // Handle all other inputs
        }
    }

    return 0;  // Return success upon exit
}