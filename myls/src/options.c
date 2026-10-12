
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   
#include <string.h>

#include "options.h"


int parse_options(int argc, char *argv[], Options *opts)
{
    
    memset(opts, 0, sizeof(*opts));

    int ch;
    while ((ch = getopt(argc, argv, "AacdFfhiklnqRrSstuw")) != -1) {
        switch (ch) {

       
        case 'A': opts->opt_A = true; break;
        case 'a': opts->opt_a = true; break;
        case 'F': opts->opt_F = true; break;
        case 'f': opts->opt_f = true; break;
        case 'i': opts->opt_i = true; break;
        case 'r': opts->opt_r = true; break;
        case 'S': opts->opt_S = true; break;
        case 's': opts->opt_s = true; break;
        case 't': opts->opt_t = true; break;

       
        case 'h':
            opts->opt_h = true;
            opts->opt_k = false;
            break;
        case 'k':
            opts->opt_k = true;
            opts->opt_h = false;
            break;

        
        case 'l':
            opts->opt_l = true;
            opts->opt_n = false;
            break;
        case 'n':
            opts->opt_n = true;
            opts->opt_l = false;
            break;

        
        case 'q':
            opts->opt_q = true;
            opts->opt_w = false;
            break;
        case 'w':
            opts->opt_w = true;
            opts->opt_q = false;
            break;

        
        case 'c':
            opts->opt_c = true;
            opts->opt_u = false;
            break;
        case 'u':
            opts->opt_u = true;
            opts->opt_c = false;
            break;

        
        case 'R':
            opts->opt_R = true;
            opts->opt_d = false;
            break;
        case 'd':
            opts->opt_d = true;
            opts->opt_R = false;
            break;

        
        default:
            
            fprintf(stderr,
                    "usage: myls [-AacdFfhiklnqRrSstuw] [file ...]\n");
            exit(1);
        }
    }

    
    return optind;
}
