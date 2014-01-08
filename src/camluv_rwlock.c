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
#include "camluv_rwlock.h"

static void
camluv_rwlock_struct_finalize(value v)
{
}

static int
camluv_rwlock_struct_compare(value v1, value v2)
{
  camluv_rwlock_t *rwlock1 = camluv_rwlock_struct_val(v1);
  camluv_rwlock_t *rwlock2 = camluv_rwlock_struct_val(v2);
  if (rwlock1 == rwlock2) return 0;
  else if (rwlock1 < rwlock2) return -1;
  return 1;
}

static long
camluv_rwlock_struct_hash(value v)
{
  return (long)camluv_rwlock_struct_val(v);
}

static struct custom_operations camluv_rwlock_struct_ops = {
  "camluv.rwlock",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

static value
camluv_copy_rwlock(camluv_rwlock_t *camluv_rwlock)
{
  CAMLparam0();
  CAMLlocal1(rwlock);

  rwlock = caml_alloc_custom(&camluv_rwlock_struct_ops,
           sizeof(camluv_rwlock_t *), 0, 1);
  camluv_rwlock_struct_val(rwlock) = camluv_rwlock;

  CAMLreturn(rwlock);
}

static camluv_rwlock_t *
camluv_rwlock_new(void)
{
  camluv_rwlock_t *rwlock =
          (camluv_rwlock_t *)malloc(sizeof(camluv_rwlock_t));
  if (!rwlock) return NULL;

  return rwlock;
}

CAMLprim value
camluv_rwlock_init(value unit)
{
  camluv_rwlock_t *camluv_rwlock = camluv_rwlock_new();

  int rc = uv_rwlock_init(&(camluv_rwlock->uv_rwlock));
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  return camluv_copy_rwlock(camluv_rwlock);
}

CAMLprim value
camluv_rwlock_destroy(value rwlock)
{
  CAMLparam1(rwlock);

  camluv_rwlock_t *camluv_rwlock = camluv_rwlock_struct_val(rwlock);
  uv_rwlock_destroy(&(camluv_rwlock->uv_rwlock));

  return Val_unit;
}

CAMLprim value
camluv_rwlock_rdlock(value rwlock)
{
  CAMLparam1(rwlock);

  camluv_rwlock_t *camluv_rwlock = camluv_rwlock_struct_val(rwlock);
  uv_rwlock_rdlock(&(camluv_rwlock->uv_rwlock));

  return Val_unit;
}

CAMLprim value
camluv_rwlock_tryrdlock(value rwlock)
{
  CAMLparam1(rwlock);

  camluv_rwlock_t *camluv_rwlock = camluv_rwlock_struct_val(rwlock);
  int rc = uv_rwlock_tryrdlock(&(camluv_rwlock->uv_rwlock));

  return camluv_errno_c2ml(rc);
}

CAMLprim value
camluv_rwlock_rdunlock(value rwlock)
{
  CAMLparam1(rwlock);

  camluv_rwlock_t *camluv_rwlock = camluv_rwlock_struct_val(rwlock);
  uv_rwlock_rdunlock(&(camluv_rwlock->uv_rwlock));

  return Val_unit;
}

CAMLprim value
camluv_rwlock_wrlock(value rwlock)
{
  CAMLparam1(rwlock);

  camluv_rwlock_t *camluv_rwlock = camluv_rwlock_struct_val(rwlock);
  uv_rwlock_wrlock(&(camluv_rwlock->uv_rwlock));

  return Val_unit;
}

CAMLprim value
camluv_rwlock_trywrlock(value rwlock)
{
  CAMLparam1(rwlock);

  camluv_rwlock_t *camluv_rwlock = camluv_rwlock_struct_val(rwlock);
  int rc = uv_rwlock_trywrlock(&(camluv_rwlock->uv_rwlock));

  return camluv_errno_c2ml(rc);
}

CAMLprim value
camluv_rwlock_wrunlock(value rwlock)
{
  CAMLparam1(rwlock);

  camluv_rwlock_t *camluv_rwlock = camluv_rwlock_struct_val(rwlock);
  uv_rwlock_wrunlock(&(camluv_rwlock->uv_rwlock));

  return Val_unit;
}

