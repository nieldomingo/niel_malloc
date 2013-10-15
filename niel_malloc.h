
#define align8(x) (((((x) - 1)>>3)<<3)+8)
#define calc_mem_blk(x) (void *)((char *)(x) + mem_blk_header_size)

typedef struct mem_blk_header {
    struct mem_blk_header *next;
    struct mem_blk_header *prev;
    void *mem_blk;
    size_t blk_size;
    int assigned_flag;
} mem_blk_header;

static mem_blk_header *heap_start = NULL;
static size_t mem_blk_header_size = align8(sizeof(mem_blk_header));

mem_blk_header  *allocate_new_blk(mem_blk_header *last_blk, size_t size);
mem_blk_header *assign_blk(mem_blk_header *blk, size_t size);
void *niel_malloc(size_t size);
void collapse_consecutive_blks(mem_blk_header *start_blk);
void niel_free(void *ptr);
void *niel_calloc (size_t nr, size_t size);
