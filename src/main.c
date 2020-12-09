#include <iron/full.h>
#include <microio.h>
#include <awsm.h>

typedef wasm_execution_stack stack;
#include "awsmvm.h"

void _sleep(stack * ctx){
  i32 v;
  v = awsm_pop_i32(ctx);
  iron_usleep(v * 1000);
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
  awsm_register_function(mod, _suspend_machine, "suspend_machine");
  awsm_module_set_user_data(mod, m);
  m->module_count += 1;
  m->modules = realloc(m->modules, sizeof(m->modules[0]) * m->module_count);
  m->module_path = realloc(m->module_path, sizeof(m->module_path[0]) * m->module_count);
  
  m->modules[m->module_count - 1] = mod;
  m->module_path[m->module_count - 1] = fmtstr("%s", path);

  for(u32 i = 0; i < m->driver_count; i++){
    m->drivers[i].register_driver(mod, m->drivers[i].user_data);
  }

}

void read_some(void * data, u64 count, void * ptr){
  FILE * f = ptr;
  fread(data, count, 1, f);
}


void write_some(void * data, u64 count, void * ptr){
  FILE * f = ptr;
  fwrite(data, count, 1, f);
}


static u32 marker_object = 0x67391fa2;
void suspend_core(machine * m, const char * path){
  printf("Suspend core\n");

  char buf[200];
  sprintf(buf, "%s.tmp", path);
  FILE * f = fopen(buf, "w");
  binary_io io = {.f = write_some, .user_data = f};
  io_write_u32(&io, m->module_count);
  for(u32 i = 0; i < m->module_count; i++){
    io_write_str0(&io, m->module_path[i]);
    awsm_module_save(&io, m->modules[i]);
    io_write_u32(&io, marker_object);
  }
  
  for(u32 i = 0; i < m->driver_count; i++){
    m->drivers[i].suspend(&io, m->drivers[i].user_data);
    io_write_u32(&io, marker_object);
  }
  
  fclose(f);
  remove(path);
  rename(buf, path);
}

void _resume_core(binary_io * rd, machine * m){
  u32 module_count = io_read_u32(rd);
  printf("Modules: %i\n", module_count);
  for(u32 i = 0; i < module_count; i++){
    char * path = io_read_str0(rd);
    printf("loading module %s \n", path);
    wasm_module * mod = awsm_load_module_from_file(path);
    machine_add_module(m, mod, path);
    awsm_module_load(rd, mod);
    u32 marker_check = io_read_u32(rd);
    ASSERT(marker_check == marker_object);
  }

  for(u32 i = 0; i < m->driver_count; i++){
    m->drivers[i].resume(rd, m->drivers[i].user_data);
    u32 marker_check = io_read_u32(rd);
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
