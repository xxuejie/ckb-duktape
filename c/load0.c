/*
 * This assume the first dep cell contains the actual JS code to load,
 * then loads it directly. It is a tradeoff that sacrifices the first
 * cell, but frees script args part for script to use.
 */
#include "glue.h"

#define SCRIPT_SIZE (128 * 1024)

int main() {
  unsigned char script[SCRIPT_SIZE];
  uint64_t len = SCRIPT_SIZE;
  int ret = ckb_load_cell_data(script, &len, 0, 0, CKB_SOURCE_CELL_DEP);
  if (ret != CKB_SUCCESS) {
    return ret;
  }
  if (len > SCRIPT_SIZE) {
    return -1;
  }

  duk_context *ctx = duk_create_heap_default();
  ckb_init(ctx);

  if (duk_peval_lstring(ctx, (const char *) script, len) != 0) {
    ckb_debug(duk_safe_to_string(ctx, -1));
    return -2;
  }
  duk_pop(ctx); /* pop eval result */
  duk_destroy_heap(ctx);

  return 0;
}
