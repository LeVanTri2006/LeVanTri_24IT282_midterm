

#ifndef LS_H
#define LS_H

#include <sys/stat.h>   
#include <sys/types.h>  
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>     


#ifndef PATH_MAX
#define PATH_MAX 1024
#endif


typedef struct {
    char        name[PATH_MAX]; 
    char        path[PATH_MAX];  
    struct stat st;              
    char       *link_target;     
    bool        is_operand;      
} FileEntry;


typedef struct {
    FileEntry **entries;  
    size_t      count;   
    size_t      capacity; 
} DirList;


#define DIRLIST_INIT_CAP 64

#endif 
