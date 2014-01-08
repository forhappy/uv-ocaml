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
#include "camluv_condition.h"

static void
camluv_condition_struct_finalize(value v)
{
}

static int
camluv_condition_struct_compare(value v1, value v2)
{
  camluv_condition_t *condition1 = camluv_condition_struct_val(v1);
  camluv_condition_t *condition2 = camluv_condition_struct_val(v2);
  if (condition1 == condition2) return 0;
  else if (condition1 < condition2) return -1;
  return 1;
}

static long
camluv_condition_struct_hash(value v)
{
  return (long)camluv_condition_struct_val(v);
}

static struct custom_operations camluv_condition_struct_ops = {
  "camluv.condition",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

static value
camluv_copy_condition(camluv_condition_t *camluv_condition)
{
  CAMLparam0();
  CAMLlocal1(condition);

  condition = caml_alloc_custom(&camluv_condition_struct_ops,
           sizeof(camluv_condition_t *), 0, 1);
  camluv_condition_struct_val(condition) = camluv_condition;

  CAMLreturn(condition);
}

static camluv_condition_t *
camluv_condition_new(void)
{
  camluv_condition_t *condition =
          (camluv_condition_t *)malloc(sizeof(camluv_condition_t));
  if (!condition) return NULL;

  return condition;
}

CAMLprim value
camluv_condition_init(value unit)
{
  camluv_condition_t *camluv_condition = camluv_condition_new();

  int rc = uv_cond_init(&(camluv_condition->uv_condition));
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  return camluv_copy_condition(camluv_condition);
}

CAMLprim value
camluv_condition_destroy(value condition)
{
  CAMLparam1(condition);

  camluv_condition_t *camluv_condition = camluv_condition_struct_val(condition);
  uv_cond_destroy(&(camluv_condition->uv_condition));

  return Val_unit;
}

CAMLprim value
camluv_condition_signal(value condition)
{
  CAMLparam1(condition);

  camluv_condition_t *camluv_condition = camluv_condition_struct_val(condition);
  uv_cond_signal(&(camluv_condition->uv_condition));

  return Val_unit;
}

CAMLprim value
camluv_condition_broadcast(value condition)
{
  CAMLparam1(condition);

  camluv_condition_t *camluv_condition = camluv_condition_struct_val(condition);
  uv_cond_broadcast(&(camluv_condition->uv_condition));

  return Val_unit;
}

CAMLprim value
camluv_condition_wait(value condition, value mutex)
{
  CAMLparam1(condition);

  camluv_condition_t *camluv_condition = camluv_condition_struct_val(condition);
  camluv_mutex_t *camluv_mutex = camluv_mutex_struct_val(mutex);
  uv_cond_wait(&(camluv_condition->uv_condition), &(camluv_mutex->uv_mutex));

  return Val_unit;
}

