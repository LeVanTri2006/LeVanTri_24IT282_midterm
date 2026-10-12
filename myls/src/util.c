

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>

#include "util.h"


#ifndef S_ISVTX
#define S_ISVTX 01000
#endif

void *xmalloc(size_t size)
{
    void *p = malloc(size);
    if (p == NULL) {
        fprintf(stderr, "myls: out of memory\n");
        exit(1);
    }
    return p;
}

void *xrealloc(void *ptr, size_t size)
{
    void *p = realloc(ptr, size);
    if (p == NULL) {
        fprintf(stderr, "myls: out of memory\n");
        exit(1);
    }
    return p;
}

char *xstrdup(const char *s)
{
    char *p = strdup(s);
    if (p == NULL) {
        fprintf(stderr, "myls: out of memory\n");
        exit(1);
    }
    return p;
}


void format_mode(mode_t mode, char buf[11])
{
    if (buf == NULL)
        return;

    char type = '-';

#if defined(S_ARCH2)
    if ((mode & S_ARCH2) != 0)
        type = 'A';
    else
#endif
#if defined(S_ARCH1)
    if ((mode & S_ARCH1) != 0)
        type = 'a';
    else
#endif
#if defined(S_ISWHT)
    if (S_ISWHT(mode))
        type = 'w';
    else
#endif
    if (S_ISREG(mode))
        type = '-';
    else if (S_ISDIR(mode))
        type = 'd';
    else if (S_ISLNK(mode))
        type = 'l';
    else if (S_ISBLK(mode))
        type = 'b';
    else if (S_ISCHR(mode))
        type = 'c';
    else if (S_ISFIFO(mode))
        type = 'p';
    else if (S_ISSOCK(mode))
        type = 's';

    buf[0] = type;
    buf[1] = (mode & S_IRUSR) ? 'r' : '-';
    buf[2] = (mode & S_IWUSR) ? 'w' : '-';
    buf[3] = (mode & S_IXUSR) ? 'x' : ((mode & S_ISUID) ? 'S' : '-');
    buf[4] = (mode & S_IRGRP) ? 'r' : '-';
    buf[5] = (mode & S_IWGRP) ? 'w' : '-';
    buf[6] = (mode & S_IXGRP) ? 'x' : ((mode & S_ISGID) ? 'S' : '-');
    buf[7] = (mode & S_IROTH) ? 'r' : '-';
    buf[8] = (mode & S_IWOTH) ? 'w' : '-';
    buf[9] = (mode & S_IXOTH) ? 'x' : ((mode & S_ISVTX) ? 'T' : '-');

    if ((mode & S_ISUID) && (mode & S_IXUSR))
        buf[3] = 's';
    if ((mode & S_ISGID) && (mode & S_IXGRP))
        buf[6] = 's';
    if ((mode & S_ISVTX) && (mode & S_IXOTH))
        buf[9] = 't';

    buf[10] = '\0';
}


void format_size_human(long long bytes, char *buf, size_t buflen)
{
    static const char units[] = "BKMGTPE";
    unsigned long long value;
    double scaled;
    size_t unit = 0;

    if (buf == NULL || buflen == 0)
        return;
    if (bytes < 0) {
        (void)snprintf(buf, buflen, "?");
        return;
    }

    value = (unsigned long long)bytes;
    scaled = (double)value;
    while (scaled >= 1024.0 && unit + 1 < sizeof(units) - 1) {
        scaled /= 1024.0;
        unit++;
    }

    if (unit == 0)
        (void)snprintf(buf, buflen, "%lluB", value);
    else if (scaled < 10.0)
        (void)snprintf(buf, buflen, "%.1f%c", scaled, units[unit]);
    else
        (void)snprintf(buf, buflen, "%.0f%c", scaled, units[unit]);
}

char type_indicator(mode_t mode)
{
#if defined(S_ISWHT)
    if (S_ISWHT(mode))
        return '%';
#endif
    if (S_ISDIR(mode))
        return '/';
    if (S_ISLNK(mode))
        return '@';
    if (S_ISFIFO(mode))
        return '|';
    if (S_ISSOCK(mode))
        return '=';
    if (mode & (S_IXUSR | S_IXGRP | S_IXOTH))
        return '*';
    return '\0';
}


void printable_name(const char *name, int replace_nonprint)
{
    if (name == NULL)
        return;

    for (const unsigned char *p = (const unsigned char *)name; *p != '\0'; p++) {
        if (replace_nonprint && !isprint(*p))
            putchar('?');
        else
            putchar(*p);
    }
}
