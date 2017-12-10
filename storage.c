#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#include "directory.h"
#include "storage.h"
#include "structs.h"

void
storage_init(const char* path)
{
	pages_init(path);
}

static int
streq(const char* aa, const char* bb)
{
	return strcmp(aa, bb) == 0;
}

static file_data*
get_file_data(const char* path)
{
	inode* node = get_inode_from_path(path);
	if (node != 0 && S_ISREG(node->mode)) {
		return &node->direct_blocks[0].file;
	}
	return 0;
}

static directory*
get_dir_data(const char* path)
{
	inode* node = get_inode_from_path(path);
	if (node != 0 && S_ISDIR(node->mode)) {
		return &node->direct_blocks[0].dir;
	}
	return 0;
}

int
get_stat(const char* path, struct stat* st)
{
	file_data* dat = get_file_data(path);
	if (!dat) {
		directory* dir = get_dir_data(path);
		if (!dir) {
			return -1;
		}
		memset(st, 0, sizeof(struct stat));
		st->st_uid = getuid();
		st->st_mode = 040755;
		st->st_size = 0;
	} else {
		memset(st, 0, sizeof(struct stat));
		st->st_uid  = getuid();
		st->st_mode = dat->mode;
		if (dat->data) {
			st->st_size = strlen(dat->data);
		} else {
			st->st_size = 0;
		}
		st->st_blocks = 4;
		st->st_blksize = 512;
	}
	return 0;
}

const char*
get_data(const char* path)
{
	file_data* dat = get_file_data(path);
	if (!dat) {
		return 0;
	}

	return dat->data;
}
