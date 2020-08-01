
typedef struct{
  void (* suspend)(io_writer * writer, void * user_data);
  void (* resume) (io_reader * reader, void * user_data);
  void * user_data;
}machine_driver;


typedef struct{
  wasm_module ** modules;
  const char ** module_path;
  size_t module_count;
  bool suspend;

  machine_driver * drivers;
  u32 driver_count;
}machine;


void _malloc(stack * ctx);
void * malloc_impl(wasm_module * mod, size_t size);
void free_impl(wasm_module * mod, void * ptr);
void register_alloc(wasm_module * mod);

void machine_add_driver(machine * m, machine_driver driver);
void machine_add_module(machine * m, wasm_module * mod, const char * path);
