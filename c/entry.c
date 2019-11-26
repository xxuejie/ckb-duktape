#include "glue.h"
#include "blockchain.h"

#define ERROR_LOADING_SCRIPT -1

#define MAX_WITNESS_SIZE 32768
#define SCRIPT_SIZE 32768

int main(int argc, char *argv[]) {
  unsigned char script[SCRIPT_SIZE];
  uint64_t len = SCRIPT_SIZE;
  int ret = ckb_load_script(script, &len, 0);
  if (ret != CKB_SUCCESS) {
    return ERROR_LOADING_SCRIPT;
  }
  if (len > SCRIPT_SIZE) {
    return ERROR_LOADING_SCRIPT;
  }
  mol_seg_t script_seg;
  script_seg.ptr = (uint8_t *)script;
  script_seg.size = len;
  if (MolReader_Script_verify(&script_seg, false) != MOL_OK) {
    return ERROR_LOADING_SCRIPT;
  }
  mol_seg_t args_seg = MolReader_Script_get_args(&script_seg);
  mol_seg_t args_bytes_seg = MolReader_Bytes_raw_bytes(&args_seg);
  if (args_bytes_seg.size > SCRIPT_SIZE) {
    return ERROR_LOADING_SCRIPT;
  }

  duk_context *ctx = duk_create_heap_default();
  if (argc != 2) {
    return -1;
  }
  ckb_init(ctx);

  if (duk_peval_lstring(ctx, (const char *) args_bytes_seg.ptr, args_bytes_seg.size) != 0) {
    ckb_debug(duk_safe_to_string(ctx, -1));
    return -2;
  }
  duk_pop(ctx); /* pop eval result */
  duk_destroy_heap(ctx);

  return 0;
}
