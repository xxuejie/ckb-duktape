#include "glue.h"

int main(int argc, char* argv[]) {
  duk_context *ctx = duk_create_heap_default();
  ckb_init(ctx);

  int ret = ckb_load_js_source(ctx, argc, argv);
  if (ret != 0) {
    duk_destroy_heap(ctx);
    return ret;
  }

  duk_pop(ctx); /* pop eval result */
  duk_destroy_heap(ctx);

  return 0;
}
