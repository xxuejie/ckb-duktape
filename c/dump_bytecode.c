#include <stdio.h>
#include <stdlib.h>

#include "duktape.h"

duk_double_t dummy_get_now(void) {
  /*
   * Return a fixed time here as a dummy value since CKB does not support
   * fetching current timestamp
   */
  return -11504520000.0;
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    printf("Usage: %s <input source file name> <output bytecode file name>\n", argv[0]);
    return 1;
  }

  FILE *fp = fopen(argv[1], "rb");
  if (!fp) {
    printf("Failed to open file %s\n", argv[1]);
    return 2;
  }
  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  void *buf = malloc(size);
  if (!buf) {
    printf("Failed to allocate buffer!\n");
    fclose(fp);
    return 2;
  }

  if (fread(buf, size, 1, fp) != 1) {
    printf("Failed to read file!\n");
    fclose(fp);
    return 2;
  }
  fclose(fp);

  duk_context *ctx = duk_create_heap_default();
  if (duk_pcompile_lstring(ctx, 0, buf, size) != 0) {
    printf("Compile failed: %s\n", duk_safe_to_string(ctx, -1));
    free(buf);
    duk_destroy_heap(ctx);
    return 3;
  }
  duk_dump_function(ctx);
  free(buf);

  size_t out_size = 0;
  void *out_buf = duk_require_buffer_data(ctx, -1, &out_size);

  FILE *out_fp = fopen(argv[2], "wb");
  if (!out_fp) {
    printf("Failed to open file %s\n", argv[2]);
    duk_destroy_heap(ctx);
    return 2;
  }

  fwrite(out_buf, out_size, 1, out_fp);
  fclose(out_fp);

  return 0;
}
