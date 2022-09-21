// Derek Wright 04/19/2022
// This file contains the functions for the rmdir command for the EXT2 file system simulator.

#include "functions.h"

int dirempty(MINODE *mip, int ino) {
  char buf[BLKSIZE];
  printf("Checking that dir is empty\n");
  if (mip->INODE.i_links_count > 2) {
    return 0;
  }
  
  get_block(dev, mip->INODE.i_block[0], buf);
  dp = (DIR *)buf;
  char *cp = buf;
  int i = 0;
  while (cp + dp->rec_len <= buf + BLKSIZE) {
    i++;
    cp += dp->rec_len;
    dp = (DIR *)cp;
  }
  if (i > 2) return 0;
  return 1;
}

int rm_child(MINODE *pmip, char *name) {
  // search parent INODE's data block(s) for the entry of name
  // Delete name entry from parent directory by
  //   If (first and only entry in a data block) {
  //      deallocate the data block, reduce parents file size by BLKSIZE,
  //      compact parent's i_block[ ] array to eliminate the deleted entry if its between nonzero entrie's.
  //   Else if (Last entry in block) {
  //      Absorb its rec_len to the preceding entry
  //   Else (entry is first but not the only entry, or in the middle of a block) {
  //      move all trailing entries LEFT to overlay the deleted entry;
  //      add deleted rec_len to the LAST entry; do not change parent's file size;
  //      hint: memcpy(dp, cp, size);
  DIR *dp;
  char buf[BLKSIZE], temp[128], *cp;
  int i;
  for (i = 0; i < 12; i++) {
    if (pmip->INODE.i_block[i] == 0) {
      printf("%s not found\n", name);
      return 0;
    }
    
    get_block(pmip->dev, pmip->INODE.i_block[i], buf);
    dp = (DIR *) buf;
    DIR *pdp = dp;
    cp = buf;
    while (cp + dp->rec_len <= buf + BLKSIZE) {
      strncpy(temp, dp->name, dp->name_len);
      temp[dp->name_len] = 0;
      if (strcmp(temp, name)==0) {
        if (dp->rec_len == BLKSIZE) {
          bdalloc(pmip->dev, pmip->INODE.i_block[i]);
          pmip->INODE.i_size -= BLKSIZE;
          while (i < 11 && pmip->INODE.i_block[i] != 0) {
            pmip->INODE.i_block[i] = pmip->INODE.i_block[i+1];
            i++;
          }
        }
        else if (cp + dp->rec_len >= buf + BLKSIZE) {
          pdp->rec_len += dp->rec_len;
        }
        else {
          int rlen = dp->rec_len;
          int prev_len = dp->rec_len;
          int offset;
          char *cp2;

          while (cp + pdp->rec_len< buf + BLKSIZE) {
            cp2 = cp;
            cp += dp->rec_len;
            
            // need to account for different rec_len when copying memory
            // otherwise a new rec_len 16 will jump over the memory pointed to by
            // a previous rec_len of 12.
	    offset = dp->rec_len - prev_len;
            prev_len = dp->rec_len;
            cp2 += offset;
            dp = (DIR *) cp2;
            
            memcpy(dp, cp, ((DIR *)cp)->rec_len);
            pdp = dp;
            dp = (DIR *)cp;
          }
          pdp->rec_len += rlen;
        }
        put_block(pmip->dev, pmip->INODE.i_block[i], buf);
        return 1;
      }
      pdp = dp;
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
  }
}

int my_rmdir(char *pathname) {
  char buf[BLKSIZE];
  DIR *dp;
  int ino = getino(pathname);
  if (ino == 0) {
    return 0;
  }
  MINODE *mip = iget(dev, ino);
  printf("ref count: %d\n", mip->refCount);
  // Verify that dir is not busy
  printf("Checking that dir is not busy\n");
  if (mip->refCount > 1) {
    printf("ref count: %d\n", mip->refCount);
    printf("DIR %s is busy and cannot be removed\n", pathname);
    iput(mip);
    return 0;
  }
  
  // Verify that DIR is empty
  if (dirempty(mip, ino) == 0) {
    printf("DIR %s is not empty and cannot be removed\n", pathname);
    iput(mip);
    return 0;
  }
  
  printf("Dir is empty\n\n");
  int pino = findino(mip); // Get pino from .. entry in INODE.i_block[0]
  MINODE *pmip = iget(mip->dev, pino);
  char name[128];
  findmyname(pmip, ino, name);
  // Remove child from parent directory
  rm_child(pmip, name);
  // dec parent links_count by 1, mark parent pmip dirty
  pmip->dirty = 1;
  pmip->INODE.i_links_count--;
  iput(pmip);
  bdalloc(mip->dev, mip->INODE.i_block[0]);
  idalloc(mip->dev, mip->ino);
  iput(mip);
}
