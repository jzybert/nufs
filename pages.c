
#define _GNU_SOURCE
#include <string.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

#include "pages.h"
#include "directory.h"
#include "util.h"

static int   pages_fd   = -1;
static void* pages_base =  0;

// Bitmap code attributed to Dale Hagglund @
// https://stackoverflow.com/questions/1225998/what-is-a-bitmap-in-c
void
set_bit(word_t *words, int n)
{
	words[WORD_OFFSET(n)] |= (1 << BIT_OFFSET(n));
}

void
clear_bit(word_t *words, int n)
{
	words[WORD_OFFSET(n)] &= ~(1 << BIT_OFFSET(n));
}

int
get_bit(word_t *words, int n)
{
	word_t bit = words[WORD_OFFSET(n)] & (1 << BIT_OFFSET(n));
	return bit != 0;
}
// End attribution

void
pages_init(const char* path)
{
	// Open file path
	pages_fd = open(path, O_CREAT | O_RDWR, 0644);
	assert(pages_fd != -1);
	// Truncate file to 1MB
	int rv = ftruncate(pages_fd, NUFS_SIZE);
	assert(rv == 0);
	// mmap 1MB of memory
	pages_base = mmap(0, NUFS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, pages_fd, 0);
	assert(pages_base != MAP_FAILED);
	int alreadyCreated = get_bit((word_t*)pages_base, 0);
	if (!alreadyCreated) {
		// set the bits in the inode and free data block bitmaps to 0
		for (int ii = 0; ii < NUM_OF_INODES; ++ii) {
			clear_bit(((word_t*)pages_base), ii);
			clear_bit((word_t*)(((char*)pages_base) + FREE_BLOCK_OFFSET), ii);
		}
		pages_create_file("/", 040755);
	}
}

int
get_inode_bit(int inum)
{
	return get_bit((word_t*)pages_base, inum);
}

int
get_dir_bit(int inum)
{
	return get_bit((word_t*)((char*)pages_base + FREE_BLOCK_OFFSET), inum);
}

void
pages_clear_inode_bit(int inum)
{
	clear_bit((word_t*)pages_base, inum);
}

void
pages_clear_dir_bit(int inum)
{
	clear_bit((word_t*)((char*)pages_base + FREE_BLOCK_OFFSET), inum);
}

void
pages_print_bits()
{
	for (int ii = 0; ii < NUM_OF_INODES; ++ii) {
		int bit = get_inode_bit(ii);
		printf("%i", bit);
	}
	printf("\n");
	for (int ii = 0; ii < NUM_OF_INODES; ++ii) {
		int bit = get_dir_bit(ii);
		printf("%i", bit);
	}
	printf("\n");
}

void
pages_free()
{
	int rv = munmap(pages_base, NUFS_SIZE);
	assert(rv == 0);
}

data*
pages_get_page(int inum)
{
    	return (data*)((((char*)pages_base) + DATA_LIST_OFFSET) + (4096 * 10 * inum));
	
}

inode*
pages_get_node(int node_id)
{
    	return (inode*)(((char*)pages_base) + INODE_LIST_OFFSET) + (sizeof(inode*) * node_id);
}

int
pages_find_empty()
{
	int inum = -1;
	for (int ii = 0; ii < NUM_OF_INODES; ++ii) {
		int bit = get_bit((word_t*)pages_base, ii);
		if (bit == 0) { // if page is empty
			inum = ii;
			break;
		}
	}
	return inum;
}


int
pages_create_file(const char *path, mode_t mode)
{
	if (strlen(path) > 100) {
		return -ENAMETOOLONG;
	}
	inode* exists = get_inode_from_path(path);
	if (exists != 0) {
		return -EEXIST;
	}
	// Find a "off" inode and turn it "on"
	int inum = pages_find_empty();
	if (inum == -1) {
		return -ENOSPC;
	}
	set_bit((word_t*)pages_base, inum);
	set_bit((word_t*)((char*)pages_base + FREE_BLOCK_OFFSET), inum);
	// Set the inode's mode, refs, uid, size, and direct block mem
	inode* node = pages_get_node(inum);
	node->inum = inum;
	node->mode = mode;
	node->refs = 1;
	node->uid = getuid();
	data* mem = pages_get_page(inum);
	for (int ii = 0; ii < 10; ++ii) {
		data* d = (data*)(((char*)mem) + (4096 * ii));
                node->direct_blocks[ii] = (*d);
        }
	if (S_ISREG(mode)) { // regular file
		// Read the path contents and store in file data at direct block mem
		int fd = open(path, O_RDONLY);
		char buffer[4096];
		read(fd, buffer, 0);
		strncpy(node->direct_blocks[0].file.path, path, strlen(path) + 1);
		node->direct_blocks[0].file.mode = mode;
		strncpy(node->direct_blocks[0].file.data, buffer, strlen(path) + 1);
		node->size = strlen(node->direct_blocks[0].file.data);
	} else { // directory
		strncpy(node->direct_blocks[0].dir.path, path, strlen(path) + 1);
		node->direct_blocks[0].dir.inum = inum;
		for (int ii = 0; ii < 10; ++ii) {
			node->direct_blocks[0].dir.ents[ii].set = 0;
		}
		node->size = 0;
	}
	// add file to parent directory pointer
	if (path != "/") {
		directory_add_path(path, inum);
	}
	return 0;
}

int
pages_delete_file(const char* path)
{
	if (strlen(path) > 100) {
		return -ENAMETOOLONG;
	}
	inode* node = get_inode_from_path(path);
	if (S_ISDIR(node->mode)) {
		return -EISDIR;
	}
	int inum = node->inum;
	pages_clear_inode_bit(inum);
	pages_clear_dir_bit(inum);
	directory_remove_path(path, inum);
	return 0;
}

void
print_node(inode* node)
{
	if (node) {
		if (S_ISREG(node->mode)) {
			file_data* data = (file_data*)node->direct_blocks;
			printf("node{inum: %d, refs: %d, mode: %04o, size: %d, path: %s}\n", 
			node->inum, node->refs, node->mode, node->size, data->path);
		}
	}
	else {
		printf("node{null}\n");
	}
}

inode*
get_inode_from_path(const char* path)
{
	for (int ii = 0; ii < NUM_OF_INODES; ++ii) {
		if (get_bit((word_t*)pages_base, ii)) {
			inode* node = pages_get_node(ii);
			if (S_ISREG(node->mode)) {
				file_data data = node->direct_blocks[0].file;
				if (streq(path, data.path)) {
					return node;
				}
			} else {
				directory dir = node->direct_blocks[0].dir;
				if (streq(path, dir.path)) {
					return node;
				}
			}
		}
	}
	return 0;
}

int
pages_set_time(const char* path, const struct timespec ts[2])
{
	inode* node = get_inode_from_path(path);
	if (node == 0) {
		return -1;
	}
	node->atim = ts[0].tv_sec;
	node->mtim = ts[0].tv_nsec;
	node->ctim = ts[1].tv_sec;
	return 0;
}

int
pages_write_data(const char* path, const char* buf, size_t size, off_t offset)
{
	inode* node = get_inode_from_path(path);
	file_data* dat = (file_data*)node->direct_blocks;
	strcpy(dat->data + offset, buf);
	node->size = strlen(dat->data);
	return size;
}

int
pages_truncate(const char* path, size_t size)
{
	inode* node = get_inode_from_path(path);
	node->size = size;
	return 0;
}
