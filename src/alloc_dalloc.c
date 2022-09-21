// Derek Wright 04/19/2022
// This file contains functions required for allocating and deallocating blocks and INODES
// for the EXT2 filesystem simulator.

#include "functions.h"

int tst_bit(char *buf, int bit)
{
  int i = bit / 8;
  int j = bit % 8;
  
  if (buf[i] && (1 << j)) {
    return 1;
  }
  return 0;
}

int set_bit(char *buf, int bit)
{
  int i = bit / 8;
  int j = bit % 8;
  buf[i] |= (1 << j);
  return 1;
}

int clr_bit(char *buf, int bit) {
  int i = bit / 8;
  int j = bit % 8;
  buf[i] &= -(1 << j);
  return 1;
}

int changeFreeInodes(int dev, int change)
{
// dec free inodes count in SUPER and GD
  SUPER *sp;
  GD *gp;
  
  char buf[BLKSIZE];
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count += change;
  put_block(dev, 1, buf);
  
  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count += change;
  put_block(dev, 2, buf);
}

int changeFreeBlocks(int dev, int change) {
  // dec free blocks count in SUPER and GD
  SUPER *sp;
  GD *gp;
  
  char buf[BLKSIZE];
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count += change;
  put_block(dev, 1, buf);
  
  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count += change;
  put_block(dev, 2, buf);
}

int decFreeInodes(int dev) {
  changeFreeInodes(dev, -1);
}

int incFreeInodes(int dev) {
  changeFreeInodes(dev, 1);
}

int decFreeBlocks(int dev) {
  changeFreeBlocks(dev, -1);
}

int incFreeBlocks(int dev) {
  changeFreeInodes(dev, 1);
}

// Allocates an inode number from inode_bitmap
int ialloc(int dev)
{
  MOUNT *mnt = getmptr(dev);
  int i;
  char buf[BLKSIZE];
  
  // read inode_nitmap block
  get_block(dev, mnt->imap, buf);
  
  // use ninodes from SUPER block
  for (i = 0; i < mnt->ninodes; i++) {
    if (tst_bit(buf, i) == 0) {
      set_bit(buf, i);
      put_block(dev, mnt->imap, buf);
      
      decFreeInodes(dev);
      
      printf("inode allocated in = %d\n", i+1); // bits count from 0 but ino counts from 1
      return i+1;
    }
  }
  return 0;
}

// Allocates a free disk block number
int balloc(int dev) 
{
  MOUNT *mnt = getmptr(dev);
  int i;
  char buf[BLKSIZE];
  
  // read inode_nitmap block
  get_block(dev, mnt->bmap, buf);
  
  // use ninodes from SUPER block
  for (i = 0; i < mnt->nblocks; i++) {
    if (tst_bit(buf, i) == 0) {
      set_bit(buf, i);
      put_block(dev, mnt->bmap, buf);
      
      decFreeBlocks(dev);
      
      printf("block allocated in = %d\n", i+1); // bits count from 0 but ino counts from 1
      return i+1;
    }
  }
  return 0;
}


int idalloc(int dev, int ino) {
  MOUNT *mnt = getmptr(dev);
  char buf[BLKSIZE];
  
  if (ino > mnt->ninodes) {
    printf("inumber %d out of range \n", ino);
    return -1;
  }
  
  // get inode bitmap block
  get_block(dev, mnt->imap, buf);
  clr_bit(buf, ino-1);
  
  // Write buf back
  put_block(dev, mnt->imap, buf);
  
  // Update free inode count in SUPER and GD
  incFreeInodes(dev);
}

int bdalloc(int dev, int bno) {
  MOUNT *mnt = getmptr(dev);
  char buf[BLKSIZE];
  
  if (bno > mnt->nblocks) {
    printf("bnumber %d out of range \n", bno);
    return -1;
  }
  
  // get inode bitmap block
  get_block(dev, mnt->bmap, buf);
  clr_bit(buf, bno-1);
  
  // Write buf back
  put_block(dev, mnt->bmap, buf);
  
  // Update free inode count in SUPER and GD
  incFreeBlocks(dev);
}
