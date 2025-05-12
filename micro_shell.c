#include <stdio.h>    // For printf(), perror(), fgets()
#include <stdlib.h>   // For malloc(), free(), exit()
#include <string.h>   // For strlen(), strtok(), strcmp(), strdup()
#include <unistd.h>   // For fork(), execvp(), chdir(), getcwd(), setenv(), pipe(), dup2()
#include <sys/wait.h> // For wait()
#include <fcntl.h>    // For open(), O_WRONLY, O_CREAT, O_TRUNC, O_APPEND

// Structure to store local variables
typedef struct {
    char **names;
    char **values;
    int size;
    int capacity;
} VarTable;

void initVarTable(VarTable *vt) {
    vt->names = (char **)malloc(16 * sizeof(char *));
    vt->values = (char **)malloc(16 * sizeof(char *));
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
        vt->names = (char **)realloc(vt->names, vt->capacity * sizeof(char *));
        vt->values = (char **)realloc(vt->values, vt->capacity * sizeof(char *));
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

// Execute a single command (with redirection if specified)
void executeCommand(char **args, int arg_count, VarTable *vt, int *pipefd, int pipe_out, int pipe_in) {
    char *cmd_args[64];
    int cmd_arg_count = 0;
    int fd = -1;
    int append = 0;

    // Parse for redirection
    for (int i = 0; i < arg_count; i++) {
        if (strcmp(args[i], ">") == 0 || strcmp(args[i], ">>") == 0) {
            if (i + 1 >= arg_count) {
                printf("Invalid command\n");
                return;
            }
            if (strcmp(args[i], ">>") == 0) append = 1;
            fd = open(args[i + 1], O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC), 0644);
            if (fd < 0) {
                perror("File open failed");
                return;
            }
            break;
        }
        cmd_args[cmd_arg_count] = args[i];
        cmd_arg_count++;
    }
    cmd_args[cmd_arg_count] = NULL;

    // Handle built-in commands
    if (strcmp(cmd_args[0], "echo") == 0) {
        if (pipe_out) dup2(pipefd[1], STDOUT_FILENO);
        if (pipe_in) dup2(pipefd[0], STDIN_FILENO);
        if (fd >= 0) dup2(fd, STDOUT_FILENO);
        for (int i = 1; i < cmd_arg_count; i++) {
            if (cmd_args[i][0] == '$') {
                char *var_name = cmd_args[i] + 1;
                char *var_value = getVar(vt, var_name);
                if (var_value) {
                    printf("%s", var_value);
                } else {
                    printf("$%s", var_name);
                }
            } else {
                printf("%s", cmd_args[i]);
            }
            if (i < cmd_arg_count - 1) printf(" ");
        }
        printf("\n");
        if (fd >= 0) close(fd);
        return;
    } else if (strcmp(cmd_args[0], "pwd") == 0) {
        if (pipe_out) dup2(pipefd[1], STDOUT_FILENO);
        if (pipe_in) dup2(pipefd[0], STDIN_FILENO);
        if (fd >= 0) dup2(fd, STDOUT_FILENO);
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("getcwd() error");
        }
        if (fd >= 0) close(fd);
        return;
    } else if (strcmp(cmd_args[0], "cd") == 0) {
        if (cmd_arg_count == 1) {
            chdir(getenv("HOME"));
        } else if (chdir(cmd_args[1]) != 0) {
            perror("cd failed");
        }
        return;
    }

    // External command
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid == 0) {
        if (pipe_out) {
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[0]);
            close(pipefd[1]);
        }
        if (pipe_in) {
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            close(pipefd[1]);
        }
        if (fd >= 0) {
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        execvp(cmd_args[0], cmd_args);
        perror("Command not found");
        exit(1);
    }
    if (fd >= 0) close(fd);
}

int microshell_main(int argc, char *argv[]) {
    VarTable vt;
    initVarTable(&vt);

    char command[256];
    char *args[64];
    int arg_count;

    while (1) {
        printf("Micro Shell Prompt > ");
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
            *eq = '\0';
            char *name = args[0];
            char *value = eq + 1;
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
            for (int i = 0; i < arg_count; i++) free(args[i]);
            continue;
        }

        // Handle pipelines
        int pipefd[2];
        int start = 0;
        int pipe_in = 0;

        for (int i = 0; i <= arg_count; i++) {
            if (i == arg_count || strcmp(args[i], "|") == 0) {
                char *cmd_args[64];
                int cmd_arg_count = 0;
                for (int j = start; j < i; j++) {
                    cmd_args[cmd_arg_count++] = args[j];
                }
                cmd_args[cmd_arg_count] = NULL;

                int pipe_out = (i < arg_count); // More commands after this one?
                if (pipe_out) {
                    if (pipe(pipefd) < 0) {
                        perror("Pipe failed");
                        break;
                    }
                }

                pid_t pid = fork();
                if (pid < 0) {
                    perror("Fork failed");
                    exit(1);
                } else if (pid == 0) {
                    executeCommand(cmd_args, cmd_arg_count, &vt, pipefd, pipe_out, pipe_in);
                    exit(0);
                }

                if (pipe_in) {
                    close(pipefd[0]);
                    close(pipefd[1]);
                }
                if (pipe_out) {
                    close(pipefd[1]);
                    pipe_in = 1;
                }
                start = i + 1;
            }
        }

        // Wait for all child processes
        while (wait(NULL) > 0);

        // Free arguments
        for (int i = 0; i < arg_count; i++) {
            free(args[i]);
        }
    }

    return 0;
}