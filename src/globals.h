#ifndef GLOBALS
#define GLOBALS

#include <ext2fs/ext2_fs.h>
#include "type.h"

extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;
extern MOUNT mountTable[8];
extern OFT    oft[64];

extern char gpath[128]; // global for tokenized components
extern char *name[64];  // assume at most 64 components in pathname
extern int   n;         // number of component strings
extern int b_zero[256];

extern int dev;
extern int nblocks, ninodes, bmap, imap, iblk;

extern SUPER *sp;
extern GD    *gp;
extern INODE *ip;
extern DIR   *dp;

#endif
