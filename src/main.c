#include <iron/full.h>
#include <awsm.h>

typedef wasm_execution_stack stack;
#include "awsmvm.h"
void _sleep(stack * ctx){
  i32 v;
  v = awsm_pop_i32(ctx);
  iron_usleep(v * 1000);
}

/*
void _fopen(stack * ctx){
  char * str = awsm_pop_ptr(ctx);
  u32 result;
  if(strlen(str) > 0){

  }
  }*/


int main(int argc, char ** argv){
  UNUSED(argc);UNUSED(argv);
  wasm_module * mod = awsm_load_module_from_file("kernel.wasm");
  //awsm_log_diagnostic = true;
  awsm_register_function(mod, _sleep, "awsm_sleep");

  malloc_impl(mod, 8);
  malloc_impl(mod, 8);
  void * pt = malloc_impl(mod, 8);
  malloc_impl(mod, 8);
  malloc_impl(mod, 8);
  free_impl(mod, pt);

  void * pt2 = malloc_impl(mod, 8);
  free_impl(mod, pt2);
  void * pt3 = malloc_impl(mod, 64);
  free_impl(mod, pt3);
  register_alloc(mod);
  //return 0;
  if(awsm_load_thread(mod, "kernel") == false){
    printf("Unable to load thread");
    return 1;
  }
  while(awsm_process(mod, 200)){
    
  }
  return 0;
}
