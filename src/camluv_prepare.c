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
#include "camluv_prepare.h"

static void
camluv_prepare_struct_finalize(value v)
{
}

static int
camluv_prepare_struct_compare(value v1, value v2)
{
  camluv_prepare_t *prepare1 = camluv_prepare_struct_val(v1);
  camluv_prepare_t *prepare2 = camluv_prepare_struct_val(v2);
  if (prepare1 == prepare2) return 0;
  else if (prepare1 < prepare2) return -1;
  return 1;
}

static long
camluv_prepare_struct_hash(value v)
{
  return (long)camluv_prepare_struct_val(v);
}

static struct custom_operations camluv_prepare_struct_ops = {
  "camluv.prepare",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

static value
camluv_copy_prepare(camluv_prepare_t *camluv_prepare)
{
  CAMLparam0();
  CAMLlocal1(prepare);

  prepare = caml_alloc_custom(&camluv_prepare_struct_ops,
           sizeof(camluv_prepare_t *), 0, 1);
  camluv_prepare_struct_val(prepare) = camluv_prepare;

  CAMLreturn(prepare);
}

static void
camluv_prepare_cb(uv_prepare_t* uv_handle, int status)
{
  camluv_enter_callback();

  CAMLlocal3(prepare_cb, camluv_handle, camluv_status);

  camluv_prepare_t *camluv_prepare =(camluv_prepare_t *)(uv_handle->data);
  prepare_cb = camluv_prepare->prepare_cb;
  camluv_handle = camluv_copy_prepare(camluv_prepare);
  camluv_status = Val_int(status);

  callback2(prepare_cb, camluv_handle, camluv_status);

  camluv_leave_callback();
}

static camluv_prepare_t *
camluv_prepare_new(void)
{
  camluv_prepare_t *prepare = (camluv_prepare_t *)malloc(sizeof(camluv_prepare_t));
  if (!prepare) return NULL;

  return prepare;
}

CAMLprim value
camluv_prepare_init(value loop)
{
  CAMLparam1(loop);

  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  camluv_prepare_t *camluv_prepare = camluv_prepare_new();
  uv_prepare_init(camluv_loop->uv_loop, &(camluv_prepare->uv_prepare));

  (camluv_prepare->uv_prepare).data = camluv_prepare;

  ((camluv_handle_t *)camluv_prepare)->uv_handle =
          (uv_handle_t *)&(camluv_prepare->uv_prepare);
  camluv_init_handle_with_loop((camluv_handle_t *)
                               (&(camluv_prepare->camluv_handle)),
                               camluv_loop);

  return camluv_copy_prepare(camluv_prepare);
}

CAMLprim value
camluv_prepare_start(value prepare, value prepare_cb)
{
  CAMLparam2(prepare, prepare_cb);
  int rc = -1;

  camluv_prepare_t *camluv_prepare = camluv_prepare_struct_val(prepare);
  camluv_prepare->prepare_cb = prepare_cb;
  rc = uv_prepare_start(&(camluv_prepare->uv_prepare), camluv_prepare_cb);

  return Val_int(rc);
}

CAMLprim value
camluv_prepare_stop(value prepare)
{
  CAMLparam1(prepare);
  int rc = -1;

  camluv_prepare_t *camluv_prepare = camluv_prepare_struct_val(prepare);
  rc = uv_prepare_stop(&(camluv_prepare->uv_prepare));

  return Val_int(rc);
}

