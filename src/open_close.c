// Derek Wright 04/19/2022
// This file contains the functions for open, close and lseek 
// for the EXT2 file system simulator.

#include "functions.h"

int my_truncate(MINODE *mip)
{
  char buf1[BLKSIZE], buf2[BLKSIZE];
  int *block1, *block2, i, j;
  INODE *ip = &(mip->INODE);
  // Release direct blocks
  for(i = 0; i < 12; i++) {
    if (ip->i_block[i] == 0) {
      break;
    }
    bdalloc(mip->dev, ip->i_block[i]);
  }

  // Release indirect blocks
  if (ip->i_block[12] != 0) {
    get_block(mip->dev, ip->i_block[12], buf1);
    block1 = (int *)buf1;
    for (i = 0; i < 256; i++) {
      if (block1[i] == 0) {
        break;
      }
      bdalloc(mip->dev, block1[i]);
    }
  }
  // Release doubly indirect blocks
  if (ip->i_block[13] != 0) {
    get_block(mip->dev, ip->i_block[13], buf1);
    block1 = (int *)buf1;
    for (i = 0; i < 256; i++) {
      if (block1[i] == 0) {
        break;
      }
      
      get_block(mip->dev, block1[i], buf2);
      block2 = (int *)buf2;
      
      for (j = 0; j < 256; j++) {
        if (block2[j] == 0) {
          break;
        }
        bdalloc(mip->dev, block2[j]);
      }
    }
  }
  mip->dirty = 1;
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
  ip->i_size = 0;
  return 1;
}

int my_open_file(char *pathname, int mode)
{
  OFT *oftp;
  int i, ino;
  ino = getino(pathname);
  
  // Check that mode is valid
  // modes : 0|1|2|3 -> R|W|RW|APPEND respectively
  if (mode < 0 || mode > 4) {
    printf("mode '%d' unknown. available modes: 0|1|2|3 -> R|W|RW|APPEND respectively\n", mode);
    return 0;
  }
  
  // If file does not already exist, create it.
  if (ino == 0) {
    printf("Creating file %s\n", pathname);
    my_creat(pathname);
    ino = getino(pathname);
  }
  
  MINODE *mip = iget(dev, ino);
  // Check that pathname is a REGULAR file
  if ((mip->INODE.i_mode & 0xF000) != 0x8000) {
    printf("%s is not a regular file\n", pathname);
    iput(mip);
    return 0;
  }
  
  i = 0;
  while (oft[i].refCount != 0) {
    i++;
  }
  oftp = &oft[i];
  
  oftp->mode = mode;
  oftp->minodePtr = mip;
  oftp->refCount = 1;
  if (mode == 1) {
    my_truncate(mip);
  }
  oftp->offset = 0;
  if (mode == 3) {
    oftp->offset = mip->INODE.i_size;
  }
  
  i = 0;
  while (running->fd[i] != 0) {
    i++;
    if (i == NFD) {
      printf("no free devices left\n");
      iput(mip);
      return 0;
    }
  }
  
  running->fd[i] = oftp;
  
  mip->INODE.i_atime = time(0L);
  if (mode != 0) {
    mip->INODE.i_mtime = time(0L);
  }
  
  mip->dirty = 1;
  
  return i;
}

int my_close_file(int fd)
{
  if (fd < 0 || fd > NFD) {
    printf("device out of range\n");
    return 0;
  }
  
  if (running->fd[fd] == 0) {
    printf("file not open\n");
    return 0;
  }
  
  OFT *oftp = running->fd[fd];
  running->fd[fd] = 0;
  oftp->refCount--;
  
  // Another device has the file open, so done here
  if (oftp->refCount > 0) {
    return 0;
  }
  
  // No more devices are using it so release the minode
  iput(oft->minodePtr);
  
  return 0;
}

int my_lseek(int fd, int position)
{
  if (fd < 0 || fd > NFD) {
    printf("device out of range\n");
    return 0;
  }
  OFT *oftp = running->fd[fd];
  
  if (position < 0 || position >= oftp->minodePtr->INODE.i_size) {
    printf("lseek position out of bounds\n");
    return 0;
  }
  
  int old = oftp->offset;
  oftp->offset = position;
  return old;
}

int pfd()
{
  int count = 0;
  OFT *oftp;
  MINODE *mip;
  PROC *p;
  char mode[20];
  printf("  fd     mode    offset    INODE\n");
  printf(" ----    ----    ------   --------\n");
  
  for (int j = 0; j < NPROC; j++) {
    p = &proc[j];
    for (int i = 0; i < NFD; i++) {
      if (p->fd[i] == 0) {
        continue;
      }
      
      count++;
      oftp = (OFT *)p->fd[i];
      mip = oftp->minodePtr;
      switch (oftp->mode) {
        case 0:
          strcpy(mode, "READ");
          mode[4] = 0;
          break;
        case 1:
          strcpy(mode, "WRITE");
          mode[5] = 0;
          break;
        case 2:
          strcpy(mode, "RW");
          mode[2] = 0;
          break;
        case 3:
          strcpy(mode, "APPEND");
          mode[6] = 0;
          break;
        default:
          strcpy(mode, "ERROR");
          mode[5] = 0;
          break;
      }
      
      printf("%4d %8s %8d     [%d %d]  %d\n", i, mode, oftp->offset, mip->dev, mip->ino, oftp->refCount);
    }
  }
  
  if (count == 0) {
    printf ("  none\n");
  }
  
  return 1;
}

void my_dup(int fd)
{
  if (running->fd[fd] == 0) {
    printf("%d not open\n", fd);
    return;
  }
  
  int i = 0;
  while (running->fd[i] != 0) {
    i++;
  }
  
  running->fd[i] = running->fd[fd];
  running->fd[fd]->refCount++;
}

void my_dup2(int fd, int gd)
{
  if (running->fd[fd] == 0) {
    printf("%d not open\n", fd);
    return;
  }
  
  if (running->fd[gd] != 0) {
    my_close_file(gd);
  }
  
  running->fd[gd] = running->fd[fd];
  running->fd[fd]->refCount++;
}
