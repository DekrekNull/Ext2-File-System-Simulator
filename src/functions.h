#ifndef FUNCTIONS
#define FUNCTIONS

// C Libs:
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>


// USR
#include "type.h"
#include "globals.h"


/* util.c */
int get_block(int dev, int blk, char *buf);
int put_block(int dev, int blk, char *buf);
int tokenize(char *pathname);
MINODE *iget(int dev, int ino);
void iput(MINODE *mip);
int search(MINODE *mip, char *name);
int getino(char *pathname);
int findmyname(MINODE *parent, u32 myino, char myname[]);
int findino(MINODE *mip);

/* cd_ls_pwd.c */
int my_cd(char *pathname);
int ls_file(MINODE *mip, char *name);
int ls_dir(MINODE *mip);
int my_ls(char *pathname);
void my_pwd(MINODE *wd);

/* alloc_balloc.c */
int ialloc(int dev);
int balloc(int dev);
int idalloc(int dev, int ino);
int bdalloc(int dev, int bno);

/* mkdir_creat.c */
int create_inode(MINODE *mip, int blk, int mode, int links, int size);
int create_block(MINODE *pmip, int ino, int blk);
int get_cwd_string(MINODE *wd, char *cwd);
char *getdirname(char *pathname);
char *getbasename(char *pathname);
int enter_child(MINODE * pmip, int ino, char *name);
int my_mkdir(char *pathname);
int my_creat(char *pathname);

/* rmdir.c */
int dirempty(MINODE *mip, int ino);
int rm_child(MINODE *pmip, char *name);
int my_rmdir(char *pathname);

/* link_unlink.c */
int my_link(char *old_file, char *new_file);
int my_unlink(char *pathname);

/* symlink.c */
int my_symlink(char *old_file, char *new_file);
int my_readlink(char *pathname, char *buf);

/* open_close_lseek.c */
int my_truncate(MINODE *mip);
int my_open_file(char *pathname, int mode);
int my_close_file(int fd);
int my_lseek(int fd, int position);
int pfd();
void my_dup(int fd);
void my_dup2(int fd, int gd);

/* read_cat.c */
int my_read_file(int fd, int nbytes);
int my_read(int fd, char buf[], int nbytes);
void my_cat(char *filename);

/* write_cp.c */
int my_write(int fd, char buf[], int nbytes);
void my_cp(char *src, char *dest);
void my_mv(char *src, char *dest);

/* mount_unmount.c */
MOUNT *getmptr(int dev);
int display_current_mount();
int my_mount(char *filesys, char *mount_point);
int my_unmount(char *filesys);

#endif
