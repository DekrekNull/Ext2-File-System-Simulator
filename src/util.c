/*********** util.c file ****************/
#ifndef UTIL
#define UTIL
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "type.h"
#include "functions.h"
#include "globals.h"

int get_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   read(dev, buf, BLKSIZE);
}   

int put_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   write(dev, buf, BLKSIZE);
}   

int tokenize(char *pathname)
{
  int i;
  char *s;
  printf("tokenize %s\n", pathname);

  strcpy(gpath, pathname);   // tokens are in global gpath[ ]
  n = 0;

  s = strtok(gpath, "/");
  while(s){
    name[n] = s;
    n++;
    s = strtok(0, "/");
  }
  name[n] = 0;
  
  for (i= 0; i<n; i++)
    printf("%s  ", name[i]);
  printf("\n");
}

// return minode pointer to loaded INODE
MINODE *iget(int dev, int ino)
{
  int i;
  MINODE *mip;
  char buf[BLKSIZE];
  int blk, offset;
  INODE *ip;
  MOUNT *mnt = getmptr(dev);

  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount && mip->dev == dev && mip->ino == ino){
       mip->refCount++;
       printf("found [%d %d] as minode[%d] in core; mounted=%d\n", dev, ino, i, mip->mounted);
       return mip;
    }
  }
    
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount == 0){
       //printf("allocating NEW minode[%d] for [%d %d]\n", i, dev, ino);
       mip->refCount = 1;
       mip->dev = dev;
       mip->ino = ino;

       // get INODE of ino to buf    
       blk    = (ino-1)/8 + mnt->iblk;
       offset = (ino-1) % 8;

       //printf("iget: ino=%d blk=%d offset=%d\n", ino, blk, offset);

       get_block(dev, blk, buf);
       ip = (INODE *)buf + offset;
       // copy INODE to mp->INODE
       mip->INODE = *ip;
       return mip;
    }
  }   
  printf("PANIC: no more free minodes\n");
  return 0;
}

int midalloc(MINODE *mip)
{
  mip->refCount = 0;
}

void iput(MINODE *mip)
{
  int i, block, offset;
  char buf[BLKSIZE];
  INODE *ip;
  MOUNT *mnt = getmptr(dev);
  
  if (mip==0) {
    return;
  }

  mip->refCount--;
 
  if (mip->refCount > 0) return;
  if (mip->dirty == 0)   return;
 
  // Write the INODE back to disk
  block = (mip->ino - 1) / 8 + mnt->iblk;
  offset = (mip->ino - 1) % 8;
 
  // Get the block containing this INODE
  get_block(mip->dev, block, buf);
  ip = (INODE *)buf + offset;
  *ip = mip->INODE;
  put_block(mip->dev, block, buf);
  midalloc(mip);
} 

int search(MINODE *mip, char *name)
{
   int i; 
   char *cp, c, sbuf[BLKSIZE], temp[256];
   DIR *dp;
   INODE *ip;

   printf("search for %s in MINODE = [%d, %d]\n", name,mip->dev,mip->ino);
   ip = &(mip->INODE);

   /*** search for name in mip's data blocks: ASSUME i_block[0] ONLY ***/

   get_block(dev, ip->i_block[0], sbuf);
   dp = (DIR *)sbuf;
   cp = sbuf;
   printf("  ino   rlen  nlen  name\n");

   while (cp < sbuf + BLKSIZE){
     strncpy(temp, dp->name, dp->name_len);
     temp[dp->name_len] = 0;
     printf("%4d  %4d  %4d    %s\n", 
           dp->inode, dp->rec_len, dp->name_len, dp->name);
     if (strcmp(temp, name)==0){
        printf("found %s : ino = %d\n", temp, dp->inode);
        return dp->inode;
     }
     cp += dp->rec_len;
     dp = (DIR *)cp;
   }
   return 0;
}

int getino(char *pathname)
{
  int i, ino, blk, offset;
  char buf[BLKSIZE];
  INODE *ip;
  MINODE *mip;

  printf("getino: pathname=%s\n", pathname);
  if (strcmp(pathname, "/")==0)
      return 2;
  
  // starting mip = root OR CWD
  if (pathname[0]=='/')
     mip = root;
  else
     mip = running->cwd;
  mip->refCount++;         // because we iput(mip) later
  
  tokenize(pathname);

  for (i=0; i<n; i++){
      printf("===========================================\n");
      printf("getino: i=%d name[%d]=%s\n", i, i, name[i]);
      
      if (mip->mounted  == 0) {
        ino = search(mip, name[i]);
        printf("ino=%d dev=%d root->dev=%d mounted=%d\n", ino, mip->dev, root->dev,  mip->mounted);
        if (ino==0){
          iput(mip);
          printf("name %s does not exist\n", name[i]);
          return 0;
        }
        else if (ino==2 && mip->dev != root->dev) {
          printf("root of mounted\n");
          MOUNT *mntptr = getmptr(mip->dev);
          iput(mip);
          mip = mntptr->mounted_inode;
          dev = mntptr->dev;
        }
        
        iput(mip);
        mip = iget(dev, ino);
      }
      else {
        MOUNT *mntptr = mip->mptr;
        iput(mip);
        mip = iget(mntptr->dev, 2);
        printf("[%d %d] is mounted on; cross mounting point newDev=%d\n", dev, ino, mntptr->dev);
        dev = mntptr->dev;        
      }
   }
   if (mip->mounted == 1) {
     MOUNT *mntptr = mip->mptr;
     iput(mip);
     mip = iget(mntptr->dev, 2);
     printf("[%d %d] is mounted on; cross mounting point newDev=%d\n", dev, ino, mntptr->dev);
     dev = mntptr->dev; 
   }
   iput(mip);
   return ino;
}

// These 2 functions are needed for pwd()
int findmyname(MINODE *parent, u32 myino, char myname[ ]) 
{
  // WRITE YOUR code here
  // search parent's data block for myino; SAME as search() but by myino
  // copy its name STRING to myname[ ]
  char pbuf[BLKSIZE];
  get_block(dev, parent->INODE.i_block[0], pbuf);
  DIR *pd = (DIR *) pbuf;
  char *pc = pbuf;
  while (pc < pbuf + BLKSIZE){
    if (myino == pd->inode) {
      strncpy(myname, pd->name, pd->name_len);
      myname[pd->name_len] = 0;
      return 1;
    }
    pc += pd->rec_len;
    pd = (DIR *)pc;
  }
  return -1;
}

int findino(MINODE *mip) // myino = i# of . return i# of ..
{
  char pbuf[BLKSIZE], temp[256];
  get_block(dev, mip->INODE.i_block[0], pbuf);
  DIR *pd = (DIR *) pbuf;
  char *pc = pbuf;
  while (pc < pbuf + BLKSIZE){
    strncpy(temp, pd->name, pd->name_len);
    temp[pd->name_len] = 0;
    if (strcmp("..", temp) == 0) {
      return pd->inode;
    }
    pc += pd->rec_len;
    pd = (DIR *)pc;
  }
  return 0;
}

#endif
