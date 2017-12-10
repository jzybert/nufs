#ifndef PAGES_H
#define PAGES_H

#include <limits.h>
#include <stdint.h>
#include <stdio.h>

#include "structs.h"
#include "consts.h"

void   pages_init(const char* path);
void   pages_free();

int    get_inode_bit(int inum);
int    get_dir_bit(int inum);
void   pages_clear_inode_bit(int inum);
void   pages_clear_dir_bit(int inum);
void   pages_print_bits();

data*   pages_get_page(int inum);
inode* pages_get_node(int node_id);
int    pages_find_empty();

void   print_node(inode* node);

int    pages_create_file(const char* path, mode_t mode);
int    pages_delete_file(const char* path);

int    pages_set_time(const char* path, const struct timespec ts[2]);
inode* get_inode_from_path(const char* path);

int    pages_write_data(const char* path, const char *buf, size_t size, off_t offset);
int    pages_truncate(const char* path, size_t size);

#endif
