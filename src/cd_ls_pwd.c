/************* cd_ls_pwd.c file **************/

#include "functions.h"

char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

int my_cd(char *pathname)
{
  int ino = getino(pathname);
  if (ino == 0) {
    printf("dir %s does not exist\n", pathname);
    return -1;
  }
  MINODE *mip = iget(dev, ino);
  INODE *ip2 = &mip->INODE;
  printf("dev=%d ino=%d\n", dev, ino);
  printf("mode=%x   newip=[%d %d ref=%d]\n", ip2->i_mode, dev, ino, mip->refCount);
  if ((ip2->i_mode & 0xF000) != 0x4000) {
    printf("%s is not a directory\n", pathname);
  }
  iput(running->cwd);
  running->cwd = mip;
  printf("after cd : cwd = [%d %d]\n",running->cwd->dev, running->cwd->ino);
}

char *convert_time(time_t raw_time)
{
  struct tm *timeinfo;
  char *date_time = (char *) malloc(64);
  timeinfo = localtime(&raw_time);
  strftime(date_time, 64, "%b %d %H:%M:%S %y",timeinfo);
  return date_time;
}

void print_stats(INODE *ip2, char *fname)
{
  if ((ip2->i_mode & 0xF000) == 0x8000) {
    printf("%c", '-');
  }
  if ((ip2->i_mode & 0xF000) == 0x4000) {
    printf("%c", 'd');
  }
  if ((ip2->i_mode & 0xF000) == 0xA000) {
    printf("%c", 'l');
  }
  for (int i = 8; i >= 0; i--) {
    if (ip2->i_mode & (1 << i)) {
      printf("%c", t1[i]);
    }
    else {
      printf("%c", t2[i]);
    }
  }
}

int ls_file(MINODE *mip, char *name)
{
  INODE *ip = &mip->INODE;
  print_stats(ip, name);
  time_t raw_time = ip->i_ctime;
  char *date_time = convert_time(raw_time);
  printf("%4u%4u%4u%21s%8u    %-12s[%d %d]\n",
  ip->i_links_count,
  ip->i_gid, ip->i_uid,
  date_time,
  ip->i_size,
  name,
  mip->dev,
  mip->ino);
}

int ls_dir(MINODE *mip)
{
  char buf[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;

  get_block(dev, mip->INODE.i_block[0], buf);
  dp = (DIR *)buf;
  cp = buf;
  
  printf("ls_dir i_block[0] = %d\n",mip->INODE.i_block[0]);
  int i = 0;
  while (cp < buf + BLKSIZE){
     strncpy(temp, dp->name, dp->name_len);
     temp[dp->name_len] = 0;
     MINODE *mip2 = iget(dev, dp->inode);
     ls_file(mip2, temp);
     
     iput(mip2);
     
     cp += dp->rec_len;
     dp = (DIR *)cp;
     if (i == 10) break;
     i++;
  }
  printf("\n");
}

int my_ls(char *pathname)
{
  if (strcmp("", pathname) == 0) {
    ls_dir(running->cwd);
  }
  else {
    int myino = getino(pathname);
    MINODE *mip = iget(dev, myino);
    ls_dir(mip);
    iput(mip);
  }
}

char *rpwd(MINODE *wd) {
  if (wd == root) {
    return 0;
  }
  int pino = findino(wd);
  MINODE *pip = iget(dev, pino);
  char my_name[256];
  findmyname(pip, wd->ino, my_name);
  rpwd(pip);
  iput(pip);
  printf("/%s", my_name);
}

void my_pwd(MINODE *wd)
{
  printf("CWD = ");
  if (wd == root){
    printf("/");
  }
  else {
    rpwd(wd);
  }
  printf("\n");
}



