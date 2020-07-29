#include <iron/full.h>
#include <awsm.h>

typedef wasm_execution_stack stack;
#include "awsmvm.h"

void _sleep(stack * ctx){
  i32 v;
  v = awsm_pop_i32(ctx);
  iron_usleep(v * 1000);
}

void awsm_push_ptr(stack * ctx, void * ptr){
  wasm_module * mod = awsm_stack_module(ctx);
  void * heap = awsm_module_heap_ptr(mod);
  u32 offset = ptr - heap;
  awsm_push_u32(ctx, offset);

}

void _fopen(stack * ctx){
  char * perm = awsm_pop_ptr(ctx);
  char * str = awsm_pop_ptr(ctx);

  char buf[100];
  if(str[0] == '/'){
    sprintf(buf, "system%s", str);
  }else{
    sprintf(buf, "system/%s", str);
  }
  FILE * f = fopen(buf, perm);
  if(f != NULL){

    FILE ** fptr = malloc_impl(awsm_stack_module(ctx), sizeof(f));
    *fptr = f;
    awsm_push_ptr(ctx, fptr);
    return;
  }
  awsm_push_i32(ctx, 0);  
}

void _fclose(stack * ctx){
  FILE ** f = awsm_pop_ptr(ctx);
  fclose(*f);
  free_impl(awsm_stack_module(ctx), f);
}

void _fread(stack * ctx){
  FILE ** f = awsm_pop_ptr(ctx);
  u32 count = awsm_pop_u32(ctx);
  u32 size = awsm_pop_u32(ctx);
  void * buffer = awsm_pop_ptr(ctx);

  size_t read = fread(buffer,size,count, *f);
  awsm_push_u32(ctx, (u32)read);
}


void _fwrite(stack * ctx){
  FILE ** f = awsm_pop_ptr(ctx);
  u32 count = awsm_pop_u32(ctx);
  u32 size = awsm_pop_u32(ctx);
  void * buffer = awsm_pop_ptr(ctx);

  size_t read = fwrite(buffer,size,count, *f);
  awsm_push_u32(ctx, (u32)read);
}

int main(int argc, char ** argv){
  UNUSED(argc);UNUSED(argv);
  wasm_module * mod = awsm_load_module_from_file("kernel.wasm");
  //awsm_log_diagnostic = true;
  awsm_register_function(mod, _sleep, "awsm_sleep");
  awsm_register_function(mod, _fopen, "fopen");
  awsm_register_function(mod, _fclose, "fclose");
  awsm_register_function(mod, _fread, "fread");
  awsm_register_function(mod, _fwrite, "fwrite");

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
