#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define MAX_LINE 1024

char buf[MAX_LINE];
char prev[MAX_LINE];
int count = 0;

// Custom implementation of strncmp
int mystrncmp(const char *s1, const char *s2, int n) {
  int i;
  for (i = 0; i < n; i++) {
    if (s1[i] != s2[i] || s1[i] == '\0' || s2[i] == '\0')
      return s1[i] - s2[i];
  }
  return 0;
}

// Custom implementation of strncpy
void mystrncpy(char *dest, const char *src, int n) {
  int i;
  for (i = 0; i < n - 1 && src[i] != '\0'; i++) {
    dest[i] = src[i];
  }
  dest[i] = '\0';
}

void print_line(int c_flag, int u_flag) {
  if (count > 0 && ((!u_flag && !c_flag) || (c_flag && count >= 1) || (u_flag && count == 1))) {
    if (c_flag) {
      printf(1, "%d ", count);
    }
    printf(1, "%s\n", prev);
  }
}

void uniq(int fd, int c_flag, int u_flag, int w_flag, int w_num) {
  int n, i;
  char *p;

  memset(prev, 0, MAX_LINE);
  while ((n = read(fd, buf, MAX_LINE - 1)) > 0) {
    buf[n] = '\0';
    p = buf;
    for (i = 0; i < n; i++) {
      if (buf[i] == '\n' || i == n - 1) {
        buf[i] = '\0';
        if (w_flag) {
          if (mystrncmp(prev, p, w_num) == 0) {
            count++;
          } else {
            print_line(c_flag, u_flag);
            mystrncpy(prev, p, MAX_LINE);
            count = 1;
          }
        } else {
          if (strcmp(prev, p) == 0) {
            count++;
          } else {
            print_line(c_flag, u_flag);
            mystrncpy(prev, p, MAX_LINE);
            count = 1;
          }
        }
        p = &buf[i + 1];
      }
    }
  }
  print_line(c_flag, u_flag);
}

int main(int argc, char *argv[]) {
  int fd, i;
  int c_flag = 0, u_flag = 0, w_flag = 0;
  int w_num = 0;

  if (argc <= 1) {
    uniq(0, 0, 0, 0, 0);
    exit();
  }

  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      if (argv[i][1] == 'c')
        c_flag = 1;
      else if (argv[i][1] == 'u')
        u_flag = 1;
      else if (argv[i][1] == 'w') {
        w_flag = 1;
        if (i + 1 < argc)
          w_num = atoi(argv[++i]);
      }
    } else {
      break;
    }
  }

  if (i >= argc) {
    uniq(0, c_flag, u_flag, w_flag, w_num);
    exit();
  }

  fd = open(argv[i], 0);

  if (fd < 0) {
    printf(1, "uniq: cannot open %s\n", argv[i]);
    exit();
  }

  uniq(fd, c_flag, u_flag, w_flag, w_num);
  close(fd);
  exit();
}