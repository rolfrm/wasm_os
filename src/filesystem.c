#include <iron/full.h>
#include <awsm.h>

typedef wasm_execution_stack stack;
#include "awsmvm.h"

typedef struct{
  char * name;
  char * perm;
  u64 offset;
  FILE * file;
  int next;
}file_data;

typedef struct{
  file_data * files;
  u32 files_count;
  i32 free;
}filesystem_data;


static filesystem_data fd = {.files = NULL, .files_count = 0, .free = -1};

void filesystem_suspend(data_writer * writer, void * user_data){
  UNUSED(user_data);
  writer_write_u32(writer, fd.files_count);
  writer_write(writer, fd.files, sizeof(fd.files[0]) * fd.files_count);
  for(u32 i = 0; i < fd.files_count; i++){
    if(fd.files[i].name != NULL){
      ASSERT(fd.files[i].file != NULL);
      writer_write_str(writer, fd.files[i].name);
      writer_write_str(writer, fd.files[i].perm);
    }
  }

  writer_write_i32(writer, fd.free);
}

void filesystem_resume(data_reader * reader, void * user_data){
  UNUSED(user_data);
  u32 count = reader_readu32_fixed(reader);
  fd.files = alloc0(sizeof(fd.files[0]) * count);
  fd.files_count = count;
  reader_read(reader, fd.files, sizeof(fd.files[0]) * count);
  for(u32 i = 0; i < count; i++){
    if(fd.files[i].name != NULL){
      fd.files[i].name = reader_read_str(reader);
      fd.files[i].perm = reader_read_str(reader);
      fd.files[i].file = fopen(fd.files[i].name, fd.files[i].perm);
      fseek(fd.files[i].file, fd.files[i].offset, SEEK_SET);
      ASSERT(fd.files[i].file != NULL);
    }else{
      ASSERT(fd.files[i].file == NULL);
    }

  }
  
  fd.free = reader_readi32_fixed(reader);
  
}

void filesystem_register(wasm_module * mod, void * user_data);
machine_driver filesystem_driver(){
  machine_driver r;
  r.suspend = filesystem_suspend;
  r.resume = filesystem_resume;
  r.register_driver = filesystem_register;
  r.user_data = alloc0(sizeof(filesystem_data));
  return r;
}

u32 encode_file_id(u32 v){
  return (v << 8) | 0x33;
}

u32 decode_file_id(u32 v){
  return v >> 8;
}


void _fopen(stack * ctx){
  char * perm = awsm_pop_ptr(ctx);
  char * str = awsm_pop_ptr(ctx);
  if(perm != NULL && str != NULL){
   
    char buf[100];
    if(str[0] == '/'){
      sprintf(buf, "system%s", str);
    }else{
      sprintf(buf, "system/%s", str);
    }
    FILE * f = fopen(buf, perm);
    if(f != NULL){
      
      u32 f_id = fd.files_count;
      if(fd.free >= 0){
	f_id = (u32)fd.free;
	fd.free = (i32)fd.files[f_id].next;
      }
      else
	fd.files = realloc(fd.files, (fd.files_count += 1) * sizeof(fd.files[0]));
      
      fd.files[f_id].name = fmtstr("%s", buf);
      fd.files[f_id].perm = fmtstr("%s", perm);
      fd.files[f_id].offset = ftell(f);
      fd.files[f_id].file = f;
      fd.files[f_id].next = -1;
      u32 eid = encode_file_id(f_id);
      awsm_push_u32(ctx, eid);
      return;
    }
  }
  awsm_push_i32(ctx, 0);  
}


void _fclose(stack * ctx){
  u32 f_id = decode_file_id(awsm_pop_u32(ctx));
  file_data * f = &fd.files[f_id];
  fclose(f->file);
  free(f->name);
  free(f->perm);
  f->name = NULL;
  f->perm = NULL;
  f->file = NULL;
  f->next = fd.free;
  fd.free = (int)f_id;
}

void _fread(stack * ctx){
  u32 f_id = decode_file_id(awsm_pop_u32(ctx));
  u32 count = awsm_pop_u32(ctx);
  u32 size = awsm_pop_u32(ctx);
  void * buffer = awsm_pop_ptr(ctx);


  file_data * f = &fd.files[f_id];
  
  size_t read = fread(buffer,size,count, f->file);
  f->offset = ftell(f->file);
  awsm_push_u32(ctx, (u32)read);
}


void _fwrite(stack * ctx){
  u32 encoded_fid = awsm_pop_u32(ctx);
  u32 count = awsm_pop_u32(ctx);
  u32 size = awsm_pop_u32(ctx);
  void * buffer = awsm_pop_ptr(ctx);

  u32 f_id = decode_file_id(encoded_fid);
  file_data * f = &fd.files[f_id];
  
  size_t read = fwrite(buffer,size,count, f->file);
  f->offset = ftell(f->file);
  awsm_push_u32(ctx, (u32)read);
}


void filesystem_register(wasm_module * mod, void * user_data){
  UNUSED(user_data);
  awsm_register_function(mod, _fopen, "fopen");
  awsm_register_function(mod, _fclose, "fclose");
  awsm_register_function(mod, _fread, "fread");
  awsm_register_function(mod, _fwrite, "fwrite");
}
