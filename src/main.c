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

/*u32 module_add_file(module_data * data){
  data->open_file_count += 1;
  data->open_files = realloc(data->open_files, data->open_file_count * sizeof(data->open_files[0]));
  u32 f = data->open_file_count - 1;
  data->open_files[f] = 0;
  return f;

  }*/
void _fopen(stack * ctx){
  char * perm = awsm_pop_ptr(ctx);
  char * str = awsm_pop_ptr(ctx);
  
  //module_data * data = get_module_data(awsm_stack_module(ctx));
  
  char buf[100];
  if(str[0] == '/'){
    sprintf(buf, "system%s", str);
  }else{
    sprintf(buf, "system/%s", str);
  }
  FILE * f = fopen(buf, perm);
  if(f != NULL){
    //u32 index = module_add_file(data);
    //data->open_files[index] = (u64)f;
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


void _suspend_machine(stack * ctx){
  machine * vm = awsm_module_get_user_data(awsm_stack_module(ctx));
  vm->suspend = true;
}

void machine_add_driver(machine * m, machine_driver driver){
  m->drivers = realloc(m->drivers, (m->driver_count += 1) * sizeof(m->drivers[0]));
  m->drivers[m->driver_count - 1] = driver;
}

void machine_add_module(machine * m, wasm_module * mod, const char * path){

  awsm_register_function(mod, _sleep, "awsm_sleep");
  awsm_register_function(mod, _fopen, "fopen");
  awsm_register_function(mod, _fclose, "fclose");
  awsm_register_function(mod, _fread, "fread");
  awsm_register_function(mod, _fwrite, "fwrite");
  awsm_register_function(mod, _suspend_machine, "suspend_machine");
  awsm_module_set_user_data(mod, m);
  m->module_count += 1;
  m->modules = realloc(m->modules, sizeof(m->modules[0]) * m->module_count);
  m->module_path = realloc(m->module_path, sizeof(m->module_path[0]) * m->module_count);

  
  m->modules[m->module_count - 1] = mod;
  m->module_path[m->module_count - 1] = fmtstr("%s", path);
  

}

void read_some(void * data, u64 count, void * ptr){
  FILE * f = ptr;
  fread(data, count, 1, f);
}


void write_some(void * data, u64 count, void * ptr){
  FILE * f = ptr;
  fwrite(data, count, 1, f);
}

void writer_write_str(binary_io * io, const char * str){
  int len = strlen(str);
  writer_write(io, str, len + 1);
}

char * reader_read_str(binary_io * io){
  char * buf = NULL;
  char c = 0;
  size_t s = 0;
  do{
    c = reader_read1(io);
    buf = realloc(buf, (s = s + 1));
    buf[s- 1] = c;
  }while(c != 0);
  return buf;
}

static u32 marker_object = 0x67391fa2;
void suspend_core(machine * m, const char * path){
  printf("Suspend core\n");

  char buf[200];
  sprintf(buf, "%s.tmp", path);
  FILE * f = fopen(buf, "w");
  binary_io io = {.f = write_some, .user_data = f};
  writer_write_u32(&io, m->module_count);
  for(u32 i = 0; i < m->module_count; i++){
    writer_write_str(&io, m->module_path[i]);
    awsm_module_save(&io, m->modules[i]);
    writer_write_u32(&io, marker_object);
  }
  
  for(u32 i = 0; i < m->driver_count; i++){
    m->drivers[i].suspend(&io, m->drivers[i].user_data);
    writer_write_u32(&io, marker_object);
  }
  
  fclose(f);
  remove(path);
  rename(buf, path);
}

void _resume_core(binary_io * rd, machine * m){
  u32 module_count = reader_readu32_fixed(rd);
  printf("Modules: %i\n", module_count);
  for(u32 i = 0; i < module_count; i++){
    char * path = reader_read_str(rd);
    printf("loading module %s \n", path);
    wasm_module * mod = awsm_load_module_from_file(path);
    machine_add_module(m, mod, path);
    awsm_module_load(rd, mod);
    u32 marker_check = reader_readu32_fixed(rd);
    ASSERT(marker_check == marker_object);
  }

  for(u32 i = 0; i < m->driver_count; i++){
    m->drivers[i].resume(rd, m->drivers[i].user_data);
    u32 marker_check = reader_readu32_fixed(rd);
    ASSERT(marker_check == marker_object);
  }
}

bool resume_core(machine * m, const char * path){
  printf("Resume core\n");
  FILE * f = fopen(path, "r");
  if(f == NULL) return false;
  binary_io io = {.f = read_some, .user_data = f};
  _resume_core(&io, m);
  fclose(f);
  return true;
}

typedef struct{
  int test;
}filesystem_data;

void filesystem_suspend(data_writer * writer, void * user_data){
  UNUSED(writer); UNUSED(user_data);
  filesystem_data * fd = user_data;
  fd->test += 1;
  writer_write_i32(writer, fd->test);
  printf("Suspend: %i\n", fd->test);
}

void filesystem_resume(data_reader * reader, void * user_data){
  UNUSED(reader); UNUSED(user_data);

  i32 test2 = reader_readi32_fixed(reader);
  filesystem_data * fd = user_data;
  fd->test = test2;
  printf("Resume: %i\n", test2);
}


machine_driver filesystem_driver(){
  machine_driver r;
  r.suspend = filesystem_suspend;
  r.resume = filesystem_resume;
  r.user_data = alloc0(sizeof(filesystem_data));
  return r;
}


int main(int argc, char ** argv){
  UNUSED(argc);UNUSED(argv);
  

  //awsm_log_diagnostic = true;
  machine vm = {0};
  machine_add_driver(&vm, filesystem_driver());
  bool resumed = resume_core(&vm, "core.bin");
  if(!resumed){
    wasm_module * mod = awsm_load_module_from_file("kernel.wasm");
    if(awsm_load_thread(mod, "kernel") == false){
      printf("Unable to load thread");
      return 1;
    }
    machine_add_module(&vm, mod, "kernel.wasm");
  }


  while(!vm.suspend){
    bool any_process = false;
    for(u32 i = 0; i < vm.module_count; i++){
      wasm_module * mod = vm.modules[i];
      any_process |= awsm_process(mod, 50);
      if(vm.suspend){

	break;
      }
    }
    if(!any_process)
      // every process on the kernel, including the kernel process has ended.
      break; 
  }

  if(vm.suspend){
    suspend_core(&vm, "core.bin");
  }
  

   
  
  return 0;
}
