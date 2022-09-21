#include "functions.h"

MOUNT *getmptr(int dev)
{
  for (int i = 0; i < 8; i++) {
    if (mountTable[i].dev == dev) {
      return &mountTable[i];
    }
  }
  
  return NULL;
}

int display_current_mount()
{
  MOUNT *current = getmptr(dev);
  printf("%s mounted on %s\n", current->name, current->mount_name);
  return 0;
}

int is_mounted(char *filesys)
{
  for (int i = 0; i < 8; i++) {
    if (strcmp(mountTable[i].name, filesys) == 0) {
      return mountTable[i].dev;
    }
  }
  
  return 0;
}

int my_mount(char *filesys, char *mount_point)
{
  char buf[BLKSIZE];
  if (strcmp(filesys, "") == 0) {
    return display_current_mount();
  }
  
  if (is_mounted(filesys) == 1) {
    printf("filesys '%s' is already mounted\n", filesys);
    return 1;
  }
  
  int fd = open(filesys, O_RDWR);
  int newDev = fd;
  
  get_block(newDev, 1, buf);
  sp = (SUPER *)buf;

  /* verify it's an ext2 file system ***********/
  if (sp->s_magic != 0xEF53){
      printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
      return 1;
  }
  
  int ino = getino(mount_point);
  MINODE *mip = iget(dev, ino);
  
  if ((mip->INODE.i_mode & 0xF000) != 0x4000) {
    printf("moint point '%s' is not a dir\n", mount_point);
    iput(mip);
    return 1;
  }

  MOUNT *newMount = getmptr(0);
  if (!newMount) {
    printf("no available mount tables for allocation\n");
    iput(mip);
    return 1;
  }
  
  get_block(dev, 2, buf); 
  gp = (GD *)buf;
  
  newMount->dev = newDev;
  newMount->ninodes = sp->s_inodes_count;
  newMount->nblocks = sp->s_blocks_count;
  newMount->bmap = gp->bg_block_bitmap;
  newMount->imap = gp->bg_inode_bitmap;
  newMount->iblk = gp->bg_inode_table;
  strcpy(newMount->name, filesys);
  newMount->name[strlen(filesys)] = 0;
  strcpy(newMount->mount_name, mount_point);
  newMount->mount_name[strlen(mount_point)] = 0;
  
  mip->mounted = 1;
  mip->mptr = newMount;
  newMount->mounted_inode = mip;
  
  
  
  printf("mount : mounted %s on %s\n", filesys, mount_point); 
  
  return 0;
}

int mount_busy(int fd) {
  for (int i = 0; i < ninodes; i++) {
    if (minode[i].dev == fd) {
      if (minode[i].refCount > 0) {
        return 1;
      }
    }
  }
  return 0;
}

int my_unmount(char *filesys)
{
  int fd = is_mounted(filesys);
  if (fd == 0) {
    printf("filesys '%s' is not mounted\n", filesys);
    return 1;
  }
  
  if (mount_busy(fd) == 1) {
    printf("filesys '%s' is busy and cannot be unmounted\n", filesys);
  }
  
  MOUNT *mount = getmptr(fd);
  MINODE *mip = mount->mounted_inode;
  
  mip->mounted = 0;
  iput(mip);
  return 0;
}
