// Derek Wright 04/19/2022
// This file contains the functions for the mkdir and creat commands for the EXT2 file system simulator

#include "functions.h"

int create_inode(MINODE *mip, int blk, int mode, int links, int size) {
  INODE *ip = &mip->INODE;
  ip->i_mode = mode; // type and permissions
  ip->i_uid  = running->uid; // owner id
  ip->i_gid  = running->gid; // group id
  ip->i_size = size;      // size in bits
  ip->i_links_count = links;     // . and ..
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
  ip->i_block[0] = blk;
  for (int i = 1; i <= 14; i++) {
    ip->i_block[i] = 0;
  } 
  mip->dirty = 1;
  iput(mip);
}

int create_block(MINODE *pmip, int ino, int blk) {
  // make data block 0 of INODE contain . and ..
  // Write to disk block blk
  char buf[BLKSIZE];
  bzero(buf, BLKSIZE);
  DIR *dp = (DIR *)buf;
  // make . entry
  dp->inode = ino;
  dp->rec_len = 12;
  dp->name_len = 1;
  dp->name[0] = '.';
  // make .. entry: pino=parent DIR ino, blk = allockated block
  dp = (DIR *)((char *)dp + 12);
  dp->inode = pmip->ino;
  dp->rec_len = BLKSIZE-12;
  dp->name_len = 2;
  dp->name[0] = dp->name[1] = '.';
  put_block(dev, blk, buf); 
}

int get_cwd_string(MINODE *wd, char *cwd)
{
  if (wd == root) {
    char rt[256];
    rt[0] = '\0';
    strcat(rt, "/");
    if (strlen(cwd) > 0) {
      strncat(rt, cwd, strlen(cwd));
    }
    strncpy(cwd, rt, strlen(rt));
    cwd[strlen(rt)] = 0;
    return 1;
  }
  int pino = findino(wd);
  MINODE *pip = iget(dev, pino);
  char my_name[256];
  findmyname(pip, wd->ino, my_name);
  get_cwd_string(pip, cwd);
  iput(pip);
  strncat(cwd, my_name, strlen(my_name));
  return 1;
}

char *getdirname(char *pathname) {
  char *name = (char *)malloc(128);
  name[0] = 0;
  char *temp = strrchr(pathname, '/');
  if (temp == NULL) {
    get_cwd_string(running->cwd, name);
  }
  else {
    strncpy(name, pathname, strlen(pathname) - strlen(temp));
    name[strlen(pathname) - strlen(temp)] = 0;
  }
  
  return name;
}

char *getbasename(char *pathname) {
  char *name = strchr(pathname, '/');
  if (name != NULL) {
    name++;
    return name;
  }
  
  name = (char *)malloc(128);
  memcpy(name, pathname, strlen(pathname));
  name[strlen(pathname)] = 0;
  return name;
}

// Enters a new dir_entry (ino, name) into the parent directory pmip
int enter_child(MINODE * pmip, int ino, char *name) {
  INODE *pip = &(pmip->INODE);
  char buf[BLKSIZE], *cp;
  DIR *dp;
  int i, ideal_length, remain;
  int need_length = 4 * ( (8 + strlen(name) + 3) / 4);
  for (i = 0; i < 12; i++) {
    if (pip->i_block[i] == 0) break;
    
    get_block(pmip->dev, pip->i_block[i], buf);
    dp = (DIR *) buf;
    cp = buf;
    while (cp + dp->rec_len < buf + BLKSIZE) {
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
    // dp now points at last entry in the block
    ideal_length = 4 * ( (8 + dp->name_len + 3) / 4);
    remain = dp->rec_len - ideal_length;
    if (remain >= need_length) {
      dp->rec_len = ideal_length;
      cp += dp->rec_len;
      dp = (DIR *) cp;
      dp->inode = ino;
      dp->rec_len = remain;
      dp->name_len = strlen(name);
      strcpy(dp->name, name);
      dp->name[dp->name_len] = 0;
      put_block(pmip->dev, pip->i_block[i], buf);
      return 1;
    }
  }
  pip->i_block[i] = balloc(pmip->dev);
  get_block(pmip->dev, pip->i_block[i], buf);
  pip->i_size += BLKSIZE;
  dp = (DIR *) buf;
  dp->inode = ino;
  dp->rec_len = BLKSIZE;
  dp->name_len = strlen(name);
  strcpy(dp->name, name);
  put_block(pmip->dev, pip->i_block[i], buf);
  return 1;
}

int kmkdir(MINODE *pmip, char *basename) {
  //Allocate an inode and disk block
  int ino = ialloc(dev);
  int blk = balloc(dev);
  
  // Create and inode in a minode and write the inode to the disk
  MINODE *mip = iget(dev, ino);
  create_inode(mip, blk, 0x41ED, 2, BLKSIZE);
  create_block(pmip, ino, blk);
  // enter (ino, basename)as a dir_entry to the parent INODE pmip
  enter_child(pmip, ino, basename);
  
  return 1;
}

int my_mkdir(char *pathname) {
  int pino;
  MINODE *pmip;
  INODE *pin;
  
  char *basename = getbasename(pathname); 
  char *dirname = getdirname(pathname);
  
  printf("dirname=%s basename=%s\n", dirname, basename);
  pino = getino(dirname);
  pmip = iget(dev, pino);
  pin = &(pmip->INODE);
  
  // Check pmip->INODE is a DIR;
  if ((pin->i_mode & 0xF000) != 0x4000) {
    printf("%s is not a DIR\n", dirname);
    iput(pmip);
    return 0;
  }
  
  // Check that basename does not already exist in dirname
  if (search(pmip, basename) != 0) {
    printf("DIR %s already exists in DIR %s\n", basename, dirname);
    iput(pmip);
    return 0;
  }
  
  printf("Making DIR %s in DIR %s...\n", basename, dirname);
  // Make the dir basename in the dir dirname
  kmkdir(pmip, basename);
  pin->i_links_count++;
  pmip->dirty = 1;
  iput(pmip);
  printf("DIR %s made successfully\n", basename);
  return 1;
}

int kcreat(MINODE *pmip, char *basename) {
  //Allocate an inode and disk block
  int ino = ialloc(dev);
  
  // Create an inode in a minode and write the inode to the disk
  MINODE *mip = iget(dev, ino);
  create_inode(mip, 0, 0x81A4, 1, 0);
  // enter (ino, basename)as a dir_entry to the parent INODE pmip
  enter_child(pmip, ino, basename);
  
  return 1;
}

int my_creat(char *pathname) {
  int pino;
  MINODE *pmip;
  INODE *pin;
  
  char *basename = getbasename(pathname); 
  char *dirname = getdirname(pathname);
  
  printf("dirname=%s filename=%s\n", dirname, basename);
  pino = getino(dirname);
  pmip = iget(dev, pino);
  pin = &(pmip->INODE);
  
  // Check pmip->INODE is a DIR;
  if ((pin->i_mode & 0xF000) != 0x4000) {
    printf("%s is not a DIR\n", dirname);
    iput(pmip);
    return 0;
  }
  
  // Check that basename does not already exist in dirname
  if (search(pmip, basename) != 0) {
    printf("FILE %s already exists in DIR %s\n", basename, dirname);
    iput(pmip);
    return 0;
  }
  
  printf("Making FILE %s in DIR %s...\n", basename, dirname);
  // Make the dir basename in the dir dirname
  kcreat(pmip, basename);
  pmip->dirty = 1;
  iput(pmip);
  printf("FILE %s made successfully\n", basename);
  return 1;
}
