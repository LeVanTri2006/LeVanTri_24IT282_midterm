

#ifndef SORT_H
#define SORT_H

#include "ls.h"
#include "options.h"


void sort_entries(FileEntry **entries, size_t count, const Options *opts);


int cmp_name(const void *a, const void *b);


int cmp_size(const void *a, const void *b);


int cmp_mtime(const void *a, const void *b);


int cmp_ctime(const void *a, const void *b);


int cmp_atime(const void *a, const void *b);

#endif 
