#ifndef STRUCTS_H
#define STRUCTS_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

typedef struct file_data {
        int   mode;
        char  path[100];
        char  data[100];
} file_data;

typedef struct direntry {
	int  set;
	char name[48];
	int  inum;
} dirent;

typedef struct directory {
	char   path[100];
	int    inum;
	dirent ents[10];
} directory;

typedef union data {
	file_data file;
	directory dir;
} data;

typedef struct inode {
	int inum; // inode number
        int refs; // reference count
        int mode; // permission & type
        int size; // bytes for file
        int uid;  // user id
	time_t atim; // time of last access
	time_t mtim; // time of last modification
	time_t ctim; // time of last status change
	union data direct_blocks[10]; // 4k sized data blocks
} inode;

#endif
