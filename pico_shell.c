#include <stdio.h>    // For printf(), perror()
#include <stdlib.h>   // For malloc(), free(), exit()
#include <string.h>   // For strtok(), strcmp()
#include <unistd.h>   // For fork(), execvp(), chdir(), getcwd()
#include <sys/wait.h> // For wait()

int picoshell_main(int argc, char *argv[]) {
    char command[256];  // Buffer for user input
    char *args[64];     // Array to hold command arguments
    int arg_count;      // Number of arguments

    while (1) {
        printf("Pico shell prompt > ");  // Display prompt
        fflush(stdout);  // Ensure prompt is displayed immediately

        if (fgets(command, sizeof(command), stdin) == NULL) {
            printf("Error reading input\n");
            break;
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

        // Parse command into arguments
        arg_count = 0;
        char *token = strtok(command, " ");
        while (token != NULL && arg_count < 63) {
            args[arg_count] = strdup(token);  // Dynamically allocate memory for each argument
            if (args[arg_count] == NULL) {
                perror("Memory allocation failed");
                exit(1);
            }
            arg_count++;
            token = strtok(NULL, " ");
        }
        args[arg_count] = NULL;  // Null-terminate the argument list

        // Handle built-in commands
        if (strcmp(args[0], "exit") == 0) {
            for (int i = 0; i < arg_count; i++) {
                free(args[i]);  // Free allocated memory
            }
            printf("Good Bye :)\n");
            break;
        } else if (strcmp(args[0], "echo") == 0) {
            for (int i = 1; i < arg_count; i++) {
                printf("%s", args[i]);
                if (i < arg_count - 1) printf(" ");  // Space between arguments
            }
            printf("\n");
        } else if (strcmp(args[0], "pwd") == 0) {
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("%s\n", cwd);
            } else {
                perror("getcwd() error");
            }
        } else if (strcmp(args[0], "cd") == 0) {
            if (arg_count == 1) {
                chdir(getenv("HOME"));  // Change to home directory if no arg
            } else if (chdir(args[1]) != 0) {
                perror("cd failed");
            }
        } else {
            pid_t pid = fork();
            if (pid < 0) {
                perror("Fork failed");
                for (int i = 0; i < arg_count; i++) free(args[i]);
                exit(1);
            } else if (pid == 0) {
                // Child process
                execvp(args[0], args);
                perror("Command not found");  // execvp only returns on error
                exit(1);
            } else {
                // Parent process
                int status;
                wait(&status);  // Wait for child to finish
            }
        }

        // Free dynamically allocated memory
        for (int i = 0; i < arg_count; i++) {
            free(args[i]);
        }
    }

    return 0;
}