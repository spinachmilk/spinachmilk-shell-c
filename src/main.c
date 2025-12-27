#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

int find_path(const char *command, char *out_path){
  char *path_env = getenv("PATH");
  if (!path_env) return 0;

  char *path_copy = strdup(path_env);
  char *dir = strtok(path_copy, ":");
  while (dir != NULL){
    snprintf(out_path, 1024, "%s/%s", dir, command);
    if(access(out_path, X_OK) == 0){
      char full_path[1024];
      if (realpath(out_path, full_path)){
        strcpy(out_path, full_path);
      }
      free(path_copy);
      return 1;
    }
    dir = strtok(NULL, ":");
  }
  free(path_copy);
  return 0;
}

int parse_args(char *input, char **args){
  int args_count = 0;
  char *p = input;
  int in_single_quotes = 0;
  int in_double_quotes = 0;

  while (*p == ' ') p++;
  if (*p == '\0') return 0;
  args[args_count++] = p; // first argument starts here

  while (*p){
    if((*p == '\'') && !in_double_quotes){
      in_single_quotes = !in_single_quotes;
      memmove(p, p + 1, strlen(p));
      continue;
    }
    if ((*p == '\"') && !in_single_quotes){
      in_double_quotes = !in_double_quotes;
      memmove(p, p + 1, strlen(p));
      continue;
    }
    if (*p == ' ' && !in_single_quotes && !in_double_quotes){
      *p = '\0'; // terminate current word
      p++;
      while (*p == ' ') p++;
      if (*p != '\0'){
        args[args_count++] = p;
      }
      continue;
    }
    if (*p == '\\' && !in_single_quotes){
      char next = *(p+1);
      if (in_double_quotes){
        if (next == '\\' || next == '\n' || next == '$' || next == '\"' || next == '`'){
          memmove(p, p+1, strlen(p));
          p++;
          continue;
        }
      } else{
        memmove(p, p+1, strlen(p));
        p++;
        continue;
      }
    }
    p++;
  }
  args[args_count] = NULL;
  return args_count;
}

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);
  char command[1024];
  while (1){
    printf("$ ");
    fgets(command, sizeof(command), stdin);
    command[strcspn(command, "\n")] = '\0';
    
    char *args[128];
    int args_count = parse_args(command, args);
    if (args_count == 0) continue;
    // check for '>' operator
    int redirect_stdout_idx = -1;
    int redirect_stderr_idx = -1;
    for (int i =0; args[i] != NULL; i++){
      if((strcmp(args[i], ">") == 0) || (strcmp(args[i], "1>") == 0)){
        redirect_stdout_idx = i;
        break;
      } else if (strcmp(args[i], "2>") == 0){
        redirect_stderr_idx = i;
        break;
      }
    }
    int saved_stdout = -1;
    int saved_stderr = -1;
    // fd 1
    if (redirect_stdout_idx != -1){
      char *filename = args[redirect_stdout_idx + 1];
      if (filename == NULL){
        continue;
      }
      int fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0644);
      if (fd < 0){
        continue;
      }
      saved_stdout = dup(1);
      dup2(fd, 1);
      close(fd);
      args[redirect_stdout_idx] = NULL;
    }
    // fd 2
    if (redirect_stderr_idx != -1){
      char *filename = args[redirect_stderr_idx + 1];
      if (filename == NULL){
        continue;
      }
      int fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0644);
      if (fd < 0){
        continue;
      }
      saved_stderr = dup(2);
      dup2(fd, 2);
      close(fd);
      args[redirect_stderr_idx] = NULL;
    }

    const char *cmd = args[0];

    // exit logic
    if (strcmp(cmd, "exit") == 0){
      break;
    }

    // echo logic
    if (strcmp(cmd, "echo") == 0){
      for (int i = 1; args[i] != NULL; i++){
        printf("%s ", args[i]);
      }
      printf("\n");
      goto cleanup;
    }

    // pwd logic
    if (strcmp(cmd, "pwd") == 0){
      char cwd[1024];
      if (getcwd(cwd, sizeof(cwd)) != NULL){
        printf("%s\n", cwd);
      }
      goto cleanup;
    }

    // cd logic
    if (strcmp(cmd, "cd") == 0){
      if ((args_count == 1) || (strcmp(args[1], "~") == 0)){
        char *home = getenv("HOME");
        if (home) chdir(home);
      } else {
        if (chdir(args[1]) != 0){
          printf("cd: %s: No such file or directory\n", args[1]);
        }
      }
      goto cleanup;
    }

    // type logic
    if (strcmp(args[0], "type") == 0){
      const char *target = args[1];
      if (!target) continue;
      if (strcmp(target, "echo") == 0 || strcmp(target, "type") == 0 || strcmp(target, "exit") == 0 || strcmp(target, "pwd") == 0){
        printf("%s is a shell builtin\n", args[1]);
      } else{
        char found_path[1024];
        if (find_path(target, found_path)){
          printf("%s is %s\n", target, found_path);
        } else {
          printf("%s: not found\n", target);
        }
      }
      goto cleanup;
    } else{
      char exec_path[1024];
      if(find_path(cmd, exec_path)){
        if (fork() == 0){
          execv(exec_path, args);
          exit(1);
        } else {
          wait(NULL);
        }
      } else {
        printf("%s: command not found\n", cmd);
      }
      cleanup:
      if (saved_stdout != -1){
        dup2(saved_stdout, 1);
        close(saved_stdout);
        saved_stdout = -1;
      }
      if (saved_stderr != -1){
        dup2(saved_stderr, 2);
        close(saved_stderr);
        saved_stderr = -1;
      }
    }
  }
  return 0;
}