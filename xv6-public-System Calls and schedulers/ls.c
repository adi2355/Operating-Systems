#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"


char*
fmtname(char *path, int is_dir)
{
  static char buf[DIRSIZ+2];
  char *p;

  // Find first character after last slash
  for(p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

   // Return blank-padded name.
  int fname_len = strlen(p);
  memset(buf, 0, sizeof(buf));
  memmove(buf, p, fname_len < DIRSIZ ? fname_len : DIRSIZ);
  
  if (is_dir && fname_len < DIRSIZ) {
    buf[fname_len] = '/';
    fname_len++; 
  }
  memset(buf + fname_len, ' ', DIRSIZ - fname_len);

  return buf;
}

void
ls(char *path, int flag)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    printf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    printf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
    case T_FILE:
      printf(1, "%s %d %d %d\n", fmtname(path, 0), st.type, st.ino, st.size);
      break;

    case T_DIR:
      if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf(1, "ls: path too long\n");
      break;
    }
      strcpy(buf, path);
      p = buf + strlen(buf);
      *p++ = '/';

    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;

      if(!flag && de.name[0] == '.')
        continue;

      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;

      if(stat(buf, &st) < 0){
        printf(1, "ls: cannot stat %s\n", buf);
        continue;
      }

      printf(1, "%s %d %d %d\n", fmtname(buf, st.type == T_DIR), st.type, st.ino, st.size);
    }
    break;
    
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  int i;
  int flag = 0;

  for(i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-a") == 0) {
      flag = 1;
      break;
    }
  }
  if(argc == 1 || (argc == 2 && flag)){
    ls(".", flag);
  } 
  else {
    for(i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-a") != 0)  
        ls(argv[i], flag);
    }
  }
  exit();
}
