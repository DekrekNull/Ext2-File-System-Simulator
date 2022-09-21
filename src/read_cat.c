// Derek Wright 04/19/2022
// This file contains the functions for read and cat 
// for the EXT2 file system simulator.

#include "functions.h"

int my_read_file(int fd, int nbytes) {
  if (running->fd[fd] != 0) {
    if (running->fd[fd]->mode != 0 && running->fd[fd]->mode != 0) {
      printf("%d not open for reading\n", fd);
      return 0;
    }
    char *buf;
    return my_read(fd, buf, nbytes);
  }
  printf("%d not open\n", fd);
  return 0;
}

int my_read(int fd, char buf[], int nbytes)
{
  OFT *oftp = running->fd[fd];
  MINODE *mip = oftp->minodePtr;
  int lbk, blk, startByte, remain, *indirect, *double_indirect;
  int count = 0;
  
  int avil = mip->INODE.i_size - oftp->offset;
  char readbuf[BLKSIZE], ibuf[BLKSIZE], ibuf2[BLKSIZE], *cq = buf;
  
  while ((nbytes > 0) && (avil > 0)) {
    lbk = oftp->offset / BLKSIZE;
    startByte = oftp->offset % BLKSIZE;
    
    if (lbk < 12) {
      blk = mip->INODE.i_block[lbk];
    }
    else if ((lbk >= 12) && (lbk < 256 + 12)) {
      get_block(mip->dev, mip->INODE.i_block[12], ibuf);
      blk = ((int *)ibuf)[lbk-12];
    }
    else {
      // Mailman algorithm
      int block = (lbk - (256 + 12)) / 256;
      int ino = (lbk - (256 + 12)) % 256;
      
      get_block(mip->dev, mip->INODE.i_block[13], ibuf);
      indirect = (int *)ibuf;
      
      get_block(mip->dev,indirect[block], ibuf2);
      double_indirect = (int *)ibuf2;
      
      blk = double_indirect[ino];
    }
    get_block(mip->dev, blk, readbuf);
    
    char *cp = readbuf + startByte;
    remain = BLKSIZE - startByte;
    
    int readCount = nbytes;
    if (readCount > avil) {
      readCount = avil;
    }
    if (readCount > remain) {
      readCount = remain;
    }
    
    memcpy(cq, cp, readCount);
    oftp->offset += readCount;
    count += readCount;
    nbytes -= readCount;
    avil -= readCount;
    remain -= readCount;
  }
  // printf("myread: read %d char from file descriptor %d\n", count, fd);
  return count;
}

void my_cat(char *filename)
{
  char mybuf[BLKSIZE], dummy = 0;
  int n;
  
  int fd = my_open_file(filename, 0);
  printf("fd=%d\n", fd);

  while (n = my_read(fd, mybuf, BLKSIZE)) {
    mybuf[n] = dummy;
    char *cp = mybuf;
    while (*cp != dummy) {
      printf("%c", *cp);
      cp++;
    }
  }

  my_close_file(fd);
}
