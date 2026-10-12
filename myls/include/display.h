

#ifndef DISPLAY_H
#define DISPLAY_H

#include "ls.h"
#include "options.h"


void display_list(FileEntry **entries, size_t count,
                  const Options *opts, const char *label,
                  bool directory_listing);


void display_short(FileEntry **entries, size_t count, const Options *opts);


void display_long(FileEntry **entries, size_t count, const Options *opts);

#endif 
