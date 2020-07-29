
extern void awsm_sleep(int ms);
extern void print_str(const char * str);
extern void print_i32(int i);

void *malloc (unsigned long size);
void free(void * ptr);


typedef struct _file file;

typedef file FILE;
typedef unsigned long u32;
typedef u32 size_t;

FILE * fopen(const char * file, const char * perm);
void fclose(file * f);
size_t fread ( void * ptr, size_t size, size_t count, FILE * stream );
size_t fwrite ( const void * ptr, size_t size, size_t count, FILE * stream );
#define NULL 0
