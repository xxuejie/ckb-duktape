#include "glue.h"

void duk_print_alert_init(duk_context *ctx, duk_uint_t flags);
static int handle_interactive(duk_context *ctx);

int main() {
  duk_context *ctx = duk_create_heap_default();
  ckb_init(ctx);

  int ret = ckb_load_js_source(ctx);
  if (ret != 0) {
    duk_destroy_heap(ctx);
    return ret;
  }

  duk_print_alert_init(ctx, 0 /*flags*/);

  handle_interactive(ctx);

  duk_pop(ctx); /* pop eval result */
  duk_destroy_heap(ctx);

  return 0;
}

/* Adapted from https://github.com/svaarala/duktape/blob/44ca54f726bfa651a7ab59286dd5c371dba2ddfc/examples/cmdline/duk_cmdline.c */
#define  LINEBUF_SIZE       65536
static int interactive_mode = 0;

static void print_pop_error(duk_context *ctx, FILE *f) {
	fprintf(f, "%s\n", duk_safe_to_stacktrace(ctx, -1));
	fflush(f);
	duk_pop(ctx);
}

static duk_ret_t wrapped_compile_execute(duk_context *ctx, void *udata) {
	const char *src_data;
	duk_size_t src_len;
	duk_uint_t comp_flags;

	(void) udata;

	/* XXX: Here it'd be nice to get some stats for the compilation result
	 * when a suitable command line is given (e.g. code size, constant
	 * count, function count.  These are available internally but not through
	 * the public API.
	 */

	/* Use duk_compile_lstring_filename() variant which avoids interning
	 * the source code.  This only really matters for low memory environments.
	 */

	/* [ ... bytecode_filename src_data src_len filename ] */

	src_data = (const char *) duk_require_pointer(ctx, -3);
	src_len = (duk_size_t) duk_require_uint(ctx, -2);

	if (src_data != NULL && src_len >= 1 && src_data[0] == (char) 0xbf) {
		/* Bytecode. */
		void *buf;
		buf = duk_push_fixed_buffer(ctx, src_len);
		memcpy(buf, (const void *) src_data, src_len);
		duk_load_function(ctx);
	} else {
		/* Source code. */
		comp_flags = DUK_COMPILE_SHEBANG;
		duk_compile_lstring_filename(ctx, comp_flags, src_data, src_len);
	}

	/* [ ... bytecode_filename src_data src_len function ] */

	/* Optional bytecode dump. */
	if (duk_is_string(ctx, -4)) {
		FILE *f;
		void *bc_ptr;
		duk_size_t bc_len;
		size_t wrote;
		char fnbuf[256];
		const char *filename;

		duk_dup_top(ctx);
		duk_dump_function(ctx);
		bc_ptr = duk_require_buffer_data(ctx, -1, &bc_len);
		filename = duk_require_string(ctx, -5);
		snprintf(fnbuf, sizeof(fnbuf), "%s", filename);
		fnbuf[sizeof(fnbuf) - 1] = (char) 0;

		f = fopen(fnbuf, "wb");
		if (!f) {
			(void) duk_generic_error(ctx, "failed to open bytecode output file");
		}
		wrote = fwrite(bc_ptr, 1, (size_t) bc_len, f);  /* XXX: handle partial writes */
		(void) fclose(f);
		if (wrote != bc_len) {
			(void) duk_generic_error(ctx, "failed to write all bytecode");
		}

		return 0;  /* duk_safe_call() cleans up */
	}

	duk_push_global_object(ctx);  /* 'this' binding */
	duk_call_method(ctx, 0);

	if (interactive_mode) {
		/*
		 *  In interactive mode, write to stdout so output won't
		 *  interleave as easily.
		 *
		 *  NOTE: the ToString() coercion may fail in some cases;
		 *  for instance, if you evaluate:
		 *
		 *    ( {valueOf: function() {return {}},
		 *       toString: function() {return {}}});
		 *
		 *  The error is:
		 *
		 *    TypeError: coercion to primitive failed
		 *            duk_api.c:1420
		 *
		 *  These are handled now by the caller which also has stack
		 *  trace printing support.  User code can print out errors
		 *  safely using duk_safe_to_string().
		 */

		duk_push_global_stash(ctx);
		duk_get_prop_string(ctx, -1, "dukFormat");
		duk_dup(ctx, -3);
		/* duk_call(ctx, 1);  /\* -> [ ... res stash formatted ] *\/ */

		fprintf(stdout, "= %s\n", duk_to_string(ctx, -1));
		fflush(stdout);
	} else {
		/* In non-interactive mode, success results are not written at all.
		 * It is important that the result value is not string coerced,
		 * as the string coercion may cause an error in some cases.
		 */
	}

	return 0;  /* duk_safe_call() cleans up */
}

static int handle_interactive(duk_context *ctx) {
	const char *prompt = "duk> ";
	char *buffer = NULL;
	int retval = 0;
	int rc;
	int got_eof = 0;

	buffer = (char *) malloc(LINEBUF_SIZE);
	if (!buffer) {
		fprintf(stderr, "failed to allocated a line buffer\n");
		fflush(stderr);
		retval = -1;
		goto done;
	}

	while (!got_eof) {
		size_t idx = 0;

		fwrite(prompt, 1, strlen(prompt), stdout);
		fflush(stdout);

		for (;;) {
			int c = fgetc(stdin);
			if (c == EOF) {
				got_eof = 1;
				break;
			} else if (c == '\n') {
				break;
			} else if (idx >= LINEBUF_SIZE) {
				fprintf(stderr, "line too long\n");
				fflush(stderr);
				retval = -1;
				goto done;
			} else {
				buffer[idx++] = (char) c;
			}
		}

		duk_push_pointer(ctx, (void *) buffer);
		duk_push_uint(ctx, (duk_uint_t) idx);
		duk_push_string(ctx, "input");

		interactive_mode = 1;  /* global */

		rc = duk_safe_call(ctx, wrapped_compile_execute, NULL /*udata*/, 3 /*nargs*/, 1 /*nret*/);

		if (rc != DUK_EXEC_SUCCESS) {
			/* in interactive mode, write to stdout */
			print_pop_error(ctx, stdout);
			retval = -1;  /* an error 'taints' the execution */
		} else {
			duk_pop(ctx);
		}
	}

 done:
	if (buffer) {
		free(buffer);
		buffer = NULL;
	}

	return retval;
}

/* Adapted from https://github.com/svaarala/duktape/blob/44ca54f726bfa651a7ab59286dd5c371dba2ddfc/extras/print-alert/duk_print_alert.c */

/* Faster, less churn, higher footprint option. */
static duk_ret_t duk__print_alert_helper(duk_context *ctx, FILE *fh) {
	duk_idx_t nargs;
	const duk_uint8_t *buf;
	duk_size_t sz_buf;
	const char nl = '\n';
	duk_uint8_t buf_stack[256];

	nargs = duk_get_top(ctx);

	/* If argument count is 1 and first argument is a buffer, write the buffer
	 * as raw data into the file without a newline; this allows exact control
	 * over stdout/stderr without an additional entrypoint (useful for now).
	 * Otherwise current print/alert semantics are to ToString() coerce
	 * arguments, join them with a single space, and append a newline.
	 */

	if (nargs == 1 && duk_is_buffer_data(ctx, 0)) {
		buf = (const duk_uint8_t *) duk_get_buffer_data(ctx, 0, &sz_buf);
	} else if (nargs > 0) {
		duk_idx_t i;
		duk_size_t sz_str;
		const duk_uint8_t *p_str;
		duk_uint8_t *p;

		sz_buf = (duk_size_t) nargs;  /* spaces (nargs - 1) + newline */
		for (i = 0; i < nargs; i++) {
			(void) duk_to_lstring(ctx, i, &sz_str);
			sz_buf += sz_str;
		}

		if (sz_buf <= sizeof(buf_stack)) {
			p = (duk_uint8_t *) buf_stack;
		} else {
			p = (duk_uint8_t *) duk_push_fixed_buffer(ctx, sz_buf);
		}

		buf = (const duk_uint8_t *) p;
		for (i = 0; i < nargs; i++) {
			p_str = (const duk_uint8_t *) duk_get_lstring(ctx, i, &sz_str);
			memcpy((void *) p, (const void *) p_str, sz_str);
			p += sz_str;
			*p++ = (duk_uint8_t) (i == nargs - 1 ? '\n' : ' ');
		}
	} else {
		buf = (const duk_uint8_t *) &nl;
		sz_buf = 1;
	}

	/* 'buf' contains the string to write, 'sz_buf' contains the length
	 * (which may be zero).
	 */

	if (sz_buf > 0) {
		fwrite((const void *) buf, 1, (size_t) sz_buf, fh);
		fflush(fh);
	}

	return 0;
}

static duk_ret_t duk__print(duk_context *ctx) {
	return duk__print_alert_helper(ctx, stdout);
}

static duk_ret_t duk__alert(duk_context *ctx) {
	return duk__print_alert_helper(ctx, stderr);
}

void duk_print_alert_init(duk_context *ctx, duk_uint_t flags) {
	(void) flags;  /* unused at the moment */

	/* XXX: use duk_def_prop_list(). */
	duk_push_global_object(ctx);
	duk_push_string(ctx, "print");
	duk_push_c_function(ctx, duk__print, DUK_VARARGS);
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
	duk_push_string(ctx, "alert");
	duk_push_c_function(ctx, duk__alert, DUK_VARARGS);
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
	duk_pop(ctx);
}
