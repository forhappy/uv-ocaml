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

#include "camluv.h"
#include "camluv_handle.h"
#include "camluv_loop.h"
#include "camluv_mutex.h"

static void
camluv_mutex_struct_finalize(value v)
{
}

static int
camluv_mutex_struct_compare(value v1, value v2)
{
  camluv_mutex_t *mutex1 = camluv_mutex_struct_val(v1);
  camluv_mutex_t *mutex2 = camluv_mutex_struct_val(v2);
  if (mutex1 == mutex2) return 0;
  else if (mutex1 < mutex2) return -1;
  return 1;
}

static long
camluv_mutex_struct_hash(value v)
{
  return (long)camluv_mutex_struct_val(v);
}

static struct custom_operations camluv_mutex_struct_ops = {
  "camluv.mutex",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

static value
camluv_copy_mutex(camluv_mutex_t *camluv_mutex)
{
  CAMLparam0();
  CAMLlocal1(mutex);

  mutex = caml_alloc_custom(&camluv_mutex_struct_ops,
           sizeof(camluv_mutex_t *), 0, 1);
  camluv_mutex_struct_val(mutex) = camluv_mutex;

  CAMLreturn(mutex);
}

static camluv_mutex_t *
camluv_mutex_new(void)
{
  camluv_mutex_t *mutex =
          (camluv_mutex_t *)malloc(sizeof(camluv_mutex_t));
  if (!mutex) return NULL;

  return mutex;
}

CAMLprim value
camluv_mutex_init(value unit)
{
  camluv_mutex_t *camluv_mutex = camluv_mutex_new();

  int rc = uv_mutex_init(&(camluv_mutex->uv_mutex));
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  return camluv_copy_mutex(camluv_mutex);
}

CAMLprim value
camluv_mutex_destroy(value mutex)
{
  CAMLparam1(mutex);

  camluv_mutex_t *camluv_mutex = camluv_mutex_struct_val(mutex);
  uv_mutex_destroy(&(camluv_mutex->uv_mutex));

  return Val_unit;
}

CAMLprim value
camluv_mutex_lock(value mutex)
{
  CAMLparam1(mutex);

  camluv_mutex_t *camluv_mutex = camluv_mutex_struct_val(mutex);
  uv_mutex_lock(&(camluv_mutex->uv_mutex));

  return Val_unit;
}

CAMLprim value
camluv_mutex_trylock(value mutex)
{
  CAMLparam1(mutex);

  camluv_mutex_t *camluv_mutex = camluv_mutex_struct_val(mutex);
  int rc = uv_mutex_trylock(&(camluv_mutex->uv_mutex));

  return camluv_errno_c2ml(rc);
}

CAMLprim value
camluv_mutex_unlock(value mutex)
{
  CAMLparam1(mutex);

  camluv_mutex_t *camluv_mutex = camluv_mutex_struct_val(mutex);
  uv_mutex_unlock(&(camluv_mutex->uv_mutex));

  return Val_unit;
}

