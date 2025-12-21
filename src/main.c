#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    char *rest_of_string = first_word + strlen(first_word) + 1;
    if (strcmp(first_word, "echo") == 0){
      printf("%s\n", rest_of_string);
      continue;
    }
    // type logic
    if (strcmp(first_word, "type") == 0){
      char *second_word = strtok(NULL, " ");
      if (strcmp(second_word, "echo") == 0){
        printf("echo is a shell builtin\n");
        continue;
      }
      else if (strcmp(second_word, "exit") == 0){
        printf("exit is a shell builtin\n");
        continue;
      }
      else if (strcmp(second_word, "type") == 0){
        printf("type is a shell builtin\n");
        continue;
      }
      else{
        printf("%s: not found\n", second_word);
        continue;
      }
    }
    printf("%s: command not found\n", command);
  }
  return 0;
}