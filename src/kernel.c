//clang-9 --target=wasm32 -nostdlib -Wl,--export-all -Wl,--no-entry -O3 -Wl,-no-gc-sections kernel.c -Wl,--allow-undefined  -o kernel.wasm 
#include "awsm_api.h"

void kernel(){
  char c = 'A';
  while(1){
    if(c > 'Z')
      c = 'A';
    print_str("Hello world\n"); 
    char * test = malloc(8);
    test[0] = c++;
    test[1] = '\n';
    test[2] = 0;
    print_str(test);
    free(test);
    if(0){
    

    file * f2 = fopen("/hello3", "a");
    char * towrite = "asd";
    fwrite(towrite, 3 ,1 ,f2);
    fclose(f2);
    
    file * f = fopen("/hello3", "rw+");
    print_i32((int) f);
    print_str("\n"); 
    if(f != NULL){
      char buffer[100];
      int cnt = 0;
      while(0 < (cnt = fread(buffer, 1, 100, f))){
	buffer[cnt] = 0;
	print_str(buffer);
      }
      print_str("\n");
      fclose(f);
    }

    }
    suspend_machine();
    yield();
  }

}
