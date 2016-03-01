#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char**argv) {

  // Launches the program of the same
  // name in /usr/bin/tests

  // If no args just launches the busybox shell

  char *path = NULL;
  if (argc > 1) {
    path = malloc(512);
    sprintf(path, "/usr/bin/tests/%s.sh", argv[1]);
  }

  execl("/bin/busybox", "sh", path, (char*) NULL);

}
