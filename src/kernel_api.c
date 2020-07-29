#include "awsm_api.h"

void __attribute__ ((noinline)) awsm_sleep(int ms){

}

void __attribute__ ((noinline)) print_str(const char * str){

}

void __attribute__ ((noinline)) print_i32(int v){

}


void * __attribute__ ((noinline)) malloc(unsigned long size){ return 0; }
void __attribute__ ((noinline)) free(void * ptr){ }

file * __attribute__ ((noinline)) fopen(const char * file, const char * perm){ return NULL;}
void __attribute__ ((noinline)) fclose(file * f){}



size_t fread ( void * ptr, size_t size, size_t count, FILE * stream ){return 0;}
size_t fwrite ( const void * ptr, size_t size, size_t count, FILE * stream ){return 0;}
