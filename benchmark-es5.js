"use strict";

function _createForOfIteratorHelper(o, allowArrayLike) { var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"]; if (!it) { if (Array.isArray(o) || (it = _unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") { if (it) o = it; var i = 0; var F = function F() {}; return { s: F, n: function n() { if (i >= o.length) return { done: true }; return { done: false, value: o[i++] }; }, e: function e(_e) { throw _e; }, f: F }; } throw new TypeError("Invalid attempt to iterate non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method."); } var normalCompletion = true, didErr = false, err; return { s: function s() { it = it.call(o); }, n: function n() { var step = it.next(); normalCompletion = step.done; return step; }, e: function e(_e2) { didErr = true; err = _e2; }, f: function f() { try { if (!normalCompletion && it["return"] != null) it["return"](); } finally { if (didErr) throw err; } } }; }
function _unsupportedIterableToArray(o, minLen) { if (!o) return; if (typeof o === "string") return _arrayLikeToArray(o, minLen); var n = Object.prototype.toString.call(o).slice(8, -1); if (n === "Object" && o.constructor) n = o.constructor.name; if (n === "Map" || n === "Set") return Array.from(o); if (n === "Arguments" || /^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(n)) return _arrayLikeToArray(o, minLen); }
function _arrayLikeToArray(arr, len) { if (len == null || len > arr.length) len = arr.length; for (var i = 0, arr2 = new Array(len); i < len; i++) arr2[i] = arr[i]; return arr2; }
var MAX_ITERATE = 1;
function bench(bench) {
  var value = undefined;
  if (bench.bench_prepare !== undefined) {
    value = bench.bench_prepare();
  }
  var start_cycles = CKB.current_cycles();
  bench.func(MAX_ITERATE, value);
  var cost_cycles = CKB.current_cycles() - start_cycles;
  CKB.debug("".concat(bench.name, ": ").concat(Math.round(cost_cycles / 1024), " K cycles"));
}
var global_res; /* to be sure the code is not optimized */

function empty_loop(n) {
  var j;
  for (j = 0; j < n; j++) {}
  return n;
}
function prop_read(n) {
  var obj, sum, j;
  obj = {
    a: 1,
    b: 2,
    c: 3,
    d: 4
  };
  sum = 0;
  for (j = 0; j < n; j++) {
    sum += obj.a;
    sum += obj.b;
    sum += obj.c;
    sum += obj.d;
  }
  global_res = sum;
  return n * 4;
}
function prop_write(n) {
  var obj, j;
  obj = {
    a: 1,
    b: 2,
    c: 3,
    d: 4
  };
  for (j = 0; j < n; j++) {
    obj.a = j;
    obj.b = j;
    obj.c = j;
    obj.d = j;
  }
  return n * 4;
}
function prop_create(n) {
  var obj, j;
  for (j = 0; j < n; j++) {
    obj = new Object();
    obj.a = 1;
    obj.b = 2;
    obj.c = 3;
    obj.d = 4;
  }
  return n * 4;
}
function prop_delete(n) {
  var obj, j;
  obj = {};
  for (j = 0; j < n; j++) {
    obj[j] = 1;
  }
  for (j = 0; j < n; j++) {
    delete obj[j];
  }
  return n;
}
function array_read(n) {
  var tab, len, sum, i, j;
  tab = [];
  len = 10;
  for (i = 0; i < len; i++) tab[i] = i;
  sum = 0;
  for (j = 0; j < n; j++) {
    sum += tab[0];
    sum += tab[1];
    sum += tab[2];
    sum += tab[3];
    sum += tab[4];
    sum += tab[5];
    sum += tab[6];
    sum += tab[7];
    sum += tab[8];
    sum += tab[9];
  }
  global_res = sum;
  return len * n;
}
function array_write(n) {
  var tab, len, i, j;
  tab = [];
  len = 10;
  for (i = 0; i < len; i++) tab[i] = i;
  for (j = 0; j < n; j++) {
    tab[0] = j;
    tab[1] = j;
    tab[2] = j;
    tab[3] = j;
    tab[4] = j;
    tab[5] = j;
    tab[6] = j;
    tab[7] = j;
    tab[8] = j;
    tab[9] = j;
  }
  return len * n;
}
function array_prop_create(n) {
  var tab, i, j, len;
  len = 1000;
  for (j = 0; j < n; j++) {
    tab = [];
    for (i = 0; i < len; i++) tab[i] = i;
  }
  return len * n;
}
function array_length_decr(n) {
  var tab, i, j, len;
  len = 1000;
  tab = [];
  for (i = 0; i < len; i++) tab[i] = i;
  for (j = 0; j < n; j++) {
    for (i = len - 1; i >= 0; i--) tab.length = i;
  }
  return len * n;
}
function array_hole_length_decr(n) {
  var tab, i, j, len;
  len = 1000;
  tab = [];
  for (i = 0; i < len; i++) {
    if (i != 3) tab[i] = i;
  }
  for (j = 0; j < n; j++) {
    for (i = len - 1; i >= 0; i--) tab.length = i;
  }
  return len * n;
}
function array_push(n) {
  var tab, i, j, len;
  len = 500;
  for (j = 0; j < n; j++) {
    tab = [];
    for (i = 0; i < len; i++) tab.push(i);
  }
  return len * n;
}
function array_pop(n) {
  var tab, i, j, len, sum;
  len = 500;
  for (j = 0; j < n; j++) {
    tab = [];
    for (i = 0; i < len; i++) tab[i] = i;
    sum = 0;
    for (i = 0; i < len; i++) sum += tab.pop();
    global_res = sum;
  }
  return len * n;
}
function typed_array_read(n) {
  var tab, len, sum, i, j;
  len = 10;
  tab = new Int32Array(len);
  for (i = 0; i < len; i++) tab[i] = i;
  sum = 0;
  for (j = 0; j < n; j++) {
    sum += tab[0];
    sum += tab[1];
    sum += tab[2];
    sum += tab[3];
    sum += tab[4];
    sum += tab[5];
    sum += tab[6];
    sum += tab[7];
    sum += tab[8];
    sum += tab[9];
  }
  global_res = sum;
  return len * n;
}
function typed_array_write(n) {
  var tab, len, i, j;
  len = 10;
  tab = new Int32Array(len);
  for (i = 0; i < len; i++) tab[i] = i;
  for (j = 0; j < n; j++) {
    tab[0] = j;
    tab[1] = j;
    tab[2] = j;
    tab[3] = j;
    tab[4] = j;
    tab[5] = j;
    tab[6] = j;
    tab[7] = j;
    tab[8] = j;
    tab[9] = j;
  }
  return len * n;
}
var global_var0;
function global_read(n) {
  var sum, j;
  global_var0 = 0;
  sum = 0;
  for (j = 0; j < n; j++) {
    sum += global_var0;
    sum += global_var0;
    sum += global_var0;
    sum += global_var0;
  }
  global_res = sum;
  return n * 4;
}
var global_write = (1, eval)("(function global_write(n)\n           {\n               var j;\n               for(j = 0; j < n; j++) {\n                   global_var0 = j;\n                   global_var0 = j;\n                   global_var0 = j;\n                   global_var0 = j;\n               }\n               return n * 4;\n           })");
function global_write_strict(n) {
  var j;
  for (j = 0; j < n; j++) {
    global_var0 = j;
    global_var0 = j;
    global_var0 = j;
    global_var0 = j;
  }
  return n * 4;
}
function local_destruct(n) {
  var j, v1, v2, v3, v4;
  var array = [1, 2, 3, 4, 5];
  var o = {
    a: 1,
    b: 2,
    c: 3,
    d: 4
  };
  var a, b, c, d;
  for (j = 0; j < n; j++) {
    v1 = array[0];
    v2 = array[1];
    v3 = array[3];
    v4 = array.slice(4);
    a = o.a;
    b = o.b;
    c = o.c;
    d = o.d;
    a = o.a;
    b = o.b;
    c = o.c;
    d = o.d;
  }
  return n * 12;
}
var global_v1, global_v2, global_v3, global_v4;
var global_a, global_b, global_c, global_d;
var global_destruct = (1, eval)("(function global_destruct(n) {\n  var j, v1, v2, v3, v4;\n  var array = [1, 2, 3, 4, 5];\n  var o = {\n    a: 1,\n    b: 2,\n    c: 3,\n    d: 4\n  };\n  var a, b, c, d;\n  for (j = 0; j < n; j++) {\n    global_v1 = array[0];\n    global_v2 = array[1];\n    global_v3 = array[3];\n    global_v4 = array.slice(4);\n    global_a = o.a;\n    global_b = o.b;\n    global_c = o.c;\n    global_d = o.d;\n  }\n  return n * 8;\n})");
function global_destruct_strict(n) {
  var j, v1, v2, v3, v4;
  var array = [1, 2, 3, 4, 5];
  var o = {
    a: 1,
    b: 2,
    c: 3,
    d: 4
  };
  var a, b, c, d;
  for (j = 0; j < n; j++) {
    global_v1 = array[0];
    global_v2 = array[1];
    global_v3 = array[3];
    global_v4 = array.slice(4);
    global_a = o.a;
    global_b = o.b;
    global_c = o.c;
    global_d = o.d;
  }
  return n * 8;
}
function func_call(n) {
  function f(a) {
    return 1;
  }
  var j, sum;
  sum = 0;
  for (j = 0; j < n; j++) {
    sum += f(j);
    sum += f(j);
    sum += f(j);
    sum += f(j);
  }
  global_res = sum;
  return n * 4;
}
function closure_var(n) {
  function f(a) {
    sum++;
  }
  var j, sum;
  sum = 0;
  for (j = 0; j < n; j++) {
    f(j);
    f(j);
    f(j);
    f(j);
  }
  global_res = sum;
  return n * 4;
}
function int_arith(n) {
  var i, j, sum;
  global_res = 0;
  for (j = 0; j < n; j++) {
    sum = 0;
    for (i = 0; i < 1000; i++) {
      sum += i * i;
    }
    global_res += sum;
  }
  return n * 1000;
}
function float_arith(n) {
  var i, j, sum, a, incr, a0;
  global_res = 0;
  a0 = 0.1;
  incr = 1.1;
  for (j = 0; j < n; j++) {
    sum = 0;
    a = a0;
    for (i = 0; i < 1000; i++) {
      sum += a * a;
      a += incr;
    }
    global_res += sum;
  }
  return n * 1000;
}
function bigfloat_arith(n) {
  var i, j, sum, a, incr, a0;
  global_res = 0;
  a0 = BigFloat('0.1');
  incr = BigFloat('1.1');
  for (j = 0; j < n; j++) {
    sum = 0;
    a = a0;
    for (i = 0; i < 1000; i++) {
      sum += a * a;
      a += incr;
    }
    global_res += sum;
  }
  return n * 1000;
}
function float256_arith(n) {
  return BigFloatEnv.setPrec(bigfloat_arith.bind(null, n), 237, 19);
}
function bigint_arith(n, bits) {
  var i, j, sum, a, incr, a0, sum0;
  sum0 = global_res = BigInt(0);
  a0 = BigInt(1) << BigInt(Math.floor((bits - 10) * 0.5));
  incr = BigInt(1);
  for (j = 0; j < n; j++) {
    sum = sum0;
    a = a0;
    for (i = 0; i < 1000; i++) {
      sum += a * a;
      a += incr;
    }
    global_res += sum;
  }
  return n * 1000;
}
function bigint64_arith(n) {
  return bigint_arith(n, 64);
}
function bigint256_arith(n) {
  return bigint_arith(n, 256);
}
function set_collection_add(n) {
  var s,
    i,
    j,
    len = 100;
  s = new Set();
  for (j = 0; j < n; j++) {
    for (i = 0; i < len; i++) {
      s.add(String(i), i);
    }
    for (i = 0; i < len; i++) {
      if (!s.has(String(i))) throw Error('bug in Set');
    }
  }
  return n * len;
}
function array_for(n) {
  var r, i, j, sum;
  r = [];
  for (i = 0; i < 100; i++) r[i] = i;
  for (j = 0; j < n; j++) {
    sum = 0;
    for (i = 0; i < 100; i++) {
      sum += r[i];
    }
    global_res = sum;
  }
  return n * 100;
}
function array_for_in(n) {
  var r, i, j, sum;
  r = [];
  for (i = 0; i < 100; i++) r[i] = i;
  for (j = 0; j < n; j++) {
    sum = 0;
    for (i in r) {
      sum += r[i];
    }
    global_res = sum;
  }
  return n * 100;
}
function array_for_of(n) {
  var r, i, j, sum;
  r = [];
  for (i = 0; i < 100; i++) r[i] = i;
  for (j = 0; j < n; j++) {
    sum = 0;
    var _iterator = _createForOfIteratorHelper(r),
      _step;
    try {
      for (_iterator.s(); !(_step = _iterator.n()).done;) {
        i = _step.value;
        sum += i;
      }
    } catch (err) {
      _iterator.e(err);
    } finally {
      _iterator.f();
    }
    global_res = sum;
  }
  return n * 100;
}
function math_min(n) {
  var i, j, r;
  r = 0;
  for (j = 0; j < n; j++) {
    for (i = 0; i < 1000; i++) r = Math.min(i, 500);
    global_res = r;
  }
  return n * 1000;
}

/* incremental string contruction as local var */
function string_build1(n) {
  var i, j, r;
  r = '';
  for (j = 0; j < n; j++) {
    for (i = 0; i < 100; i++) r += 'x';
    global_res = r;
  }
  return n * 100;
}

/* incremental string contruction as arg */
function string_build2(n, r) {
  var i, j;
  r = '';
  for (j = 0; j < n; j++) {
    for (i = 0; i < 100; i++) r += 'x';
    global_res = r;
  }
  return n * 100;
}

/* incremental string contruction by prepending */
function string_build3(n, r) {
  var i, j;
  r = '';
  for (j = 0; j < n; j++) {
    for (i = 0; i < 100; i++) r = 'x' + r;
    global_res = r;
  }
  return n * 100;
}

/* incremental string contruction with multiple reference */
function string_build4(n) {
  var i, j, r, s;
  r = '';
  for (j = 0; j < n; j++) {
    for (i = 0; i < 100; i++) {
      s = r;
      r += 'x';
    }
    global_res = r;
  }
  return n * 100;
}
function int_to_string(n) {
  var s, r, j;
  r = 0;
  for (j = 0; j < n; j++) {
    s = (j + 1).toString();
  }
  return n;
}
function float_to_string(n) {
  var s, r, j;
  r = 0;
  for (j = 0; j < n; j++) {
    s = (j + 0.1).toString();
  }
  return n;
}
function string_to_int(n) {
  var s, r, j;
  r = 0;
  s = '12345';
  r = 0;
  for (j = 0; j < n; j++) {
    r += s | 0;
  }
  global_res = r;
  return n;
}
function string_to_float(n) {
  var s, r, j;
  r = 0;
  s = '12345.6';
  r = 0;
  for (j = 0; j < n; j++) {
    r -= s;
  }
  global_res = r;
  return n;
}
function sort_prepare(_) {
  var originalArray = [1, 3, 5, 3, 1];
  return Array.from({
    length: 1000
  }, function () {
    return originalArray;
  }).flat();
}
function sort_bench(_, arr) {
  arr.sort();
}
function json_parse_bench(_) {
  var json = "{\n        \"name\": \"John Doe\",\n        \"age\": 30,\n        \"isStudent\": false,\n        \"grades\": [80, 90, 95],\n        \"address\": {\n          \"street\": \"123 ABC Street\",\n          \"city\": \"Example City\",\n          \"country\": \"USA\"\n        },\n        \"height\": 5.9,\n        \"skills\": [\"JavaScript\", \"Python\", \"HTML\", \"CSS\"],\n        \"isActive\": true\n    }";
  var obj = JSON.parse(json);
  CKB.assert(obj.name == 'John Doe');
}
function json_stringify_bench(_) {
  var json = {
    'name': 'John Doe',
    'age': 30,
    'isStudent': false,
    'grades': [80, 90, 95],
    'address': {
      'street': '123 ABC Street',
      'city': 'Example City',
      'country': 'USA'
    },
    'height': 5.9,
    'skills': ['JavaScript', 'Python', 'HTML', 'CSS'],
    'isActive': true
  };
  var str = JSON.stringify(json);
  CKB.assert(str != undefined);
}
var BENCH_LIST = [{
  'name': 'empty_loop',
  'func': empty_loop
}, {
  'name': 'prop_read',
  'func': prop_read
}, {
  'name': 'prop_write',
  'func': prop_write
}, {
  'name': 'prop_create',
  'func': prop_create
}, {
  'name': 'prop_delete',
  'func': prop_delete
}, {
  'name': 'array_read',
  'func': array_read
}, {
  'name': 'array_write',
  'func': array_write
}, {
  'name': 'array_prop_create',
  'func': array_prop_create
}, {
  'name': 'array_length_decr',
  'func': array_length_decr
}, {
  'name': 'array_hole_length_decr',
  'func': array_hole_length_decr
}, {
  'name': 'array_push',
  'func': array_push
}, {
  'name': 'array_pop',
  'func': array_pop
}, {
  'name': 'typed_array_read',
  'func': typed_array_read
}, {
  'name': 'typed_array_write',
  'func': typed_array_write
}, {
  'name': 'global_read',
  'func': global_read
}, {
  'name': 'global_write',
  'func': global_write
}, {
  'name': 'global_write_strict',
  'func': global_write_strict
}, {
  'name': 'local_destruct',
  'func': local_destruct
}, {
  'name': 'global_destruct',
  'func': global_destruct
}, {
  'name': 'global_destruct_strict',
  'func': global_destruct_strict
}, {
  'name': 'func_call',
  'func': closure_var
}, {
  'name': 'int_arith',
  'func': float_arith
}, {
  'name': 'array_for',
  'func': array_for
}, {
  'name': 'array_for_in',
  'func': array_for_in
}, {
  'name': 'array_for_of',
  'func': array_for_of
}, {
  'name': 'math_min',
  'func': math_min
}, {
  'name': 'string_build1',
  'func': string_build1
}, {
  'name': 'string_build2',
  'func': string_build2
}, {
  'name': 'string_build3',
  'func': string_build3
}, {
  'name': 'string_build4',
  'func': string_build4
}, {
  'name': 'int_to_string',
  'func': int_to_string
}, {
  'name': 'float_to_string',
  'func': float_to_string
}, {
  'name': 'string_to_int',
  'func': string_to_int
}, {
  'name': 'string_to_float',
  'func': string_to_float
}, {
  'name': 'json_parse_bench',
  'func': json_parse_bench
}, {
  'name': 'json_stringify_bench',
  'func': json_stringify_bench
}];
function main() {
  CKB.debug("Benchmark started. Every test may iterate ".concat(MAX_ITERATE, " internally ..."));
  for (var _i = 0, _BENCH_LIST = BENCH_LIST; _i < _BENCH_LIST.length; _i++) {
    var entry = _BENCH_LIST[_i];
    bench(entry);
  }
}
main();
