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


    file * f2 = fopen("/hello3", "a");
    char * towrite = "asd";
    fwrite(towrite, 3 ,1 ,f2);
    fclose(f2);
    
    file * f = fopen("/hello3", "rw+");
    print_i32((int) f);
    print_str("\n"); 
    if(f != NULL){
      char buffer[100];
      fread(buffer, 100, 1, f);
      print_str(buffer);
      print_str("\n");
      fclose(f);
    }
    
  }

}
