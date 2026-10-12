

#ifndef UTIL_H
#define UTIL_H

#include <sys/stat.h>   
#include <stddef.h>    


void format_mode(mode_t mode, char buf[11]);

void format_size_human(long long bytes, char *buf, size_t buflen);


char type_indicator(mode_t mode);


void printable_name(const char *name, int replace_nonprint);

void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);

char *xstrdup(const char *s);

#endif 
