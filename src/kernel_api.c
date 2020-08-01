#include "awsm_api.h"

void NOINLINE awsm_sleep(int ms){}
void NOINLINE print_str(const char * str){ }
void NOINLINE print_i32(int v){ }
void NOINLINE yield(){ }
void NOINLINE suspend_machine(){ }
void NOINLINE suspend(){
  suspend_machine();
  yield();
}


void * NOINLINE malloc(unsigned long size){ return 0; }
void NOINLINE free(void * ptr){ }

file * NOINLINE fopen(const char * file, const char * perm){ return NULL;}
void NOINLINE fclose(file * f){}



size_t NOINLINE fread ( void * ptr, size_t size, size_t count, FILE * stream ){return 0;}
size_t NOINLINE fwrite ( const void * ptr, size_t size, size_t count, FILE * stream ){return 0;}
