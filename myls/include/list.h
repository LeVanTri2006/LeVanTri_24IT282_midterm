

#ifndef LIST_H
#define LIST_H

#include "ls.h"      
#include "options.h" 


void dirlist_init(DirList *dl);

void dirlist_push(DirList *dl, FileEntry *entry);


void free_dirlist(DirList *dl);


int read_dir(const char *dirpath, DirList *dl, const Options *opts);


FileEntry *stat_operand(const char *path, const Options *opts);

#endif 
