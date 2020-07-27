//clang-9 --target=wasm32 -nostdlib -Wl,--export-all -Wl,--no-entry -O3 -Wl,-no-gc-sections kernel.c -Wl,--allow-undefined  -o kernel.wasm 
#include "awsm_api.h"

void kernel(){
  while(1){

    awsm_sleep(1000);
    print_str("Hello world\n");
    char * test = malloc(8);
    test[0] = 'A';
    test[1] = 0;
    print_str(test);
    free(test);
  }

}
