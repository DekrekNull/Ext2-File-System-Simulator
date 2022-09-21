// Derek Wright 04/19/2022
// This file contains the functions for the readlink and symlink commands for the EXT2 file system simulator.

#include "functions.h"

int my_symlink(char *old_file, char *new_file)
{
  printf("linking: oldfile=%s new_file=%s\n", old_file, new_file);
    
  // Check that old_file is not a directory, cannot link directories
  if (getino(old_file) == 0) {
    printf("%s does not exist\n", old_file);
    return 0;
  }
  
  // Check that new_file does not yet exist
  if (getino(new_file) != 0) {
    printf("%s already exists\n", new_file);
    return 0;
  }
  
  char *parent = getdirname(new_file);
  char *child = getbasename(new_file);
  printf("new_file: parent=%s child=%s\n", parent, child);
  
  int pino = getino(parent);
  MINODE *pmip = iget(dev, pino);
  my_creat(new_file);
  
  int ino = getino(child);
  MINODE *mip = iget(dev, ino);
  mip->INODE.i_mode = 0xA1FF;
  mip->INODE.i_size = strlen(old_file);
  
  put_block(dev, mip->INODE.i_block[0], old_file);
  
  mip->dirty = 1;
  iput(mip);
  pmip->dirty = 1;
  iput(pmip);
}

int my_readlink(char *pathname, char *buf)
{
  int ino = getino(pathname);
  MINODE *mip = iget(dev, ino);
  
  if ((mip->INODE.i_mode & 0xF000) != 0xA000) {
    printf("%s is not a LNK file\n", pathname);
    return 0;
  }
  
  get_block(dev, mip->INODE.i_block[0], buf);
  return mip->INODE.i_size;
}
