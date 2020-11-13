#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>

#define SIZE 1024

int pwd(int argc, char* args[])
{
  char cwd[SIZE];

  if(getcwd(cwd, sizeof(cwd)) == NULL)
  {
    fprintf(stderr, "pwd: %s\n", strerror(errno));
    return 1;
  }

  printf("%s\n", cwd);

  return 0;
}
