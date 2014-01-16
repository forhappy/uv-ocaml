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
#include "camluv_barrier.h"

#if defined(CAMLUV_USE_CUMSTOM_OPERATIONS)
/**
 * TODO: we will use ocaml cumstom operations later to support
 * user-provided finalization, comparision, hashing.
 */
static void
camluv_barrier_struct_finalize(value v)
{
}

static int
camluv_barrier_struct_compare(value v1, value v2)
{
  camluv_barrier_t *barrier1 = camluv_barrier_struct_val(v1);
  camluv_barrier_t *barrier2 = camluv_barrier_struct_val(v2);
  if (barrier1 == barrier2) return 0;
  else if (barrier1 < barrier2) return -1;
  return 1;
}

static long
camluv_barrier_struct_hash(value v)
{
  return (long)camluv_barrier_struct_val(v);
}
#endif /* CAMLUV_USE_CUMSTOM_OPERATIONS */

static struct custom_operations camluv_barrier_struct_ops = {
  "camluv.barrier",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

static value
camluv_copy_barrier(camluv_barrier_t *camluv_barrier)
{
  CAMLparam0();
  CAMLlocal1(barrier);

  barrier = caml_alloc_custom(&camluv_barrier_struct_ops,
           sizeof(camluv_barrier_t *), 0, 1);
  camluv_barrier_struct_val(barrier) = camluv_barrier;

  CAMLreturn(barrier);
}

static camluv_barrier_t *
camluv_barrier_new(void)
{
  camluv_barrier_t *barrier =
          (camluv_barrier_t *)malloc(sizeof(camluv_barrier_t));
  if (!barrier) return NULL;

  return barrier;
}

CAMLprim value
camluv_barrier_init(value count)
{
  CAMLparam1(count);
  CAMLlocal1(barrier);

  camluv_barrier_t *camluv_barrier = camluv_barrier_new();

  int rc = uv_barrier_init(&(camluv_barrier->uv_barrier), Int_val(count));
  if (rc != UV_OK) {
    // TODO: error handling.
  }
  camluv_barrier->initialized = 1;
  barrier = camluv_copy_barrier(camluv_barrier);

  CAMLreturn(barrier);
}

CAMLprim value
camluv_barrier_destroy(value barrier)
{
  CAMLparam1(barrier);

  camluv_barrier_t *camluv_barrier = camluv_barrier_struct_val(barrier);
  uv_barrier_destroy(&(camluv_barrier->uv_barrier));

  CAMLreturn(Val_unit);
}

CAMLprim value
camluv_barrier_wait(value barrier)
{
  CAMLparam1(barrier);

  camluv_barrier_t *camluv_barrier = camluv_barrier_struct_val(barrier);
  uv_barrier_wait(&(camluv_barrier->uv_barrier));

  CAMLreturn(Val_unit);
}

