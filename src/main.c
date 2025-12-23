#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);
  char command[1024];
  char command_copy[1024];
  while (1){
    printf("$ ");
    fgets(command, sizeof(command), stdin);
    command[strcspn(command, "\n")] = '\0';
    strcpy(command_copy, command);
    char *args[128];
    int arg_count = 0;
    char *token = strtok(command_copy, " ");
    while (token != NULL && arg_count < 127){
      args[arg_count++] = token;
      token = strtok(NULL, " ");
    }
    args[arg_count] = NULL;
    // exit logic
    if (strcmp(args[0], "exit") == 0){
      break;
    }
    // echo logic
    if (strcmp(args[0], "echo") == 0){
      for (int i = 1; i<arg_count; i++){
        printf("%s", args[i]);
      }
      printf("\n");
      continue;
    }
    // type logic
    if (strcmp(args[0], "type") == 0){
      if (strcmp(args[1], "echo") == 0 || strcmp(args[1], "type") == 0 || strcmp(args[1], "exit") == 0){
        printf("%s is a shell builtin\n", args[1]);
      } else{
        char *path_env = getenv("PATH");
        char *path_copy = strdup(path_env);
        int found = 0;
        char *dir = strtok(path_copy, ":");
        while (dir != NULL){
          char full_path[1024];
          snprintf(full_path, sizeof(full_path), "%s/%s", dir, args[0]);
          if (access(full_path, X_OK) == 0){
            printf("%s is %s\n", args[1], full_path);
            found = 1;
          }
          dir = strtok(NULL, ":");
        }
        if (!found){
          printf("%s: not found\n", args[1]);
        }
        free(path_copy);
      }
      continue;
    }
    char *path_env = getenv("PATH");
    char *path_copy = strdup(path_env);
    char *dir = strtok(path_copy, ":");
    int found = 0;

    while (dir != NULL) {
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, args[0]);

        if (access(full_path, X_OK) == 0) {
            found = 1;
            pid_t pid = fork();
            if (pid == 0) {
                execv(full_path, args);
                exit(1); 
            } else {
                wait(NULL);
            }
            break;
        }
        dir = strtok(NULL, ":");
    }
    free(path_copy);

    if (!found) {
        printf("%s: command not found\n", args[0]);
    }
  }
  return 0;
}