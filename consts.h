#ifndef CONSTS_H
#define CONSTS_H

#include <limits.h>
#include <stdint.h>
#include <stdio.h>

#include "structs.h"

typedef uint32_t word_t;
enum { BITS_PER_WORD = sizeof(word_t) * CHAR_BIT };
#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b)  ((b) % BITS_PER_WORD)

static int NUFS_SIZE     = 1024 * 1024; // 1MB
static int PAGE_COUNT    = 256;
static int NUM_OF_INODES = 25;

// const int FREE_INODE_OFFSET = 0;
static int FREE_BLOCK_OFFSET = sizeof(word_t);
static int INODE_LIST_OFFSET = sizeof(word_t) * 2;
static int DATA_LIST_OFFSET  = sizeof(word_t) * 2 + sizeof(inode) * 32;

#endif
