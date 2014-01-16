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
#include "camluv_sem.h"

#if defined(CAMLUV_USE_CUMSTOM_OPERATIONS)
/**
 * TODO: we will use ocaml cumstom operations later to support
 * user-provided finalization, comparision, hashing.
 */
static void
camluv_sem_struct_finalize(value v)
{
}

static int
camluv_sem_struct_compare(value v1, value v2)
{
  camluv_sem_t *sem1 = camluv_sem_struct_val(v1);
  camluv_sem_t *sem2 = camluv_sem_struct_val(v2);
  if (sem1 == sem2) return 0;
  else if (sem1 < sem2) return -1;
  return 1;
}

static long
camluv_sem_struct_hash(value v)
{
  return (long)camluv_sem_struct_val(v);
}
#endif /* CAMLUV_USE_CUMSTOM_OPERATIONS */

static struct custom_operations camluv_sem_struct_ops = {
  "camluv.sem",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

static value
camluv_copy_sem(camluv_sem_t *camluv_sem)
{
  CAMLparam0();
  CAMLlocal1(sem);

  sem = caml_alloc_custom(&camluv_sem_struct_ops,
           sizeof(camluv_sem_t *), 0, 1);
  camluv_sem_struct_val(sem) = camluv_sem;

  CAMLreturn(sem);
}

static camluv_sem_t *
camluv_sem_new(void)
{
  camluv_sem_t *sem =
          (camluv_sem_t *)malloc(sizeof(camluv_sem_t));
  if (!sem) return NULL;

  return sem;
}

CAMLprim value
camluv_sem_init(value v)
{
  CAMLparam1(v);
  CAMLlocal1(sem);

  int rc = -1;
  camluv_sem_t *camluv_sem = camluv_sem_new();
  rc = uv_sem_init(&(camluv_sem->uv_sem), Int_val(v));
  if (rc != UV_OK) {
    // TODO: error handling.
  }
  camluv_sem->initialized = 1;
  sem = camluv_copy_sem(camluv_sem);

  CAMLreturn(sem);
}

CAMLprim value
camluv_sem_destroy(value sem)
{
  CAMLparam1(sem);

  camluv_sem_t *camluv_sem = camluv_sem_struct_val(sem);
  uv_sem_destroy(&(camluv_sem->uv_sem));

  CAMLreturn(Val_unit);
}

CAMLprim value
camluv_sem_post(value sem)
{
  CAMLparam1(sem);

  camluv_sem_t *camluv_sem = camluv_sem_struct_val(sem);
  uv_sem_post(&(camluv_sem->uv_sem));

  CAMLreturn(Val_unit);
}


CAMLprim value
camluv_sem_wait(value sem)
{
  CAMLparam1(sem);

  camluv_sem_t *camluv_sem = camluv_sem_struct_val(sem);
  uv_sem_wait(&(camluv_sem->uv_sem));

  CAMLreturn(Val_unit);
}

CAMLprim value
camluv_sem_trywait(value sem)
{
  CAMLparam1(sem);

  camluv_sem_t *camluv_sem = camluv_sem_struct_val(sem);
  uv_sem_trywait(&(camluv_sem->uv_sem));

  CAMLreturn(Val_unit);
}

