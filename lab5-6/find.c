#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char*
lastname(char *path)
{
  char *p;
  for(p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  return p + 1;
}

void
find(char *path, char *name, int show_inum)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }
  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }
  if(strcmp(lastname(path), name) == 0){
    if(show_inum)
      printf("%s %d\n", path, (int)st.ino);
    else
      printf("%s\n", path);
  }
  if(st.type == T_DIR){
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)){
      fprintf(2, "find: path too long\n");
      close(fd);
      return;
    }
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      find(buf, name, show_inum);
    }
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  if(argc < 3){
    fprintf(2, "usage: find <dir> <name> [-n]\n");
    exit(1);
  }
  int show_inum = (argc > 3 && strcmp(argv[3], "-n") == 0);
  find(argv[1], argv[2], show_inum);
  exit(0);
}
