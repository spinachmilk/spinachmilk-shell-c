#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
    // exit logic
    if (strcmp(command, "exit") == 0){
      break;
    }
    // echo logic
    char *first_word = strtok(command_copy, " ");
    if (strcmp(first_word, "echo") == 0){
      char *rest_of_string = command + 5;
      printf("%s\n", rest_of_string);
      continue;
    }
    // type logic
    if (strcmp(first_word, "type") == 0){
      char *second_word = strtok(NULL, " ");
      if (strcmp(second_word, "echo") == 0 || strcmp(second_word, "type") == 0 || strcmp(second_word, "exit") == 0){
        printf("%s is a shell builtin\n", second_word);
      } else{
        char *path_env = getenv("PATH");
        char *path_copy = strdup(path_env);
        int found = 0;
        char *dir = strtok(path_copy, ":");
        while (dir != NULL){
          char full_path[1024];
          snprintf(full_path, sizeof(full_path), "%s/%s", dir, second_word);
          if (access(full_path, X_OK) == 0){
            printf("%s is %s\n", second_word, full_path);
            found = 1;
            break;
          }
          dir = strtok(NULL, ":");
        }
        if (!found){
          printf("%s: not found\n", second_word);
        }
        free(path_copy);
      }
      continue;
    }
    printf("%s: command not found\n", command);
  }
  return 0;
}