#include "ckb_syscalls.h"
#include "duktape.h"

duk_double_t dummy_get_now(void) {
  /*
   * Return a fixed time here as a dummy value since CKB does not support
   * fetching current timestamp
   */
  return -11504520000.0;
}

static duk_ret_t duk_ckb_debug(duk_context *ctx) {
  duk_push_string(ctx, " ");
  duk_insert(ctx, 0);
  duk_join(ctx, duk_get_top(ctx) - 1);
  ckb_debug(duk_safe_to_string(ctx, -1));
  return 0;
}

int main(int argc, char *argv[]) {
  duk_context *ctx = duk_create_heap_default();

  duk_push_c_function(ctx, duk_ckb_debug, DUK_VARARGS);
  duk_put_global_string(ctx, "ckb_debug");

  if (argc == 2) {
    duk_eval_string(ctx, argv[1]);
    duk_pop(ctx); /* pop eval result */
  } else if (argc == 3) {
    /* TODO: load source from one of the dep */
  } else {
    return -1;
  }

  duk_destroy_heap(ctx);

  return 0;
}
