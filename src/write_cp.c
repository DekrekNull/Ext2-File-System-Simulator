// Derek Wright 04/19/2022
// This file contains the functions for write and cp
// for the EXT2 file system simulator.

#include "functions.h"

int my_write(int fd, char buf[], int nbytes)
{
  OFT *oftp = running->fd[fd];
  MINODE *mip = oftp->minodePtr;
  int lbk, blk, startByte, remain, *indirect, *double_indirect;
  int count = 0;
  char writebuf[BLKSIZE], ibuf[BLKSIZE], ibuf2[BLKSIZE], *cq = buf;
  
  while (nbytes > 0) {
    lbk = oftp->offset / BLKSIZE;
    startByte = oftp->offset % BLKSIZE;
    
    if (lbk < 12) {
      if (mip->INODE.i_block[lbk] == 0) {
        mip->INODE.i_block[lbk] = balloc(mip->dev);
      }
      blk = mip->INODE.i_block[lbk];
    }
    else if ((lbk >= 12) && (lbk < 256 + 12)) {
      if (mip->INODE.i_block[12] == 0) {
        // Allocate Block
        mip->INODE.i_block[12] = balloc(mip->dev);
        // zero out block on disk
        get_block(mip->dev, mip->INODE.i_block[12], ibuf);
        indirect = (int *)ibuf;
        // Zero out block
        memcpy(indirect, b_zero, 256);
      }
      get_block(mip->dev, mip->INODE.i_block[12], ibuf);
      indirect = (int *)ibuf;
      blk = indirect[lbk - 12];

      if (blk == 0) {
        blk = balloc(mip->dev);
        indirect[lbk - 12] = blk;
        put_block(mip->dev, mip->INODE.i_block[12], ibuf);
      }
    }
    else {
       // Mailman algorithm
      int block = (lbk - (256 + 12)) / 256;
      int ino = (lbk - (256 + 12)) % 256;

      // Check first indirect block, allocate if needed.
      if (mip->INODE.i_block[13] == 0) {
        mip->INODE.i_block[13] = balloc(mip->dev);
        get_block(mip->dev, mip->INODE.i_block[13], ibuf);
        indirect = (int *)ibuf;
        // Zero out block
        memcpy(indirect, b_zero, 256);
      }
      
      // Get first indirect block
      get_block(mip->dev, mip->INODE.i_block[13], ibuf);
      indirect = (int *)ibuf;
      
      // Check second indirect block, allocate if needed
      if (indirect[block] == 0) {
        indirect[block] = balloc(mip->dev);
        get_block(mip->dev, indirect[block], ibuf2);
        double_indirect = (int *)ibuf2;
        // Zero out block
        memcpy(double_indirect, b_zero, 256);
      }
      // Get second indirect block
      get_block(mip->dev, indirect[block], ibuf2);
      double_indirect = (int *)ibuf2;
      
      // Check if block allocated, allocate one if not
      if (double_indirect[ino] == 0) {
        double_indirect[ino] = balloc(mip->dev);
        put_block(mip->dev, indirect[block], ibuf2);
      }
      blk = double_indirect[ino];
    }
      
    get_block(mip->dev, blk, writebuf);
    char *cp = writebuf + startByte;
    remain = BLKSIZE - startByte;
    
    int writeCount = nbytes;
    if (writeCount > remain) {
      writeCount = remain;
    }
    
    memcpy(cp, cq, writeCount);
    oftp->offset += writeCount;
    count += writeCount;
    nbytes -= writeCount;
    remain -= writeCount;
    
    if (oftp->offset > mip->INODE.i_size) {
      mip->INODE.i_size = oftp->offset;
    }
    
    put_block(mip->dev, blk, writebuf);
  }
  
  mip->dirty = 1;
  // printf("wrote %d char into file descriptor fd=%d\n", count, fd);
  return count;
}

void my_cp(char *src, char *dest)
{
  char mybuf[BLKSIZE];
  int n;
  int fd = my_open_file(src, 0);
  int gd = my_open_file(dest, 1);
  printf("opened files\n");
  
  while (n = my_read(fd, mybuf, BLKSIZE)) {
    my_write(gd, mybuf, n);
  }
  
  my_close_file(fd);
  my_close_file(gd);
}

void my_mv(char *src, char *dest)
{
  int ino = getino(src);
  if (ino == 0) {
    printf("FILE '%s' does not exist\n", src);
    return;
  }
  
}
