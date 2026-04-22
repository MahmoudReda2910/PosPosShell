#include "shell.h"

void execute_a_fork_family(char ** args , int background){
  pid_t pid = fork();

  if(pid == 0){
    execvp(args[0],args);
    perror("Command Not Found");
    exit(EXIT_FAILURE);

  }

  else if (pid > 0 ){
    if(!background){
      waitpid(pid,NULL,0);
    }
    else {
      printf("Background PID: %d\n",pid);
    }
  }
  else {
    perror("Fork Failed");
  }
}

//  pipe

void execute_pipe(char *cmd1, char *cmd2) {
    char *args1[MAX_ARGS];
    char *args2[MAX_ARGS];

    parse_args(cmd1, args1);
    parse_args(cmd2, args2);

    int pipefd[2]; // pipefd[0] = read , pipefd[1] = write

    if (pipe(pipefd) == -1) {
        perror("Pipe Failed");
        return;
    }

    pid_t pid1 = fork();

    if (pid1 == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        execvp(args1[0], args1);
        perror("Command Not Found");
        exit(EXIT_FAILURE);
    }

    pid_t pid2 = fork();

    if (pid2 == 0) {
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);

        execvp(args2[0], args2);
        perror("Command Not Found");
        exit(EXIT_FAILURE);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

void execute_command(char *input) {
  char* args[MAX_ARGS];

  char input_cpy[MAX_INPUT];
  strcpy(input_cpy, input);

  // Check on pipe
  char *pipe_pos = strchr(input_cpy, '|');
  if (pipe_pos != NULL) {
      *pipe_pos = '\0';
      char *cmd1 = input_cpy;
      char *cmd2 = pipe_pos + 1;
      execute_pipe(cmd1, cmd2);
      return;
  }

  int arg_count = parse_args(input, args);
  
  if(arg_count == 0){
    return;
  }

  if(handle_commands(arg_count, args))
    return;

  int background = 0;
  if(strcmp(args[arg_count - 1], "&") == 0)
  {
    background = 1;
    args[arg_count - 1] = NULL;
  }

  execute_a_fork_family(args, background);
}
