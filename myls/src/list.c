

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>    
#include <sys/stat.h>   
#include <unistd.h>     
#include <errno.h>
#include <limits.h>    

#include "ls.h"
#include "options.h"
#include "list.h"
#include "util.h"


void dirlist_init(DirList *dl)
{
    dl->entries  = NULL;
    dl->count    = 0;
    dl->capacity = 0;
}

void dirlist_push(DirList *dl, FileEntry *entry)
{
    if (dl->count == dl->capacity) {
        size_t new_cap = (dl->capacity == 0) ? DIRLIST_INIT_CAP
                                             : dl->capacity * 2;
        dl->entries = xrealloc(dl->entries,
                               new_cap * sizeof(FileEntry *));
        dl->capacity = new_cap;
    }
    dl->entries[dl->count++] = entry;
}

void free_dirlist(DirList *dl)
{
    for (size_t i = 0; i < dl->count; i++) {
        free(dl->entries[i]->link_target); 
        free(dl->entries[i]);
    }
    free(dl->entries);
    dirlist_init(dl); 
}


static FileEntry *make_entry(void)
{
    FileEntry *fe = xmalloc(sizeof(FileEntry));
    memset(fe, 0, sizeof(*fe));
    fe->link_target = NULL;
    return fe;
}


static void resolve_link_target(FileEntry *fe)
{
    
    char buf[PATH_MAX + 1];
    ssize_t len = readlink(fe->path, buf, PATH_MAX);
    if (len == -1)
        return; 

    buf[len] = '\0';
    fe->link_target = xstrdup(buf);
}


static int should_show(const char *name, const Options *opts)
{
   
    int effective_A = opts->opt_A || (getuid() == 0);

    if (opts->opt_a)
        return 1; 

    if (name[0] != '.')
        return 1; 

   
    if (effective_A) {
       
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
            return 0;
        return 1;
    }

    
    return 0;
}



int read_dir(const char *dirpath, DirList *dl, const Options *opts)
{
    DIR *dirp = opendir(dirpath);
    if (dirp == NULL)
        return -1; 

   
    int need_link_target = (opts->opt_l || opts->opt_n);

    struct dirent *dp;
    int ret = 0;

    errno = 0; 

    while ((dp = readdir(dirp)) != NULL) {
        const char *dname = dp->d_name;

       
        if (!should_show(dname, opts)) {
            errno = 0;
            continue;
        }

       
        char full_path[PATH_MAX];
        int n;
        
        if (strcmp(dirpath, "/") == 0)
            n = snprintf(full_path, sizeof(full_path), "/%s", dname);
        else
            n = snprintf(full_path, sizeof(full_path), "%s/%s",
                         dirpath, dname);

        if (n < 0 || (size_t)n >= sizeof(full_path)) {
           
            fprintf(stderr, "myls: %s/%s: %s\n", dirpath, dname,
                    strerror(ENAMETOOLONG));
            ret = 1;       
            errno = 0;
            continue;
        }

      
        FileEntry *fe = make_entry();

        
        strncpy(fe->name, dname, PATH_MAX - 1);
        fe->name[PATH_MAX - 1] = '\0';
        strncpy(fe->path, full_path, PATH_MAX - 1);
        fe->path[PATH_MAX - 1] = '\0';

       
        if (lstat(full_path, &fe->st) == -1) {
            fprintf(stderr, "myls: %s: %s\n", full_path, strerror(errno));
            free(fe); 
            ret = 1;       
            errno = 0;
            continue;
        }

       
        if (need_link_target && S_ISLNK(fe->st.st_mode))
            resolve_link_target(fe);

        dirlist_push(dl, fe);
        errno = 0; 
    }

    
    if (errno != 0) {
        int saved = errno;
        closedir(dirp);
        errno = saved;
        return -1;
    }

    if (closedir(dirp) == -1)
        return -1; 

   
    return ret;
}



FileEntry *stat_operand(const char *path, const Options *opts)
{
    FileEntry *fe = make_entry();
    fe->is_operand = true;

    
    const char *base = strrchr(path, '/');
    base = (base != NULL) ? base + 1 : path;
    
    if (*base == '\0')
        base = path;

    strncpy(fe->name, base, PATH_MAX - 1);
    fe->name[PATH_MAX - 1] = '\0';
    strncpy(fe->path, path, PATH_MAX - 1);
    fe->path[PATH_MAX - 1] = '\0';

  
    if (lstat(path, &fe->st) == -1) {
        int saved = errno;
        free(fe);
        errno = saved;
        return NULL;
    }

   
    int need_link_target = (opts->opt_l || opts->opt_n);
    if (need_link_target && S_ISLNK(fe->st.st_mode))
        resolve_link_target(fe);

    return fe;
}
