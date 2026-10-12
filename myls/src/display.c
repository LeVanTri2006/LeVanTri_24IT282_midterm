#include <errno.h>
#include <limits.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#if defined(__linux__)
#include <sys/sysmacros.h>
#endif

#include "display.h"
#include "util.h"

#define DISPLAY_BUFFER 128

typedef struct {
    const FileEntry *entry;
    char mode[11];
    char inode[DISPLAY_BUFFER];
    char blocks[DISPLAY_BUFFER];
    char nlink[DISPLAY_BUFFER];
    char size[DISPLAY_BUFFER];
    char timestamp[DISPLAY_BUFFER];
    char *owner;
    char *group;
} LongRow;

typedef struct {
    size_t inode;
    size_t blocks;
    size_t nlink;
    size_t owner;
    size_t group;
    size_t size;
} LongWidths;

static int
replace_nonprintable(const Options *opts)
{
    if (opts->opt_w)
        return 0;
    if (opts->opt_q)
        return 1;
    return isatty(STDOUT_FILENO) ? 1 : 0;
}

static const char *
entry_display_name(const FileEntry *entry)
{
    if (entry->is_operand && entry->path[0] != '\0')
        return entry->path;
    return entry->name;
}

static unsigned long long
allocated_bytes(const FileEntry *entry)
{
    unsigned long long blocks;

    if (entry == NULL || entry->st.st_blocks <= 0)
        return 0;

    blocks = (unsigned long long)entry->st.st_blocks;
    if (blocks > ULLONG_MAX / 512ULL)
        return ULLONG_MAX;
    return blocks * 512ULL;
}

static unsigned long long
total_allocated_bytes(FileEntry **entries, size_t count)
{
    unsigned long long total = 0;

    for (size_t i = 0; i < count; i++) {
        unsigned long long value;

        if (entries[i] == NULL)
            continue;
        value = allocated_bytes(entries[i]);
        if (ULLONG_MAX - total < value)
            return ULLONG_MAX;
        total += value;
    }
    return total;
}

static unsigned long long
block_unit(const Options *opts)
{
    const char *value;
    char *end;
    unsigned long long parsed;

    if (opts->opt_k)
        return 1024ULL;

    value = getenv("BLOCKSIZE");
    if (value == NULL || *value == '\0')
        return 512ULL;

    errno = 0;
    end = NULL;
    parsed = strtoull(value, &end, 10);
    if (errno != 0 || end == value || *end != '\0' || parsed == 0)
        return 512ULL;
    return parsed;
}

static unsigned long long
ceil_divide(unsigned long long value, unsigned long long divisor)
{
    return value / divisor + ((value % divisor) != 0 ? 1ULL : 0ULL);
}

static void
format_block_value(unsigned long long bytes, const Options *opts,
                   char *buf, size_t buflen)
{
    if (opts->opt_h) {
        long long human_bytes = bytes > (unsigned long long)LLONG_MAX
                                    ? LLONG_MAX : (long long)bytes;
        format_size_human(human_bytes, buf, buflen);
    } else {
        (void)snprintf(buf, buflen, "%llu",
                       ceil_divide(bytes, block_unit(opts)));
    }
}

static void
format_file_size(const FileEntry *entry, const Options *opts,
                 char *buf, size_t buflen)
{
    if (S_ISCHR(entry->st.st_mode) || S_ISBLK(entry->st.st_mode)) {
#if defined(major) && defined(minor)
        (void)snprintf(buf, buflen, "%llu,%llu",
                       (unsigned long long)major(entry->st.st_rdev),
                       (unsigned long long)minor(entry->st.st_rdev));
#else
        (void)snprintf(buf, buflen, "%llu",
                       (unsigned long long)entry->st.st_rdev);
#endif
        return;
    }

    if (opts->opt_h) {
        unsigned long long value = entry->st.st_size < 0
                                        ? 0ULL
                                        : (unsigned long long)entry->st.st_size;
        long long human_bytes = value > (unsigned long long)LLONG_MAX
                                    ? LLONG_MAX : (long long)value;
        format_size_human(human_bytes, buf, buflen);
    } else if (entry->st.st_size < 0) {
        (void)snprintf(buf, buflen, "?");
    } else if ((uintmax_t)entry->st.st_size > (uintmax_t)LLONG_MAX) {
        (void)snprintf(buf, buflen, "%llu",
                       (unsigned long long)entry->st.st_size);
    } else {
        (void)snprintf(buf, buflen, "%lld", (long long)entry->st.st_size);
    }
}

static char *
owner_name(uid_t uid, int numeric)
{
    char buf[DISPLAY_BUFFER];
    struct passwd *pw;

    if (!numeric) {
        pw = getpwuid(uid);
        if (pw != NULL && pw->pw_name != NULL)
            return xstrdup(pw->pw_name);
    }

    (void)snprintf(buf, sizeof(buf), "%llu", (unsigned long long)uid);
    return xstrdup(buf);
}

static char *
group_name(gid_t gid, int numeric)
{
    char buf[DISPLAY_BUFFER];
    struct group *gr;

    if (!numeric) {
        gr = getgrgid(gid);
        if (gr != NULL && gr->gr_name != NULL)
            return xstrdup(gr->gr_name);
    }

    (void)snprintf(buf, sizeof(buf), "%llu", (unsigned long long)gid);
    return xstrdup(buf);
}

static time_t
display_time(const FileEntry *entry, const Options *opts)
{
    if (opts->opt_c)
        return entry->st.st_ctime;
    if (opts->opt_u)
        return entry->st.st_atime;
    return entry->st.st_mtime;
}

static void
format_timestamp(time_t value, char *buf, size_t buflen)
{
    struct tm local_tm;
    time_t now;
    double age;
    const char *format;

    if (localtime_r(&value, &local_tm) == NULL) {
        (void)snprintf(buf, buflen, "?");
        return;
    }

    now = time(NULL);
    age = difftime(now, value);
    if (age >= 0.0 && age < (double)(6 * 30 * 24 * 60 * 60))
        format = "%b %e %H:%M";
    else
        format = "%b %e  %Y";

    if (strftime(buf, buflen, format, &local_tm) == 0)
        (void)snprintf(buf, buflen, "?");
}

static size_t
max_size(size_t current, size_t candidate)
{
    return candidate > current ? candidate : current;
}

static void
display_total(FileEntry **entries, size_t count, const Options *opts)
{
    char total[DISPLAY_BUFFER];

    if (opts->opt_h) {
        unsigned long long bytes = 0;

        for (size_t i = 0; i < count; i++) {
            unsigned long long value;

            if (entries[i] == NULL || entries[i]->st.st_size <= 0)
                continue;

            value = (unsigned long long)entries[i]->st.st_size;
            if (ULLONG_MAX - bytes < value) {
                bytes = ULLONG_MAX;
                break;
            }
            bytes += value;
        }

        long long human_bytes =
            bytes > (unsigned long long)LLONG_MAX
                ? LLONG_MAX : (long long)bytes;

        format_size_human(human_bytes, total, sizeof(total));
    } else {
        format_block_value(total_allocated_bytes(entries, count), opts,
                           total, sizeof(total));
    }

    printf("total %s\n", total);
}

void display_list(FileEntry **entries, size_t count,
                  const Options *opts, const char *label,
                  bool directory_listing)
{
    int replace_nonprint;

    if (opts == NULL)
        return;

    replace_nonprint = replace_nonprintable(opts);
    if (label != NULL) {
        printable_name(label, replace_nonprint);
        putchar(':');
        putchar('\n');
    }

    if (directory_listing &&
        ((opts->opt_l || opts->opt_n) ||
         (opts->opt_s && isatty(STDOUT_FILENO))))
        display_total(entries, count, opts);

    if (opts->opt_l || opts->opt_n)
        display_long(entries, count, opts);
    else
        display_short(entries, count, opts);
}

void display_short(FileEntry **entries, size_t count, const Options *opts)
{
    int replace_nonprint;

    if (entries == NULL || opts == NULL)
        return;
    replace_nonprint = replace_nonprintable(opts);

    for (size_t i = 0; i < count; i++) {
        FileEntry *entry = entries[i];
        char blocks[DISPLAY_BUFFER];

        if (entry == NULL)
            continue;
        if (opts->opt_i)
            printf("%llu ", (unsigned long long)entry->st.st_ino);
        if (opts->opt_s) {
            format_block_value(allocated_bytes(entry), opts,
                               blocks, sizeof(blocks));
            printf("%s ", blocks);
        }

        printable_name(entry_display_name(entry), replace_nonprint);
        if (opts->opt_F) {
            char indicator = type_indicator(entry->st.st_mode);
            if (indicator != '\0')
                putchar(indicator);
        }
        putchar('\n');
    }
}

void display_long(FileEntry **entries, size_t count, const Options *opts)
{
    LongRow *rows;
    LongWidths widths = { 1, 1, 1, 1, 1, 1 };
    size_t row_count = 0;
    int replace_nonprint;

    if (entries == NULL || opts == NULL || count == 0)
        return;

    rows = xmalloc(count * sizeof(*rows));
    memset(rows, 0, count * sizeof(*rows));
    replace_nonprint = replace_nonprintable(opts);

    for (size_t i = 0; i < count; i++) {
        LongRow *row;
        const FileEntry *entry = entries[i];

        if (entry == NULL)
            continue;
        row = &rows[row_count++];
        row->entry = entry;
        format_mode(entry->st.st_mode, row->mode);
        (void)snprintf(row->inode, sizeof(row->inode), "%llu",
                       (unsigned long long)entry->st.st_ino);
        format_block_value(allocated_bytes(entry), opts,
                           row->blocks, sizeof(row->blocks));
        (void)snprintf(row->nlink, sizeof(row->nlink), "%llu",
                       (unsigned long long)entry->st.st_nlink);
        format_file_size(entry, opts, row->size, sizeof(row->size));
        format_timestamp(display_time(entry, opts), row->timestamp,
                         sizeof(row->timestamp));
        row->owner = owner_name(entry->st.st_uid, opts->opt_n);
        row->group = group_name(entry->st.st_gid, opts->opt_n);

        widths.inode = max_size(widths.inode, strlen(row->inode));
        widths.blocks = max_size(widths.blocks, strlen(row->blocks));
        widths.nlink = max_size(widths.nlink, strlen(row->nlink));
        widths.owner = max_size(widths.owner, strlen(row->owner));
        widths.group = max_size(widths.group, strlen(row->group));
        widths.size = max_size(widths.size, strlen(row->size));
    }

    for (size_t i = 0; i < row_count; i++) {
        LongRow *row = &rows[i];
        char indicator;

        if (opts->opt_i)
            printf("%*s ", (int)widths.inode, row->inode);
        if (opts->opt_s)
            printf("%*s ", (int)widths.blocks, row->blocks);
        printf("%s %*s %-*s %-*s %*s %s ",
               row->mode,
               (int)widths.nlink, row->nlink,
               (int)widths.owner, row->owner,
               (int)widths.group, row->group,
               (int)widths.size, row->size,
               row->timestamp);

        printable_name(entry_display_name(row->entry), replace_nonprint);
        if (opts->opt_F) {
            indicator = type_indicator(row->entry->st.st_mode);
            if (indicator != '\0')
                putchar(indicator);
        }
        if (row->entry->link_target != NULL) {
            fputs(" -> ", stdout);
            printable_name(row->entry->link_target, replace_nonprint);
        }
        putchar('\n');
    }

    for (size_t i = 0; i < row_count; i++) {
        free(rows[i].owner);
        free(rows[i].group);
    }
    free(rows);
}
