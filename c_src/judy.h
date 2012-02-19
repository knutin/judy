static ErlNifResourceType* JUDY_RESOURCE;

typedef struct _judy_t
{
  unsigned long max_keys;
  size_t value_size;

  Pvoid_t judy;
  unsigned long num_keys;

  char* buf;
  unsigned long last_index;

} judy_t;

typedef struct _judy_value_t {
  size_t size;
  unsigned char* value;
} judy_value;

