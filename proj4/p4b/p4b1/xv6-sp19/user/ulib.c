#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"
#include "param.h"

// typedef struct bad_at_naming {
//   void* origin;
//   void* aligned;
// } aligned2origin;

// aligned2origin ao_table[NPROC];

int original[NPROC];
int aligned[NPROC];

char*
strcpy(char *s, char *t)
{
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
    ;
  return os;
}

int
strcmp(const char *p, const char *q)
{
  while(*p && *p == *q)
    p++, q++;
  return (uchar)*p - (uchar)*q;
}

uint
strlen(char *s)
{
  int n;

  for(n = 0; s[n]; n++)
    ;
  return n;
}

void*
memset(void *dst, int c, uint n)
{
  stosb(dst, c, n);
  return dst;
}

char*
strchr(const char *s, char c)
{
  for(; *s; s++)
    if(*s == c)
      return (char*)s;
  return 0;
}

char*
gets(char *buf, int max)
{
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
    cc = read(0, &c, 1);
    if(cc < 1)
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
  return buf;
}

int
stat(char *n, struct stat *st)
{
  int fd;
  int r;

  fd = open(n, O_RDONLY);
  if(fd < 0)
    return -1;
  r = fstat(fd, st);
  close(fd);
  return r;
}

int
atoi(const char *s)
{
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
    n = n*10 + *s++ - '0';
  return n;
}

void*
memmove(void *vdst, void *vsrc, int n)
{
  char *dst, *src;
  
  dst = vdst;
  src = vsrc;
  while(n-- > 0)
    *dst++ = *src++;
  return vdst;
}

int //@thread_create
thread_create(void (*start_routine)(void *, void *), void *arg1, void *arg2) {
  //uint pgsize = 1024;
  void* origin = malloc(4096 * 2);
  if (origin == 0) {
    //printf(1, "malloc failed\n");
    return -1;
  }
  void* stack = (void *)(((uint)origin + 4096 - 1) & ~(4096 - 1));
  for (int i = 0; i < NPROC; i++) {
    if (original[i] == 0) {
      original[i] = (int)origin;
      aligned[i] = (int)stack;
      return clone(start_routine, arg1, arg2, stack);
    }
  }
  return -1;
}

int // @thread_join
thread_join() {
  void* stack;
  int pid = join(&stack);
  if (pid >= 0) {
    for (int i = 0; i < NPROC; i++) {
      if (aligned[i] == (int)stack) {
        free((void*)original[i]);
        aligned[i] = 0;
        original[i] = 0;
        break;
      }
    }
  } else {
    printf(1, "thread_join failed\n");
  }
  
  return pid;
}

void // @ lock
lock_acquire(lock_t * lock) {
  int my_turn = fetch_and_add(&lock->ticket, 1);
  while(lock->turn != my_turn) {
  }
}

void // @lock
lock_release(lock_t * lock) {
  fetch_and_add(&lock->turn, 1);
}

void // @lock
lock_init(lock_t * lock) {
  lock->ticket = 0;
  lock->turn = 0;
}

int fetch_and_add(int* variable, int value) {
    __asm__ volatile("lock; xaddl %0, %1"
      : "+r" (value), "+m" (*variable) // input+output
      : // No input-only
      : "memory"
    );
    return value;
}
