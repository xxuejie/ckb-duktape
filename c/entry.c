#include "glue.h"

int main(int argc, char *argv[]) {
  duk_context *ctx = duk_create_heap_default();
  if (argc != 2) {
    return -1;
  }
  ckb_init(ctx);

  if (duk_peval_string(ctx, argv[1]) != 0) {
    ckb_debug(duk_safe_to_string(ctx, -1));
    return -2;
  }
  duk_pop(ctx); /* pop eval result */
  duk_destroy_heap(ctx);

  return 0;
}
