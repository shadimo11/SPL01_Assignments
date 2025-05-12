#include <stdio.h>    // For printf(), perror(), fgets()
#include <stdlib.h>   // For malloc(), free(), exit()
#include <string.h>   // For strlen(), strtok(), strcmp(), strdup()
#include <unistd.h>   // For fork(), execvp(), chdir(), getcwd(), setenv()
#include <sys/wait.h> // For wait()

// Structure to store local variables
typedef struct {
    char **names;
    char **values;
    int size;
    int capacity;
} VarTable;

void initVarTable(VarTable *vt) {
    vt->names = (char **)malloc(16 * sizeof(char *));  // Cast malloc return to char **
    vt->values = (char **)malloc(16 * sizeof(char *)); // Cast malloc return to char **
    vt->size = 0;
    vt->capacity = 16;
    if (vt->names == NULL || vt->values == NULL) {
        perror("Memory allocation failed");
        exit(1);
    }
}

void addVar(VarTable *vt, const char *name, const char *value) {
    if (vt->size >= vt->capacity) {
        vt->capacity *= 2;
        vt->names = (char **)realloc(vt->names, vt->capacity * sizeof(char *));  // Cast realloc return
        vt->values = (char **)realloc(vt->values, vt->capacity * sizeof(char *)); // Cast realloc return
        if (vt->names == NULL || vt->values == NULL) {
            perror("Memory reallocation failed");
            exit(1);
        }
    }
    vt->names[vt->size] = strdup(name);
    vt->values[vt->size] = strdup(value);
    vt->size++;
}

char *getVar(VarTable *vt, const char *name) {
    for (int i = 0; i < vt->size; i++) {
        if (strcmp(vt->names[i], name) == 0) {
            return vt->values[i];
        }
    }
    return NULL;
}

void freeVarTable(VarTable *vt) {
    for (int i = 0; i < vt->size; i++) {
        free(vt->names[i]);
        free(vt->values[i]);
    }
    free(vt->names);
    free(vt->values);
}

int nanoshell_main(int argc, char *argv[]) {
    VarTable vt;
    initVarTable(&vt);

    char command[256];
    char *args[64];
    int arg_count;

    while (1) {
        printf("Nano Shell Prompt > ");
        fflush(stdout);

        if (fgets(command, sizeof(command), stdin) == NULL) {
            printf("Error reading input\n");
            break;
        }

        // Remove trailing newline
        size_t len = strlen(command);
        if (len > 0 && command[len - 1] == '\n') {
            command[len - 1] = '\0';
        }

        if (strlen(command) == 0) continue;

        // Parse command into arguments
        arg_count = 0;
        char *token = strtok(command, " ");
        while (token != NULL && arg_count < 63) {
            args[arg_count] = strdup(token);
            if (args[arg_count] == NULL) {
                perror("Memory allocation failed");
                exit(1);
            }
            arg_count++;
            token = strtok(NULL, " ");
        }
        args[arg_count] = NULL;

        // Handle variable assignment
        if (strchr(args[0], '=') != NULL) {
            char *eq = strchr(args[0], '=');
            *eq = '\0';  // Split into name and value
            char *name = args[0];
            char *value = eq + 1;

            // Check format
            if (strlen(name) == 0 || strlen(value) == 0 || strchr(value, ' ') != NULL || strchr(name, ' ') != NULL) {
                printf("Invalid command\n");
            } else {
                addVar(&vt, name, value);
            }
            for (int i = 0; i < arg_count; i++) free(args[i]);
            continue;
        }

        // Handle built-in commands
        if (strcmp(args[0], "exit") == 0) {
            for (int i = 0; i < arg_count; i++) free(args[i]);
            freeVarTable(&vt);
            printf("Good Bye :)\n");
            break;
        } else if (strcmp(args[0], "echo") == 0) {
            for (int i = 1; i < arg_count; i++) {
                if (args[i][0] == '$') {
                    char *var_name = args[i] + 1;
                    char *var_value = getVar(&vt, var_name);
                    if (var_value) {
                        printf("%s", var_value);
                    } else {
                        printf("$%s", var_name);  // Unsubstituted if not found
                    }
                } else {
                    printf("%s", args[i]);
                }
                if (i < arg_count - 1) printf(" ");
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
                chdir(getenv("HOME"));
            } else if (chdir(args[1]) != 0) {
                perror("cd failed");
            }
        } else if (strcmp(args[0], "export") == 0) {
            if (arg_count == 2) {
                char *eq = strchr(args[1], '=');
                if (eq && !strchr(eq + 1, ' ') && !strchr(args[1], ' ')) {
                    *eq = '\0';
                    char *name = args[1];
                    char *value = eq + 1;
                    if (setenv(name, value, 1) != 0) {
                        perror("export failed");
                    }
                } else {
                    printf("Invalid command\n");
                }
            } else {
                printf("Invalid command\n");
            }
        } else {
            pid_t pid = fork();
            if (pid < 0) {
                perror("Fork failed");
                for (int i = 0; i < arg_count; i++) free(args[i]);
                exit(1);
            } else if (pid == 0) {
                // Child process inherits environment
                execvp(args[0], args);
                perror("Command not found");
                exit(1);
            } else {
                int status;
                wait(&status);
            }
        }

        // Free dynamically allocated arguments
        for (int i = 0; i < arg_count; i++) {
            free(args[i]);
        }
    }

    return 0;
}