#include <stdlib.h>
#include <string.h>

#include "sort.h"



static const FileEntry *
entry_from_qsort_arg(const void *arg)
{
    if (arg == NULL)
        return NULL;
    return *(const FileEntry * const *)arg;
}

static int
compare_names(const FileEntry *a, const FileEntry *b)
{
    if (a == b)
        return 0;
    if (a == NULL)
        return -1;
    if (b == NULL)
        return 1;

    return strcoll(a->name, b->name);
}

static int
compare_size_desc(off_t a, off_t b)
{
    if (a > b)
        return -1;
    if (a < b)
        return 1;
    return 0;
}

static int
compare_time_desc(time_t a, time_t b)
{
    if (a > b)
        return -1;
    if (a < b)
        return 1;
    return 0;
}

static int
compare_time_entries(const void *a, const void *b, time_t (*get_time)(const FileEntry *))
{
    const FileEntry *entry_a = entry_from_qsort_arg(a);
    const FileEntry *entry_b = entry_from_qsort_arg(b);

    if (entry_a == NULL || entry_b == NULL) {
        if (entry_a == entry_b)
            return 0;
        return entry_a == NULL ? -1 : 1;
    }

    int result = compare_time_desc(get_time(entry_a), get_time(entry_b));
    return result != 0 ? result : compare_names(entry_a, entry_b);
}

static time_t
get_mtime(const FileEntry *entry)
{
    return entry->st.st_mtime;
}

static time_t
get_ctime(const FileEntry *entry)
{
    return entry->st.st_ctime;
}

static time_t
get_atime(const FileEntry *entry)
{
    return entry->st.st_atime;
}


int cmp_name(const void *a, const void *b)
{
    return compare_names(entry_from_qsort_arg(a), entry_from_qsort_arg(b));
}


int cmp_size(const void *a, const void *b)
{
    const FileEntry *entry_a = entry_from_qsort_arg(a);
    const FileEntry *entry_b = entry_from_qsort_arg(b);

    if (entry_a == NULL || entry_b == NULL) {
        if (entry_a == entry_b)
            return 0;
        return entry_a == NULL ? -1 : 1;
    }

    int result = compare_size_desc(entry_a->st.st_size, entry_b->st.st_size);
    return result != 0 ? result : compare_names(entry_a, entry_b);
}


int cmp_mtime(const void *a, const void *b)
{
    return compare_time_entries(a, b, get_mtime);
}


int cmp_ctime(const void *a, const void *b)
{
    return compare_time_entries(a, b, get_ctime);
}

int cmp_atime(const void *a, const void *b)
{
    return compare_time_entries(a, b, get_atime);
}


void sort_entries(FileEntry **entries, size_t count, const Options *opts)
{
    int (*comparator)(const void *, const void *) = cmp_name;

    if (entries == NULL || count < 2 || opts == NULL || opts->opt_f)
        return;

    
    if (opts->opt_S) {
        comparator = cmp_size;
    } else if (opts->opt_t) {
        if (opts->opt_c)
            comparator = cmp_ctime;
        else if (opts->opt_u)
            comparator = cmp_atime;
        else
            comparator = cmp_mtime;
    }

    qsort(entries, count, sizeof(*entries), comparator);

    if (opts->opt_r) {
        for (size_t i = 0; i < count / 2; i++) {
            FileEntry *tmp = entries[i];
            entries[i] = entries[count - 1 - i];
            entries[count - 1 - i] = tmp;
        }
    }
}
