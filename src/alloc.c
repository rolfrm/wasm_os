
#include <iron/full.h>
#include <awsm.h>

typedef wasm_execution_stack stack;
// this should go on the module.
static size_t heap_size = 0;

const u32 page_table_size = 1024;
const u32 page_size = 1024 * 2;
#define LOG2(X) ((unsigned) (8*sizeof (unsigned long long) - __builtin_clzll((X)) - 1))

static void init_mmu(wasm_module * mod){
  if(heap_size != 0) return;
  heap_size = awsm_heap_size(mod);
  awsm_heap_increase(mod, page_table_size * sizeof(u32));
  printf("Heap size: %i new: %i\n", heap_size, awsm_heap_size(mod));
  void * heap_base = awsm_module_heap_ptr(mod);
  u32 * page_table = heap_base + heap_size;
  memset(page_table, 0, page_table_size * sizeof(page_table[0])); 
}

static u32 alloc_pages(wasm_module * mod, u32 count){
  u32 heap_top = awsm_heap_size(mod);

  awsm_heap_increase(mod, page_size * count);
  printf("  Allocating new pages: %i\n", heap_top);
  return heap_top;
}
static void prepare_pages(wasm_module * mod, u32 heap_top, u32 block_count, u32 size){
  void * heap_base = awsm_module_heap_ptr(mod);
  void * page = heap_base + heap_top;
  void * end = page + block_count * page_size;
  u32 counter = heap_top;
  while(page < end - size){
    *((u32 *)page) = counter + size;
    page += size;
    counter += size;
  }
}

u8 get_index(size_t size, u32 *block_size){
  size = size / 4; // no blocks smaller than 4!.
  *block_size = 4;
  for(u8 i = 0; i < 32; i++){
    if(size == 0)
      return i;
    size /= 2;
    *block_size *= 2;
  }
  return 255;
}

void * malloc_impl(wasm_module * mod, size_t size){
  init_mmu(mod);
  size += 1;
  void * heap_base = awsm_module_heap_ptr(mod);
  // 1024 entries in the heap table
  u32 * page_table = heap_base + heap_size;
  u32 block_size;
  u32 i = get_index(size, &block_size);
  u32 block_start = page_table[i];
  //printf("init block start: %i\n", block_start);
  if(block_start == 0){
    
    u32 block_id = alloc_pages(mod, 1);
    heap_base = awsm_module_heap_ptr(mod);
    page_table = heap_base + heap_size;

    prepare_pages(mod, block_id, 1, block_size);
    block_start = block_id;

    page_table[i] = block_start;

  }
  // ok, now just take the block and return the old one.
  void * new_block = heap_base + block_start;
  u32 next_elem = ((u32 *)new_block)[0];

  page_table[i] = next_elem;
  ((u8 *) new_block)[0] = i;
  
  printf("got block: %i\n", block_start);
  return new_block + 1;
}

void free_impl(wasm_module * mod, void * ptr){
  ptr -= 1; // pointer base
  u8 i = ((u8 *)ptr)[0];
  void * heap_base = awsm_module_heap_ptr(mod);
  u32 block_start = (u32)(size_t)(ptr - heap_base);
  printf("Free block: %i\n", block_start);
  u32 * page_table = heap_base + heap_size;
  u32 cv = page_table[i];
  ((u32 *)ptr)[0] = cv;
  page_table[i] = block_start;
}

void _malloc(stack * ctx){
  u32 size = awsm_pop_u32(ctx);
  void * heap_base = awsm_module_heap_ptr(awsm_stack_module(ctx));
  void * ptr = malloc_impl(awsm_stack_module(ctx), size);
  u32 offset = ptr - heap_base;
  awsm_push_u32(ctx, offset);
}


void _free(stack * ctx){
  u32 ptr = awsm_pop_u32(ctx);
  void * heap_base = awsm_module_heap_ptr(awsm_stack_module(ctx));
  free_impl(awsm_stack_module(ctx), heap_base + ptr);
}

void register_alloc(wasm_module * mod){
  awsm_register_function(mod, _malloc, "malloc");
  awsm_register_function(mod, _free, "free");

}

