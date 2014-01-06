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
#include "camluv_async.h"

static void
camluv_async_struct_finalize(value v)
{
}

static int
camluv_async_struct_compare(value v1, value v2)
{
  camluv_async_t *async1 = camluv_async_struct_val(v1);
  camluv_async_t *async2 = camluv_async_struct_val(v2);
  if (async1 == async2) return 0;
  else if (async1 < async2) return -1;
  return 1;
}

static long
camluv_async_struct_hash(value v)
{
  return (long)camluv_async_struct_val(v);
}

static struct custom_operations camluv_async_struct_ops = {
  "camluv.async",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

static value
camluv_copy_async(camluv_async_t *camluv_async)
{
  CAMLparam0();
  CAMLlocal1(async);

  async = caml_alloc_custom(&camluv_async_struct_ops,
           sizeof(camluv_async_t *), 0, 1);
  camluv_async_struct_val(async) = camluv_async;

  CAMLreturn(async);
}

static void
camluv_async_cb(uv_async_t* uv_handle, int status)
{
  camluv_enter_callback();

  CAMLlocal3(async_cb, camluv_handle, camluv_status);

  camluv_async_t *camluv_async =(camluv_async_t *)(uv_handle->data);
  async_cb = camluv_async->async_cb;
  camluv_handle = camluv_copy_async(camluv_async);
  camluv_status = Val_int(status);

  callback2(async_cb, camluv_handle, camluv_status);

  camluv_leave_callback();
}

static camluv_async_t *
camluv_async_new(void)
{
  camluv_async_t *async = (camluv_async_t *)malloc(sizeof(camluv_async_t));
  if (!async) return NULL;

  return async;
}

CAMLprim value
camluv_async_init(value loop, value async_cb)
{
  CAMLparam2(loop, async_cb);

  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  camluv_async_t *camluv_async = camluv_async_new();
  (camluv_async->uv_async).data = camluv_async;
  camluv_async->async_cb = async_cb;
  ((camluv_handle_t *)camluv_async)->uv_handle =
                               (uv_handle_t *)&(camluv_async->uv_async);
  camluv_init_handle_with_loop((camluv_handle_t *)
                               (&(camluv_async->camluv_handle)),
                               camluv_loop);

  uv_async_init(camluv_loop->uv_loop, &(camluv_async->uv_async), camluv_async_cb);

  return camluv_copy_async(camluv_async);
}

CAMLprim value
camluv_async_stop(value async)
{
  CAMLparam1(async);
  int rc = -1;

  camluv_async_t *camluv_async = camluv_async_struct_val(async);
  rc = uv_async_send(&(camluv_async->uv_async));

  return Val_int(rc);
}

