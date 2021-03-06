/* Copyright (c) 2014 Fu Haiping <haipingf@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/fail.h>
#include <caml/callback.h>
#include <caml/custom.h>
#include <caml/signals.h>

#include <uv.h>

#include "camluv_err.h"
#include "camluv_once.h"

static void
camluv_once_struct_finalize(value v)
{
}

static int
camluv_once_struct_compare(value v1, value v2)
{
  camluv_once_t *once1 = camluv_once_struct_val(v1);
  camluv_once_t *once2 = camluv_once_struct_val(v2);
  if (once1 == once2) return 0;
  else if (once1 < once2) return -1;
  return 1;
}

static long
camluv_once_struct_hash(value v)
{
  return (long)camluv_once_struct_val(v);
}

static struct custom_operations camluv_once_struct_ops = {
  "camluv.once",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

static value
camluv_copy_once(camluv_once_t *camluv_once)
{
  CAMLparam0();
  CAMLlocal1(once);

  once = caml_alloc_custom(&camluv_once_struct_ops,
           sizeof(camluv_once_t *), 0, 1);
  camluv_once_struct_val(once) = camluv_once;

  CAMLreturn(once);
}

static void
camluv_once_cb(void)
{
  camluv_enter_callback();
  CAMLlocal2(once_cb, unit);

  camluv_once_t *camluv_once = (camluv_once_t *)arg;
  once_cb = camluv_once->once_cb;
  arg_value = camluv_once->arg;

  callback(once_cb, unit);

  camluv_leave_callback();
}

static camluv_once_t *
camluv_once_new(void)
{
  camluv_once_t *once =
          (camluv_once_t *)malloc(sizeof(camluv_once_t));
  if (!once) return NULL;

  return once;
}

CAMLprim value
camluv_once_init(value unit)
{
  CAMLparam1(unit);

  camluv_once_t *camluv_once = camluv_once_new();
  camluv_once->uv_once = UV_ONCE_INIT;
  camluv_once->initialized = 1;

  return camluv_copy_once(camluv_once);
}

CAMLprim value
camluv_once(value once, value once_cb)
{
  CAMLparam2(once, once_cb);

  camluv_once_t *camluv_once = camluv_once_struct_val(once);
  camluv_once->once_cb = once_cb;
  uv_once(&(camluv_once->uv_once), camluv_once_cb);

  return Val_unit;
}
