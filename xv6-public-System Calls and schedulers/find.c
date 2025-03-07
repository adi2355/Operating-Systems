#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

void
find(char *path, char *name, char type, int inum, char inum_op, int printi)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0)
    return;

  if(fstat(fd, &st) < 0){
    close(fd);
    return;
  }

  switch(st.type){
  case T_FILE:
    if(type == 0 || type == 'f'){
      if(name == 0 || strcmp(path + strlen(path) - strlen(name), name) == 0) {
        if(inum == 0 || 
          (inum_op == '=' && st.ino == inum) ||
          (inum_op == '+' && st.ino > inum) ||
          (inum_op == '-' && st.ino < inum)) {
          if(printi)
            printf(1, "%d ", st.ino);
          printf(1, "%s\n", path);
        }
      }
    }
    break;

  case T_DIR:
    if(type == 'd' && (name == 0 || strcmp(path + strlen(path) - strlen(name), name) == 0)) {
      if(inum == 0 || 
        (inum_op == '=' && st.ino == inum) ||
        (inum_op == '+' && st.ino > inum) ||
        (inum_op == '-' && st.ino < inum)) {
        if(printi)
          printf(1, "%d ", st.ino);
        printf(1, "%s\n", path);
      }
    }
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf(1, "find: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf(1, "find: cannot stat %s\n", buf);
        continue;
      }
      if(strcmp(de.name, ".") != 0 && strcmp(de.name, "..") != 0)
        find(buf, name, type, inum, inum_op, printi);
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  char *path = ".";
  char *name = 0;
  char type = 0;
  int inum = 0;
  char inum_op = '=';
  int printi = 0;
  int i;

  for(i = 1; i < argc; i++){
    if(strcmp(argv[i], "-name") == 0){
      if(++i < argc)
        name = argv[i];
      else {
        printf(2, "find: missing argument to -name\n");
        exit();
      }
    } else if(strcmp(argv[i], "-type") == 0){
      if(++i < argc && (argv[i][0] == 'f' || argv[i][0] == 'd'))
        type = argv[i][0];
      else {
        printf(2, "find: invalid argument to -type\n");
        exit();
      }
    } else if(strcmp(argv[i], "-inum") == 0){
      if(++i < argc){
        if(argv[i][0] == '+' || argv[i][0] == '-'){
          inum_op = argv[i][0];
          inum = atoi(argv[i] + 1);
        } else {
          inum = atoi(argv[i]);
        }
      } else {
        printf(2, "find: missing argument to -inum\n");
        exit();
      }
    } else if(strcmp(argv[i], "-printi") == 0){
      printi = 1;
    } else if(path == ".") {
      path = argv[i];
    } else {
      printf(2, "Usage: find <path> -name <name> [-type f|d] [-inum [+-]<number>] [-printi]\n");
      exit();
    }
  }

  find(path, name, type, inum, inum_op, printi);
  exit();
}