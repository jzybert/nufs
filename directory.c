#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "util.h"
#include "directory.h"
#include "pages.h"
#include "structs.h"

int
directory_empty(directory* dir)
{
	for (int ii = 0; ii < 10; ++ii) {
		if (dir->ents[ii].set) {
			return 0;
		}
	}
	return 1;
}

directory*
directory_from_inum(int inum)
{
	inode* node = pages_get_node(inum);
	return (directory*)node->direct_blocks;
}

directory*
directory_from_path(const char* path)
{
	inode* node = get_inode_from_path(path);
	if (node != 0) {
		return (directory*)node->direct_blocks;
	} else {
		return 0;
	}
}

int 
directory_put_ent(directory* dd, const char* name, int inum)
{
	int entryAdded = 0;
	for (int ii = 0; ii < 10; ++ii) {
		if (!dd->ents[ii].set) {
			strncpy(dd->ents[ii].name, name, strlen(name) + 1);
			dd->ents[ii].inum = inum;
			entryAdded = dd->ents[ii].set = 1;
			break;
		}
	}
	if (!entryAdded) {
		return -1;
	}
	return 0;
}

int
directory_delete(directory* dd)
{
	if (directory_empty(dd)) {
		pages_clear_inode_bit(dd->inum);
		pages_clear_dir_bit(dd->inum);
		pages_print_bits();
		return 0;
	}
	return -1;
}

void
print_directory(directory* dd)
{
	if (dd) {
		printf("directory{path: %s, inum: %i}\n", dd->path, dd->inum);
	} else {
		printf("directory{null}");
	}
}

int
directory_move(const char* from, const char* to)
{
	inode* node = get_inode_from_path(from);
	if (node == 0) {
		return -1;
	}
	int inum = node->inum;
	mode_t mode = node->mode;
	if (S_ISREG(mode)) {
		directory_remove_path(from, inum);
		strcpy(node->direct_blocks[0].file.path, to);
		directory_add_path(to, inum);
	}
	return 0;
}

void
directory_add_path(const char* path, int inum)
{
	// Get parent path and file name
	int numOfSlashes = 0;
	for (int ii = 0; ii < strlen(path); ++ii) {
		if (path[ii] == '/') {
			numOfSlashes++;
		}
	}
	int numOfCharsBefore = 0;
	int numOfCharsAfter = 0;
	int numOfSlashesBefore = 0;
	for (int ii = 0; ii < strlen(path); ++ii) {
		if (path[ii] == '/') {
			numOfSlashesBefore++;
		}
		if (numOfSlashesBefore == numOfSlashes) {
			numOfCharsAfter++;
		} else {
			numOfCharsBefore++;
		}
	}
	char parentPath[numOfCharsBefore + 1];
	char fileName[numOfCharsAfter];
	strncpy(fileName, &path[numOfCharsBefore + 1], numOfCharsAfter);
	if (numOfSlashes == 1) {
		parentPath[0] = '/';
		parentPath[1] = '\0';
	} else {
		strncpy(parentPath, path, numOfCharsBefore);
		parentPath[numOfCharsBefore + 1] = '\0';
	}
	// Find the corresponding directory
	directory* dir = 0;
	for (int ii = 0; ii < NUM_OF_INODES; ++ii) {
		if (get_inode_bit(ii)) {
			inode* node = pages_get_node(ii);
			if (S_ISDIR(node->mode)) {
				directory* direct = (directory*)node->direct_blocks;
				if (streq(parentPath, direct->path)) {
					dir = direct;
				}
			}
		}
	}

	if (dir != 0) {
		directory_put_ent(dir, fileName, inum);
	}
}

void
directory_remove_path(const char* path, int inum)
{
	int numOfSlashes = 0;
        for (int ii = 0; ii < strlen(path); ++ii) {
                if (path[ii] == '/') {
                        numOfSlashes++;
                }
        }
        int numOfCharsBefore = 0;
        int numOfCharsAfter = 0;
        int numOfSlashesBefore = 0;
        for (int ii = 0; ii < strlen(path); ++ii) {
                if (path[ii] == '/') {
                        numOfSlashesBefore++;
                }
                if (numOfSlashesBefore == numOfSlashes) {
                        numOfCharsAfter++;
                } else {
                        numOfCharsBefore++;
                }
        }
        char parentPath[numOfCharsBefore + 1];
        char fileName[numOfCharsAfter];
        strncpy(fileName, &path[numOfCharsBefore + 1], numOfCharsAfter);
        if (numOfSlashes == 1) {
                parentPath[0] = '/';
                parentPath[1] = '\0';
        } else {
                strncpy(parentPath, path, numOfCharsBefore);
                parentPath[numOfCharsBefore + 1] = '\0';
        }
        // Find the corresponding directory
        directory* dir = 0;
        for (int ii = 0; ii < NUM_OF_INODES; ++ii) {
                if (get_inode_bit(ii)) {
                        inode* node = pages_get_node(ii);
                        if (S_ISDIR(node->mode)) {
                                directory* direct = (directory*)node->direct_blocks;
                                if (streq(parentPath, direct->path)) {
                                        dir = direct;
                                }
                        }
		}
	}
	if (dir != 0) {
		for (int ii = 0; ii < 10; ++ii) {
			if (dir->ents[ii].set) {
				if (streq(fileName, dir->ents[ii].name)) {
					dir->ents[ii].set = 0;
				}
			}
		}
	}
}
