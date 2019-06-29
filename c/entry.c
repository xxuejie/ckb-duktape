#include "ckb_syscalls.h"
#include "duktape.h"

duk_double_t dummy_get_now(void) {
  /*
   * Return a fixed time here as a dummy value since CKB does not support
   * fetching current timestamp
   */
  return -11504520000.0;
}

/*
 * Check if v can fit in duk_int_t, if so, push it to duktape stack, otherwise
 * throw an error.
 */
static void push_checked_integer(duk_context *ctx, uint64_t v) {
  if (v == ((uint64_t)((duk_int_t)v))) {
    duk_push_int(ctx, (duk_int_t)v);
  } else {
    duk_push_error_object(ctx, DUK_ERR_EVAL_ERROR, "Integer %lu is overflowed!",
                          v);
    (void)duk_throw(ctx);
  }
}

static void check_ckb_syscall_ret(duk_context *ctx, int ret) {
  if (ret != 0) {
    duk_push_error_object(ctx, DUK_ERR_EVAL_ERROR,
                          "Invalid CKB syscall response: %d", ret);
    (void)duk_throw(ctx);
  }
}

static duk_ret_t duk_ckb_debug(duk_context *ctx) {
  duk_push_string(ctx, " ");
  duk_insert(ctx, 0);
  duk_join(ctx, duk_get_top(ctx) - 1);
  ckb_debug(duk_safe_to_string(ctx, -1));
  return 0;
}

typedef int (*load_hash_function)(void *, volatile uint64_t *, size_t);
typedef int (*load_function)(void *, volatile uint64_t *, size_t, size_t,
                             size_t);
typedef int (*load_by_field_function)(void *, volatile uint64_t *, size_t,
                                      size_t, size_t, size_t);

static duk_ret_t duk_ckb_load_hash(duk_context *ctx, load_hash_function f) {
  char buffer[32];
  volatile uint64_t len = 32;

  check_ckb_syscall_ret(ctx, f(buffer, &len, 0));
  if (len != 32) {
    duk_push_error_object(ctx, DUK_ERR_EVAL_ERROR,
                          "Invalid CKB hash length: %ld", len);
    return duk_throw(ctx);
  }

  duk_push_lstring(ctx, buffer, 32);
  return 1;
}

static duk_ret_t duk_ckb_raw_load(duk_context *ctx, load_function f) {
  if (!(duk_is_buffer_data(ctx, 0) && duk_is_number(ctx, 1) &&
        duk_is_number(ctx, 2) && duk_is_number(ctx, 3))) {
    duk_push_error_object(ctx, DUK_ERR_EVAL_ERROR, "Invalid arguments");
    return duk_throw(ctx);
  }

  size_t buffer_size = 0;
  void *buffer = duk_get_buffer_data(ctx, 0, &buffer_size);
  size_t offset = duk_get_int(ctx, 1);
  size_t index = duk_get_int(ctx, 2);
  size_t source = duk_get_int(ctx, 3);
  duk_pop_n(ctx, 4);

  volatile uint64_t len = buffer_size;
  check_ckb_syscall_ret(ctx, f(buffer, &len, offset, index, source));
  push_checked_integer(ctx, len);

  return 1;
}

static duk_ret_t duk_ckb_load(duk_context *ctx, load_function f) {
  if (!(duk_is_number(ctx, 0) && duk_is_number(ctx, 1) &&
        duk_is_number(ctx, 2))) {
    duk_push_error_object(ctx, DUK_ERR_EVAL_ERROR, "Invalid arguments");
    return duk_throw(ctx);
  }

  size_t offset = duk_get_int(ctx, 1);
  size_t index = duk_get_int(ctx, 2);
  size_t source = duk_get_int(ctx, 3);
  duk_pop_n(ctx, 3);

  volatile uint64_t len = 0;
  check_ckb_syscall_ret(ctx, f(NULL, &len, offset, index, source));

  duk_push_fixed_buffer(ctx, len);
  void *p = duk_get_buffer(ctx, 0, NULL);
  check_ckb_syscall_ret(ctx, f(p, &len, offset, index, source));

  /* Create an ArrayBuffer for ease of handling at JS side */
  duk_push_buffer_object(ctx, 0, 0, len, DUK_BUFOBJ_ARRAYBUFFER);
  duk_swap(ctx, 0, 1);
  duk_pop(ctx);

  return 1;
}

static duk_ret_t duk_ckb_raw_load_by_field(duk_context *ctx,
                                           load_by_field_function f) {
  if (!(duk_is_buffer_data(ctx, 0) && duk_is_number(ctx, 1) &&
        duk_is_number(ctx, 2) && duk_is_number(ctx, 3) &&
        duk_is_number(ctx, 4))) {
    duk_push_error_object(ctx, DUK_ERR_EVAL_ERROR, "Invalid arguments");
    return duk_throw(ctx);
  }

  size_t buffer_size = 0;
  void *buffer = duk_get_buffer_data(ctx, 0, &buffer_size);
  size_t offset = duk_get_int(ctx, 1);
  size_t index = duk_get_int(ctx, 2);
  size_t source = duk_get_int(ctx, 3);
  size_t field = duk_get_int(ctx, 4);
  duk_pop_n(ctx, 5);

  volatile uint64_t len = buffer_size;
  check_ckb_syscall_ret(ctx, f(buffer, &len, offset, index, source, field));
  push_checked_integer(ctx, len);

  return 1;
}

static duk_ret_t duk_ckb_load_by_field(duk_context *ctx,
                                       load_by_field_function f) {
  if (!(duk_is_number(ctx, 0) && duk_is_number(ctx, 1) &&
        duk_is_number(ctx, 2) && duk_is_number(ctx, 3))) {
    duk_push_error_object(ctx, DUK_ERR_EVAL_ERROR, "Invalid arguments");
    return duk_throw(ctx);
  }

  size_t offset = duk_get_int(ctx, 1);
  size_t index = duk_get_int(ctx, 2);
  size_t source = duk_get_int(ctx, 3);
  size_t field = duk_get_int(ctx, 4);
  duk_pop_n(ctx, 4);

  volatile uint64_t len = 0;
  check_ckb_syscall_ret(ctx, f(NULL, &len, offset, index, source, field));

  duk_push_fixed_buffer(ctx, len);
  void *p = duk_get_buffer(ctx, 0, NULL);
  check_ckb_syscall_ret(ctx, f(p, &len, offset, index, source, field));

  /* Create an ArrayBuffer for ease of handling at JS side */
  duk_push_buffer_object(ctx, 0, 0, len, DUK_BUFOBJ_ARRAYBUFFER);
  duk_swap(ctx, 0, 1);
  duk_pop(ctx);

  return 1;
}

static duk_ret_t duk_ckb_load_tx_hash(duk_context *ctx) {
  return duk_ckb_load_hash(ctx, ckb_load_tx_hash);
}

static duk_ret_t duk_ckb_load_script_hash(duk_context *ctx) {
  return duk_ckb_load_hash(ctx, ckb_load_script_hash);
}

static duk_ret_t duk_ckb_raw_load_cell(duk_context *ctx) {
  return duk_ckb_raw_load(ctx, ckb_load_cell);
}

static duk_ret_t duk_ckb_load_cell(duk_context *ctx) {
  return duk_ckb_load(ctx, ckb_load_cell);
}

static duk_ret_t duk_ckb_raw_load_input(duk_context *ctx) {
  return duk_ckb_raw_load(ctx, ckb_load_input);
}

static duk_ret_t duk_ckb_load_input(duk_context *ctx) {
  return duk_ckb_load(ctx, ckb_load_input);
}

static duk_ret_t duk_ckb_raw_load_header(duk_context *ctx) {
  return duk_ckb_raw_load(ctx, ckb_load_header);
}

static duk_ret_t duk_ckb_load_header(duk_context *ctx) {
  return duk_ckb_load(ctx, ckb_load_header);
}

static duk_ret_t duk_ckb_raw_load_witness(duk_context *ctx) {
  return duk_ckb_raw_load(ctx, ckb_load_witness);
}

static duk_ret_t duk_ckb_load_witness(duk_context *ctx) {
  return duk_ckb_load(ctx, ckb_load_witness);
}

static duk_ret_t duk_ckb_raw_load_cell_by_field(duk_context *ctx) {
  return duk_ckb_raw_load_by_field(ctx, ckb_load_cell_by_field);
}

static duk_ret_t duk_ckb_load_cell_by_field(duk_context *ctx) {
  return duk_ckb_load_by_field(ctx, ckb_load_cell_by_field);
}

static duk_ret_t duk_ckb_raw_load_input_by_field(duk_context *ctx) {
  return duk_ckb_raw_load_by_field(ctx, ckb_load_input_by_field);
}

static duk_ret_t duk_ckb_load_input_by_field(duk_context *ctx) {
  return duk_ckb_load_by_field(ctx, ckb_load_input_by_field);
}

void ckb_init(duk_context *ctx) {
  duk_push_object(ctx);

  duk_push_c_function(ctx, duk_ckb_debug, DUK_VARARGS);
  duk_put_prop_string(ctx, -2, "debug");

  duk_push_c_function(ctx, duk_ckb_load_tx_hash, 0);
  duk_put_prop_string(ctx, -2, "load_tx_hash");

  duk_push_c_function(ctx, duk_ckb_load_script_hash, 0);
  duk_put_prop_string(ctx, -2, "load_script_hash");

  duk_push_c_function(ctx, duk_ckb_raw_load_cell, 4);
  duk_put_prop_string(ctx, -2, "raw_load_cell");
  duk_push_c_function(ctx, duk_ckb_load_cell, 3);
  duk_put_prop_string(ctx, -2, "load_cell");

  duk_push_c_function(ctx, duk_ckb_raw_load_input, 4);
  duk_put_prop_string(ctx, -2, "raw_load_input");
  duk_push_c_function(ctx, duk_ckb_load_input, 3);
  duk_put_prop_string(ctx, -2, "load_input");

  duk_push_c_function(ctx, duk_ckb_raw_load_header, 4);
  duk_put_prop_string(ctx, -2, "raw_load_header");
  duk_push_c_function(ctx, duk_ckb_load_header, 3);
  duk_put_prop_string(ctx, -2, "load_header");

  duk_push_c_function(ctx, duk_ckb_raw_load_witness, 4);
  duk_put_prop_string(ctx, -2, "raw_load_witness");
  duk_push_c_function(ctx, duk_ckb_load_witness, 3);
  duk_put_prop_string(ctx, -2, "load_witness");

  duk_push_c_function(ctx, duk_ckb_raw_load_cell_by_field, 5);
  duk_put_prop_string(ctx, -2, "raw_load_cell_by_field");
  duk_push_c_function(ctx, duk_ckb_load_cell_by_field, 4);
  duk_put_prop_string(ctx, -2, "load_cell_by_field");

  duk_push_c_function(ctx, duk_ckb_raw_load_input_by_field, 5);
  duk_put_prop_string(ctx, -2, "raw_load_input_by_field");
  duk_push_c_function(ctx, duk_ckb_load_input_by_field, 4);
  duk_put_prop_string(ctx, -2, "load_input_by_field");

  duk_put_global_string(ctx, "CKB");
}

int main(int argc, char *argv[]) {
  duk_context *ctx = duk_create_heap_default();
  ckb_init(ctx);

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
