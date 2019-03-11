#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "param.h"

#define assert(x) if (x) {} else { \
  printf(1, "%s: %d ", __FILE__, __LINE__); \
  printf(1, "assert failed (%s)\n", # x); \
  printf(1, "TEST FAILED\n"); \
  exit(); \
}

void
test_failed()
{
  printf(1, "TEST FAILED\n");
  exit();
}

void
test_passed()
{
  printf(1, "TEST PASSED\n");
  exit();
}

int
main(int argc, char *argv[])
{
  char *ptr1, *ptr2;//, *arg;
  int i;

  ptr1 = shmget(0);
  assert(ptr1 != NULL);

  ptr2 = shmget(0);
  assert(ptr2 != NULL);

  assert(ptr1 == ptr2);
  char arr[4] = "tmp";
  for (i = 0; i < 4; i++) {
    *(ptr2+i) = arr[i];
  }

  //argstr
  //printf(1, "%d\n", ptr2);
  //printf(1, "%c\n", *ptr2);
  int fd = open(ptr2, O_WRONLY|O_CREATE);
  assert(fd != -1);

  //argptr
  int n = write(fd, ptr2, 10);
  assert(n != -1);

  //making sure edge case is checked
  n = write(fd, (char *)(ptr2 + 4094), 10);
  assert (n == -1);

 assert(write(fd,(char *)(0x1000 + 400), 1024) != -1);

   test_passed();
   exit();
}
