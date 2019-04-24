#include <stdlib.h>
#include<stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <limits.h>
#include <string.h>

#define stat xv6_stat  // avoid clash with host struct stat
#define dirent xv6_dirent  // avoid clash with host struct stat
#include "types.h"
#include "fs.h"
#include "stat.h"
#undef stat
#undef dirent

// SXY: helper method for req5
void check_bitmap (uint block_num, void *img_ptr) {
    uchar *bm_ptr = BSIZE*28 +img_ptr;  // calculate which bit represents this data block 
    int line = block_num/8;
    int offset = block_num%8;
    char tester = 1;
    printf("block_num: %d\n", block_num);
    if ((*(bm_ptr+line) & (tester << offset)) == 0) { // TODO: check *(bm_ptr + line)
        fprintf(stderr, "ERROR: address used by inode but marked free in bitmap\n");
        //printf()
        exit(1);
    }
}

int main(int argc, char *argv[]) {
  int fd;
  if (argc != 2) {
    fprintf(stderr, "Usage: xcheck <file_system_image>\n");
    exit(1);
  }
  fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    fprintf(stderr, "image not found\n");
    exit(1);
  }

  struct stat sbuf;
  fstat(fd, &sbuf);
  void *img_ptr = mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (img_ptr == (void *)-1) {
    fprintf(stderr, "mmap failed\n");
    exit(1);
  }
  
  // How to access superblock:
  // struct superblock *sb = (struct superblock *)(img_ptr + BSIZE);

  // inode number for / is 1, 0 is unused, and How to access an inode:
  // struct dinode *dip = (struct dinode *)(img_ptr + 2 * BSIZE);
  // dip[0], dip[1] ... dip[7]

  // How to access a data block for directory:
  // uint data_block_addr = dip[1].addr[0];
  // struct xv6_dirent *entry = (struct xv6_dirent *)(img_ptr + data_block_addr * BSIZE);
  // entry[0].name;
  // entry[0].inum;

  // How to access a data block for data:
  // char* readme_ptr = (char*)(img_ptr + BSIZE * dip[2].addrs[0]);
  

  struct superblock *sb = (struct superblock *)(img_ptr + BSIZE);
  int ninodes = sb->ninodes;
  int nblocks = sb->nblocks;
//  int size = sb->size;
  struct dinode *dip = (struct dinode *)(img_ptr + 2 * BSIZE);

//  uchar *bm_ptr = img_ptr + 28*BSIZE;

  /* SXY
  
   req1: 
   Each inode is either unallocated or one of the valid types (T_FILE, T_DIR, T_DEV). If not, print ERROR: bad inode.
   
   req2: 
   For in-use inodes, each address that is used by inode is valid (points to a valid datablock address within the image). 
   If the direct block is used and is invalid, print ERROR: bad direct address in inode.; 
   if the indirect block is in use and is invalid, print ERROR: bad indirect address in inode.
  
   req5: 
   For in-use inodes, each address in use is also marked in use in the bitmap. If not, print ERROR: address used by inode but marked free in bitmap.
  */
  for (int i = 0; i < ninodes; i++) {
        if(!(dip[i].type == 0 || dip[i].type == T_FILE || dip[i].type == T_DIR ||dip[i].type == T_DEV)){
           fprintf(stderr,"ERROR: bad inode\n");  // SXY: TODO: need to check stderr
           exit(1);
        }
        if(dip[i].type == 0 ){
           continue;
        }
        // check direct data blocks
        for(int j = 0; j < NDIRECT; j ++) {
            if(dip[i].addrs[j] == 0) {
               continue;
            }
            else if(dip[i].addrs[j] < 29 || dip[i].addrs[j] > nblocks) {  //SXY: TODO: need to check "29" and also ">="
               fprintf(stderr, "ERROR: bad direct address in inode\n");
               exit(1);
            }
            else {  // check bitmap
                check_bitmap (dip[i].addrs[j], img_ptr);
            }
           // SXY: also adding incremetatio of number of accesses for every data block here?
        }
        //check indirect data block
        if(dip[i].addrs[NDIRECT] == 0) {
           continue;
        }
        else {
            if(dip[i].addrs[NDIRECT] < 29 || dip[i].addrs[NDIRECT] > nblocks) {  //SXY: TODO: need to check "29" and also ">="
              fprintf(stderr, "ERROR: bad direct address in inode\n");
              exit(1);
            }
            check_bitmap ((uint)dip[i].addrs[NDIRECT], img_ptr);
            for (int k = 0; k < BSIZE/sizeof(int*); k++) {
                if(dip[i].addrs[NDIRECT]+k == 0) {
                    continue;
                }
                if(dip[i].addrs[NDIRECT]+k < 29 || dip[i].addrs[NDIRECT]+k > nblocks) {  //SXY: TODO: need to check "29" and also ">="
                    fprintf(stderr, "ERROR: bad indirect address in inode\n");
                    exit(1);
                }
                check_bitmap(dip[i].addrs[NDIRECT]+k, img_ptr);
            }
        }
  }
   
  // req3:
  // Root directory exists, its inode number is 1, and the parent of the root directory is itself.
  // If not, print ERROR: root directory does not exist.
  if (dip[1].type != 1) {
      //printf("1\n");
      fprintf(stderr,"ERROR: root directory does not exist.\n");
      exit(1);
  }
  
  // assumption: ".." is the second entry of a directory inode
  struct xv6_dirent *entry;
  int isfound = 0;
  for (int i = 0; i < NDIRECT; i++) {
      if (dip[1].addrs[i] == 0) {
          continue;
      }
      entry = (struct xv6_dirent *)(img_ptr + dip[1].addrs[i] * BSIZE);
      for (int j = 0; j < BSIZE / sizeof(struct xv6_dirent); j++) {
          if (entry[j].inum == 0) {
              continue;
          }
          if (strcmp(entry[j].name, "..") == 0) {
              if (entry[j].inum != 1) {
                  //printf("2\n");
                  fprintf(stderr,"ERROR: root directory does not exist.\n");
                  exit(1);
              }
              isfound = 1;
              break;
          }
      }
      if (isfound) {
          break;
      }
  }
  if (!isfound) {
      uint *indirect_entry = (uint *)(img_ptr + dip[1].addrs[NDIRECT] * BSIZE);
      for (int i = 0; i < BSIZE / sizeof(uint); i++) {
          if (indirect_entry[i] == 0) {
              continue;
          }
          entry = (struct xv6_dirent *)(img_ptr + indirect_entry[i] * BSIZE);
          for (int j = 0; j < BSIZE / sizeof(struct xv6_dirent); j++) {
              if (entry[j].inum == 0) {
                  continue;
              }
              if (strcmp(entry[j].name, "..") == 0) {
                  if (entry[j].inum != 1) {
                      //printf("3\n");
                      fprintf(stderr,"ERROR: root directory does not exist.\n");
                      exit(1);
                  }
                  isfound = 1;
                  break;
              }
          }
          if (isfound) {
              break;
          }
      }
  }
  if (!isfound) {
      //printf("4\n");
      fprintf(stderr,"ERROR: root directory does not exist.\n");
      exit(1);
  }
 
  // req4:
  // Each directory contains . and .. entries, and the . entry points to the directory itself. 
  // If not, print ERROR: directory not properly formatted.
  for (int i = 0; i < ninodes; i++) {
      if (dip[i].type != 1) {
          continue;
      }
      int isfoundself = 0;
      int isfoundparent = 0;
      struct xv6_dirent *entry;
      for (int j = 0; j < NDIRECT; j++) {
          if (dip[i].addrs[j] == 0) {
              continue;
          }
          entry = (struct xv6_dirent *)(img_ptr + dip[i].addrs[j] * BSIZE);
          for (int k = 0; k < BSIZE / sizeof(struct xv6_dirent); k++) {
              if (entry[k].inum == 0) {
                  continue;
              }
              if (strcmp(entry[k].name, "..") == 0) {
                  isfoundparent = 1;
              }
              if (strcmp(entry[k].name, ".") == 0) {
                  if (entry[k].inum != i) {
                      fprintf(stderr,"ERROR: directory not properly formatted.\n");
                      exit(1);
                  }
                  isfoundself = 1;
              }
              if (isfoundparent & isfoundself) {
                  break;
              }
          }
          if (isfoundparent & isfoundself) {
              break;
          }
      }
      if (isfoundparent & isfoundself) {
          continue;
      }
      uint* indirect_entry = (uint*)(img_ptr + dip[i].addrs[NDIRECT] * BSIZE);
      for (int j = 0; j < BSIZE / sizeof(uint); j++) {
          if (indirect_entry[j] == 0) {
              continue;
          }
          entry = (struct xv6_dirent *)(img_ptr + indirect_entry[i] * BSIZE);
          for (int k = 0; k < BSIZE / sizeof(struct xv6_dirent); k++) {
              if (entry[k].inum == 0) {
                  continue;
              }
              if (strcmp(entry[k].name, "..") == 0) {
                  isfoundparent = 1;
              }
              if (strcmp(entry[k].name, ".") == 0) {
                  if (entry[k].inum != i) {
                      fprintf(stderr,"ERROR: directory not properly formatted.\n");
                      exit(1);
                  }
                  isfoundself = 1;
              }
              if (isfoundparent & isfoundself) {
                  break;
              }
          }
          if (isfoundparent & isfoundself) {
              break;
          }
      }
      if (!isfoundself || !isfoundparent) {
          fprintf(stderr,"ERROR: directory not properly formatted.\n");
          exit(1);
      }
      
  }
  
  
  // req6:
  // For blocks marked in-use in bitmap, the block should actually be in-use in an inode or indirect block somewhere. 
  // If not, print ERROR: bitmap marks block in use but it is not in use.
  exit(0);
}