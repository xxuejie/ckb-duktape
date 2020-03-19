/*
 * This assume the first dep cell contains the actual JS code to load,
 * then loads it directly. It is a tradeoff that sacrifices the first
 * cell, but frees script args part for script to use.
 */
#include "glue.h"

#define SCRIPT_SIZE (128 * 1024)

int main() {
  duk_context *ctx = duk_create_heap_default();
  ckb_init(ctx);

  syscall(4097, 0, 0, 0, 0, 0, 0);

  unsigned char script[SCRIPT_SIZE];
  uint64_t len = SCRIPT_SIZE;
  int ret = ckb_load_cell_data(script, &len, 0, 0, CKB_SOURCE_CELL_DEP);
  if (ret != CKB_SUCCESS) {
    return ret;
  }
  if (len > SCRIPT_SIZE) {
    return -1;
  }

  if (script[0] == 0xbf) {
    /* Bytecode */
    void *buf = duk_push_fixed_buffer(ctx, len);
    memcpy(buf, script, len);
    duk_load_function(ctx);
  } else {
    /* Source */
    if (duk_pcompile_lstring(ctx, 0, (const char *) script, len) != 0) {
      ckb_debug(duk_safe_to_string(ctx, -1));
      return -2;
    }
  }

  /* Provide a this value for convenience */
  duk_push_global_object(ctx);

  if (duk_pcall_method(ctx, 0) != 0) {
    ckb_debug(duk_safe_to_string(ctx, -1));
    return -3;
  }

  /* Skipping cleanup step to further save cycles */
  /* duk_pop(ctx); */
  /* duk_destroy_heap(ctx); */

  return 0;
}
