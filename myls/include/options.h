

#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>


typedef struct {
   
    bool opt_A;   
    bool opt_a;  
    bool opt_d;   
    bool opt_R;   

    
    bool opt_l;  
    bool opt_n;   
    bool opt_F;   
    bool opt_i;   
    bool opt_s;   
    bool opt_q;   
    bool opt_w;   

   
    bool opt_h;  
    bool opt_k;   

   
    bool opt_f;  
    bool opt_r;   
    bool opt_S;  
    bool opt_t;  

    
    bool opt_c;   
    bool opt_u;  
} Options;


int parse_options(int argc, char *argv[], Options *opts);

#endif 
