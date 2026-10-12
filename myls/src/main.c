

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>   
#include <unistd.h>    

#include "ls.h"
#include "options.h"
#include "list.h"
#include "sort.h"
#include "display.h"


static int g_exit_status = 0;


static void ls_error(const char *path)
{
    if (errno != 0)
        fprintf(stderr, "myls: %s: %s\n", path, strerror(errno));
    else
        fprintf(stderr, "myls: %s: cannot access\n", path);
    g_exit_status = 1;
}


static int operand_is_dir(const char *path, const Options *opts)
{
    struct stat st;
    int ret;

    if (opts->opt_d)
        ret = lstat(path, &st);
    else
        ret = stat(path, &st);

    if (ret == -1)
        return -1;

    return S_ISDIR(st.st_mode) ? 1 : 0;
}


static int process_dir(const char *path, const Options *opts,
                       const char *label)
{
    DirList dl;
    dirlist_init(&dl);

    int r = read_dir(path, &dl, opts);
    if (r == -1) {
        
        ls_error(path);
        free_dirlist(&dl);
        return -1;
    }

    if (r == 1) {
        
        g_exit_status = 1;
    }

    sort_entries(dl.entries, dl.count, opts);
    display_list(dl.entries, dl.count, opts, label, true);

    
    if (opts->opt_R) {
        for (size_t i = 0; i < dl.count; i++) {
            FileEntry *fe = dl.entries[i];

            if (!S_ISDIR(fe->st.st_mode))
                continue;
            if (strcmp(fe->name, ".") == 0 || strcmp(fe->name, "..") == 0)
                continue;

            printf("\n");
            process_dir(fe->path, opts, fe->path);
        }
    }

    free_dirlist(&dl);
    return r;
}


static void process_files(char **paths, int count, const Options *opts)
{
    DirList dl;
    dirlist_init(&dl);

    for (int i = 0; i < count; i++) {
        errno = 0; 
        FileEntry *fe = stat_operand(paths[i], opts);
        if (fe == NULL) {
            ls_error(paths[i]);
            continue;
        }
        dirlist_push(&dl, fe);
    }

    if (dl.count > 0) {
        
        display_list(dl.entries, dl.count, opts, NULL, false);
    }

    free_dirlist(&dl);
}



int main(int argc, char *argv[])
{
    
    Options opts;
    memset(&opts, 0, sizeof(opts));

    int first_arg = parse_options(argc, argv, &opts);
   
    int noperands = argc - first_arg;
    char **operands;

    
    static char *default_operands[] = { "." };
    if (noperands == 0) {
        operands  = default_operands;
        noperands = 1;
    } else {
        operands = argv + first_arg;
    }

    
    int *file_idx = malloc((size_t)noperands * sizeof(int));
    int *dir_idx  = malloc((size_t)noperands * sizeof(int));
    if (file_idx == NULL || dir_idx == NULL) {
        fprintf(stderr, "myls: out of memory\n");
        free(file_idx);
        free(dir_idx);
        return 1;
    }
    int nfiles = 0, ndirs = 0;

    for (int i = 0; i < noperands; i++) {
        errno = 0;

        if (opts.opt_d) {
           
            file_idx[nfiles++] = i;
        } else {
            int isdir = operand_is_dir(operands[i], &opts);
            if (isdir == -1) {
                
                ls_error(operands[i]);
            } else if (isdir == 1) {
                dir_idx[ndirs++] = i;
            } else {
                file_idx[nfiles++] = i;
            }
        }
    }

    
    if (!opts.opt_f) {
        for (int i = 1; i < nfiles; i++) {
            int value = file_idx[i];
            int j = i;
            while (j > 0 && strcmp(operands[file_idx[j - 1]],
                                    operands[value]) > 0) {
                file_idx[j] = file_idx[j - 1];
                j--;
            }
            file_idx[j] = value;
        }

        for (int i = 1; i < ndirs; i++) {
            int value = dir_idx[i];
            int j = i;
            while (j > 0 && strcmp(operands[dir_idx[j - 1]],
                                    operands[value]) > 0) {
                dir_idx[j] = dir_idx[j - 1];
                j--;
            }
            dir_idx[j] = value;
        }
    }

    
    int show_label = (noperands > 1) ? 1 : 0;

  
    if (nfiles > 0) {
       
        char **file_paths = malloc((size_t)nfiles * sizeof(char *));
        if (file_paths == NULL) {
            fprintf(stderr, "myls: out of memory\n");
            free(file_idx);
            free(dir_idx);
            return 1;
        }
        for (int i = 0; i < nfiles; i++)
            file_paths[i] = operands[file_idx[i]];

        process_files(file_paths, nfiles, &opts);
        free(file_paths);
    }

    
    int need_blank = (nfiles > 0) ? 1 : 0;

    for (int i = 0; i < ndirs; i++) {
        if (need_blank)
            printf("\n");
        need_blank = 1; 

        const char *dpath = operands[dir_idx[i]];
        const char *label = show_label ? dpath : NULL;

        errno = 0;
        process_dir(dpath, &opts, label);
    }

    
    free(file_idx);
    free(dir_idx);

    return g_exit_status;
}
