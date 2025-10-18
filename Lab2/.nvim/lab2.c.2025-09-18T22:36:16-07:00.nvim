#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
int main(void) {

  char *buff = NULL;
  size_t bufflen = 0;
  while (1) {

    ssize_t programs;
    printf("Enter programs to run.\n");
    printf(">");
    programs = getline(&buff, &bufflen, stdin);
    if (programs == -1) {
      break;
    }
    if (programs > 0 && buff[programs - 1] == '\n') {
      buff[programs - 1] = '\0';
    }

    pid_t pid = fork();

    if (pid == 0) {
      // child
      execl(buff, buff, (char *)NULL);

      printf("Exec failure\n");
      exit(1);
    } else {
      // parent
      int status;
      if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
      }
    }
  }
  free(buff);
  return 0;
}
