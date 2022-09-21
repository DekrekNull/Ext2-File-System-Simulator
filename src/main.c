/****************************************************************************
*                   KCW: mount root file system                             *
*****************************************************************************/
#include "type.h"
#include "functions.h"
#include "globals.h"

MINODE minode[NMINODE];
MINODE *root;
PROC   proc[NPROC], *running;
MOUNT  mountTable[8];
OFT    oft[64];

char  gpath[128]; // global for tokenized components
char  *name[64];  // assume at most 64 components in pathname
int   n;         // number of component strings
int   b_zero[256];

int   dev;
int   nblocks, ninodes, bmap, imap, iblk;

SUPER *sp;
GD    *gp;
INODE *ip;
DIR   *dp; 

// forward declarations of funcs in main.c
int init();
int quit();
int mount_root();

char *disk;

int init()
{
  int i, j;
  MINODE *mip;
  PROC   *p;
  OFT    *o;
  
  printf("init()\n");

  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    mip->dev = mip->ino = 0;
    mip->refCount = 0;
    mip->mounted = 0;
    mip->mptr = 0;
  }
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    p->pid = i;
    p->uid = p->gid = 0;
    p->cwd = 0;
    for (i=0; i<NFD; i++) {
      p->fd[i] = 0;
    }
  }
  for (i=0; i<64; i++) {
    o = &oft[i];
    o->refCount = 0;
  }
  
  for (i=0; i<256;i++) {
    b_zero[i] = 0;
  }
  
  for (i=0; i<8; i++) {
    mountTable[i].dev = 0;
  }
}

void display_nodes() {
  for (int i = 0; i < NMINODE; i++) {
    if (minode[i].dev == 0) {
      break;
    }
    printf("minode dev=%d ino=%d mounted=%d\n",
            minode[i].dev, minode[i].ino, minode[i].mounted);
  }
}

// load root INODE and set root pointer to it
int mount_root()
{  
  printf("mount_root()\n");
  mountTable[0].dev = dev;
  mountTable[0].ninodes = ninodes;
  mountTable[0].nblocks = nblocks;
  mountTable[0].bmap = bmap;
  mountTable[0].imap = imap;
  mountTable[0].iblk = iblk;
  strcpy(mountTable[0].mount_name, "/");
  mountTable[0].mount_name[1] = 0;
  strcpy(mountTable[0].name, disk);
  mountTable[0].name[strlen(disk)] = 0;
  root = iget(dev, 2);
}

int quit()
{
  int i;
  MINODE *mip;
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount > 0)
      iput(mip);
  }
  exit(0);
}

int main(int argc, char *argv[ ])
{
  disk = (char *)malloc(128);
  strncpy(disk, argv[1], 128);
  disk[strlen(argv[1])] = 0;
  int ino, fd;
  char buf[BLKSIZE], line[128], cmd[32], pathname[128], *argument;

  printf("checking EXT2 FS ....");
  if ((fd = open(disk, O_RDWR)) < 0){
    printf("open %s failed\n", disk);
    exit(1);
  }

  dev = fd;    // global dev same as this fd   

  /********** read super block  ****************/
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;

  /* verify it's an ext2 file system ***********/
  if (sp->s_magic != 0xEF53){
      printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
      exit(1);
  }     
  printf("EXT2 FS OK\n");
  ninodes = sp->s_inodes_count;
  nblocks = sp->s_blocks_count;

  get_block(dev, 2, buf); 
  gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;
  iblk = gp->bg_inode_table;
  printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, iblk);

  init();  
  mount_root();
  printf("root refCount = %d\n", root->refCount);

  printf("creating P0 as running process\n");
  running = &proc[0];
  running->cwd = iget(dev, 2);
  printf("root refCount = %d\n", root->refCount);

  // WRTIE code here to create P1 as a USER process
  
  while(1){
    printf("[ls|cd|pwd|quit|mkdir|creat|rmdir|link|unlink|symlink|readlink]\n");
    printf("input command from above options: ");
    fgets(line, 128, stdin);
    line[strlen(line)-1] = 0;

    if (line[0]==0)
       continue;
    pathname[0] = 0;

    sscanf(line, "%s %s", cmd, pathname);
    printf("cmd=%s pathname=%s\n", cmd, pathname);
  
    if (strcmp(cmd, "ls")==0)
       my_ls(pathname);
    else if (strcmp(cmd, "cd")==0)
       my_cd(pathname);
    else if (strcmp(cmd, "pwd")==0)
       my_pwd(running->cwd);
    else if (strcmp(cmd, "mkdir")==0)
       my_mkdir(pathname);
    else if (strcmp(cmd, "creat")==0)
       my_creat(pathname);
    else if (strcmp(cmd, "rmdir")==0)
       my_rmdir(pathname);
    else if (strcmp(cmd, "link") == 0) {
       argument = strrchr(line, ' ');
       argument++;
       argument[strlen(argument)] = 0;
       my_link(pathname, argument);
    }
    else if (strcmp(cmd, "unlink")==0)
       my_unlink(pathname);
    else if (strcmp(cmd, "symlink") == 0) {
       argument = strrchr(line, ' ');
       argument++;
       argument[strlen(argument)] = 0;
       my_symlink(pathname, argument);
    }
    else if (strcmp(cmd, "readlink")==0) {
       int sz = my_readlink(pathname, buf);
       printf("size=%d filename=%s\n", sz, buf);
    }
    else if (strcmp(cmd, "open") == 0) {
       argument = strrchr(line, ' ');
       argument++;
       argument[strlen(argument)] = 0;
       int mode = atoi(argument);
       my_open_file(pathname, mode);
    }
    else if (strcmp(cmd, "close")==0) {
       my_close_file(atoi(pathname));
    }
    else if (strcmp(cmd, "pfd") == 0) {
      pfd();
    }
    else if (strcmp(cmd, "dup")==0) {
       my_dup(atoi(pathname));
    }
    else if (strcmp(cmd, "dup2")==0) {
       argument = strrchr(line, ' ');
       argument++;
       argument[strlen(argument)] = 0;
       my_dup2(atoi(pathname), atoi(argument));
    }
    else if (strcmp(cmd, "cat")==0)
       my_cat(pathname);
    else if (strcmp(cmd, "cp") == 0) {
       argument = strrchr(line, ' ');
       argument++;
       argument[strlen(argument)] = 0;
       my_cp(pathname, argument);
    }
    else if (strcmp(cmd, "mount") == 0) {
       if (strlen(pathname) > 0) {
         argument = strrchr(line, ' ');
         argument++;
         argument[strlen(argument)] = 0;
         my_mount(pathname, argument);
         display_nodes();
       }
       else {
         display_current_mount();
       }
    }
    else if (strcmp(cmd, "unmount")==0)
       my_unmount(pathname);
    else if (strcmp(cmd, "quit")==0)
       quit();
  }
}
