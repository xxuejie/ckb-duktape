/*
 * This assume the first dep cell contains the actual JS code to load,
 * then loads it directly. It is a tradeoff that sacrifices the first
 * cell, but frees script args part for script to use.
 */
#include "glue.h"

#define SCRIPT_SIZE (128 * 1024)

int main(int argc, char* argv[]) {
  duk_context *ctx = duk_create_heap_default();
  ckb_init(ctx);

  syscall(4097, 0, 0, 0, 0, 0, 0);

  int ret = ckb_load_js_source(ctx, argc, argv);
  if (ret != 0) {
    /* duk_destroy_heap(ctx); */
    return ret;
  }

  /* Skipping cleanup step to further save cycles */
  /* duk_pop(ctx); */
  /* duk_destroy_heap(ctx); */

  return 0;
}
