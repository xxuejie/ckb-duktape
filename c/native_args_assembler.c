#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "args.h"

int to_int(char ch, uint8_t *value) {
  if (ch >= '0' && ch <= '9') {
    *value = ch - '0';
    return 0;
  } else if (ch >= 'a' && ch <= 'f') {
    *value = ch - 'a' + 10;
    return 0;
  } else if (ch >= 'A' && ch <= 'F') {
    *value = ch = 'A' + 10;
    return 0;
  }
  return 2;
}

int decode_hex_in_place(char *buf, size_t *len) {
  size_t src = 0;
  size_t dst = 0;

  while (buf[src] != '\0') {
    if (buf[src + 1] == '\0') {
      return 3;
    }
    uint8_t higher = 0;
    int ret = to_int(buf[src], &higher);
    if (ret != 0) {
      return ret;
    }
    uint8_t lower = 0;
    ret = to_int(buf[src + 1], &lower);
    if (ret != 0) {
      return ret;
    }

    src += 2;
    buf[dst++] = (higher << 4) | lower;
  }
  *len = dst;
  return 0;
}

int main(int argc, char *argv[]) {
  int opt;

  mol_builder_t args_builder;
  MolBuilder_BytesVec_init(&args_builder);

  char *source_file_path = NULL;
  int cell_index = 0;
  long cell_source = 3; /* cell dep */

  while ((opt = getopt(argc, argv, "f:i:s:t:b:")) != -1) {
    switch (opt) {
      case 'f': {
        if (source_file_path != NULL) {
          free(source_file_path);
        }
        source_file_path = (char *)malloc(strlen(optarg) + 1);
        strcpy(source_file_path, optarg);
      } break;
      case 'i': {
        cell_index = atoi(optarg);
      } break;
      case 's': {
        cell_source = atol(optarg);
      } break;
      case 't': {
        mol_builder_t arg_builder;
        MolBuilder_Bytes_init(&arg_builder);
        for (size_t i = 0; i < strlen(optarg); i++) {
          MolBuilder_Bytes_push(&arg_builder, optarg[i]);
        }
        mol_seg_res_t built = MolBuilder_Bytes_build(arg_builder);
        MolBuilder_BytesVec_push(&args_builder, built.seg.ptr, built.seg.size);
      } break;
      case 'b': {
        char *buf = (char *)malloc(strlen(optarg) + 1);
        strcpy(buf, optarg);

        size_t len = 0;
        int ret = decode_hex_in_place(buf, &len);
        if (ret != 0) {
          fprintf(stderr, "Hex error!");
          exit(EXIT_FAILURE);
        }

        mol_builder_t arg_builder;
        MolBuilder_Bytes_init(&arg_builder);
        for (size_t i = 0; i < len; i++) {
          MolBuilder_Bytes_push(&arg_builder, buf[i]);
        }
        mol_seg_res_t built = MolBuilder_Bytes_build(arg_builder);
        MolBuilder_BytesVec_push(&args_builder, built.seg.ptr, built.seg.size);
      } break;
      default: {
        fprintf(
            stderr,
            "Usage: %s -f <JS source file path> -i <cell index> -s <cell "
            "source "
            "value> -t <text input arg> -b <binary input arg in hex>\n"
            "Note -f will have higher priority than -i/-s, meaning when -f is "
            "used, -i and -s will be ignored\n"
            "Many -t and -b values can be included, they will be processed in "
            "the order they appear\n",
            argv[0]);
        exit(EXIT_FAILURE);
      } break;
    }
  }

  mol_builder_t source_builder;
  MolBuilder_SourceOrBytecode_init(&source_builder);
  if (source_file_path != NULL) {
    FILE *f = fopen(source_file_path, "r");
    if (f == NULL) {
      fprintf(stderr, "Cannot open file %s!\n", source_file_path);
      exit(EXIT_FAILURE);
    }
    fseek(f, 0, SEEK_END);
    size_t len = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *buf = (uint8_t *)malloc(len);
    if (fread(buf, 1, len, f) != len) {
      fprintf(stderr, "Read error!\n");
      exit(EXIT_FAILURE);
    }
    fclose(f);
    free(source_file_path);

    mol_builder_t bytes_builder;
    MolBuilder_Bytes_init(&bytes_builder);
    for (size_t i = 0; i < len; i++) {
      MolBuilder_Bytes_push(&bytes_builder, buf[i]);
    }
    free(buf);

    mol_seg_res_t bytes = MolBuilder_Bytes_build(bytes_builder);
    MolBuilder_SourceOrBytecode_set_Bytes(&source_builder, bytes.seg.ptr,
                                          bytes.seg.size);
  } else {
    mol_builder_t index_builder;
    MolBuilder_Uint32_init(&index_builder);
    MolBuilder_Uint32_set_nth0(&index_builder, ((uint8_t *)(&cell_index))[0]);
    MolBuilder_Uint32_set_nth1(&index_builder, ((uint8_t *)(&cell_index))[1]);
    MolBuilder_Uint32_set_nth2(&index_builder, ((uint8_t *)(&cell_index))[2]);
    MolBuilder_Uint32_set_nth3(&index_builder, ((uint8_t *)(&cell_index))[3]);
    mol_seg_t index_seg = MolBuilder_Uint32_build(index_builder).seg;

    mol_builder_t source_builder;
    MolBuilder_Uint64_init(&source_builder);
    MolBuilder_Uint64_set_nth0(&source_builder, ((uint8_t *)(&cell_source))[0]);
    MolBuilder_Uint64_set_nth1(&source_builder, ((uint8_t *)(&cell_source))[1]);
    MolBuilder_Uint64_set_nth2(&source_builder, ((uint8_t *)(&cell_source))[2]);
    MolBuilder_Uint64_set_nth3(&source_builder, ((uint8_t *)(&cell_source))[3]);
    MolBuilder_Uint64_set_nth4(&source_builder, ((uint8_t *)(&cell_source))[4]);
    MolBuilder_Uint64_set_nth5(&source_builder, ((uint8_t *)(&cell_source))[5]);
    MolBuilder_Uint64_set_nth6(&source_builder, ((uint8_t *)(&cell_source))[6]);
    MolBuilder_Uint64_set_nth7(&source_builder, ((uint8_t *)(&cell_source))[7]);
    mol_seg_t source_seg = MolBuilder_Uint64_build(source_builder).seg;

    mol_builder_t existing_cell_builder;
    MolBuilder_ExistingCell_init(&existing_cell_builder);
    MolBuilder_ExistingCell_set_index(&existing_cell_builder, index_seg.ptr);
    MolBuilder_ExistingCell_set_source(&existing_cell_builder, source_seg.ptr);
    mol_seg_t existing_cell_seg =
        MolBuilder_ExistingCell_build(existing_cell_builder).seg;
    MolBuilder_SourceOrBytecode_set_ExistingCell(
        &source_builder, existing_cell_seg.ptr, existing_cell_seg.size);
  }

  mol_seg_t source_seg = MolBuilder_SourceOrBytecode_build(source_builder).seg;
  mol_seg_t args_seg = MolBuilder_BytesVec_build(args_builder).seg;

  mol_builder_t input_builder;
  MolBuilder_Input_init(&input_builder);
  MolBuilder_Input_set_source(&input_builder, source_seg.ptr, source_seg.size);
  MolBuilder_Input_set_args(&input_builder, args_seg.ptr, args_seg.size);

  mol_seg_t input = MolBuilder_Input_build(input_builder).seg;
  size_t len = input.size;

  char *buf = (char *)malloc(len * 2 + 1);
  for (size_t i = 0; i < len; i++) {
    snprintf(&buf[i * 2], 3, "%02x", input.ptr[i]);
  }

  printf("%s\n", buf);

  return 0;
}
