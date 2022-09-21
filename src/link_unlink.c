// Derek Wright 04/19/2022
// This file contains the functions for the link and unlink commands of the EXT2 file system simulator.

#include "functions.h"

int my_link(char *old_file, char *new_file) {
  printf("linking: oldfile=%s new_file=%s\n", old_file, new_file);
  
  // Get the MINODE for old_file
  int oino = getino(old_file);
  MINODE *omip = iget(dev, oino);
  
  // Check that old_file is not a directory, cannot link directories
  if ((omip->INODE.i_mode & 0xF000) == 0x4000) {
    printf("%s is a DIR\n", old_file);
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
  enter_child(pmip, oino, child);
  
  omip->INODE.i_links_count++;
  omip->dirty = 1;
  iput(omip);
  iput(pmip);
  
  return 1;
}

int my_unlink(char *pathname) {
  int ino = getino(pathname);
  MINODE *mip = iget(dev, ino);
  
  // Check that pathname is not a directory, cannot link directories
  if ((mip->INODE.i_mode & 0xF000) == 0x4000) {
    printf("%s is a DIR\n", pathname);
    return 0;
  }
  
  char *parent = getdirname(pathname);
  char *child = getbasename(pathname);
  printf("file: parent=%s child=%s\n", parent, child);
  
  int pino = getino(parent);
  MINODE *pmip = iget(dev, pino);
  rm_child(pmip, child);
  pmip->dirty = 1;
  iput(pmip);
  
  // Decrement INODE's link count by 1
  mip->INODE.i_links_count--;
  if (mip->INODE.i_links_count > 0) {
    // If link count is greater than 0, write the update back to disk
    mip->dirty = 1;
  }
  else {
    // If no more links, deallocate all datablocks in INODE, then the INODE itself
    int i = 0;
    while (mip->INODE.i_block[i] != 0) {
      bdalloc(mip->dev, mip->INODE.i_block[i]);
      i++;
    }
    idalloc(mip->dev, ino);
  }
  
  iput(mip);
  return 1;
}
