#ifndef DIRECTORY_H
#define DIRECTORY_H

#define DIR_SIZE 64
#define DIR_NAME 48

#include "pages.h"
#include "structs.h"

directory* directory_from_inum(int inum);
directory* directory_from_path(const char* path);

int directory_put_ent(directory* dd, const char* name, int inum);
int directory_delete(directory* dd);

void print_directory(directory* dd);

int directory_move(const char* from, const char* to);

void directory_add_path(const char* path, int inum);
void directory_remove_path(const char* path, int inum);
#endif

