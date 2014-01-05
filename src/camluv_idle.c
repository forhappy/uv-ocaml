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
#include "camluv_idle.h"

static void
camluv_idle_struct_finalize(value v)
{
}

static int
camluv_idle_struct_compare(value v1, value v2)
{
  camluv_idle_t *idle1 = camluv_idle_struct_val(v1);
  camluv_idle_t *idle2 = camluv_idle_struct_val(v2);
  if (idle1 == idle2) return 0;
  else if (idle1 < idle2) return -1;
  return 1;
}

static long
camluv_idle_struct_hash(value v)
{
  return (long)camluv_idle_struct_val(v);
}

static struct custom_operations camluv_idle_struct_ops = {
  "camluv.idle",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

static value
camluv_copy_idle(camluv_idle_t *camluv_idle)
{
  CAMLparam0();
  CAMLlocal1(idle);

  idle = caml_alloc_custom(&camluv_idle_struct_ops,
           sizeof(camluv_idle_t *), 0, 1);
  camluv_idle_struct_val(idle) = camluv_idle;

  CAMLreturn(idle);
}

static void
camluv_idle_cb(uv_idle_t* uv_handle, int status)
{
  camluv_enter_callback();

  CAMLlocal3(idle_cb, camluv_handle, camluv_status);

  camluv_idle_t *camluv_idle =(camluv_idle_t *)(uv_handle->data);
  idle_cb = camluv_idle->idle_cb;
  camluv_handle = camluv_copy_idle(camluv_idle);
  camluv_status = Val_int(status);

  callback2(idle_cb, camluv_handle, camluv_status);

  camluv_leave_callback();
}

static camluv_idle_t *
camluv_idle_new(void)
{
  camluv_idle_t *idle = (camluv_idle_t *)malloc(sizeof(camluv_idle_t));
  if (!idle) return NULL;

  return idle;
}

CAMLprim value
camluv_idle_init(value loop)
{
  CAMLparam1(loop);

  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  camluv_idle_t *camluv_idle = camluv_idle_new();
  uv_idle_init(camluv_loop->uv_loop, &(camluv_idle->uv_idle));

  (camluv_idle->uv_idle).data = camluv_idle;

  ((camluv_handle_t *)camluv_idle)->uv_handle =
                              (uv_handle_t *)&(camluv_idle->uv_idle);
  camluv_init_handle_with_loop((camluv_handle_t *)
                               (&(camluv_idle->camluv_handle)),
                               camluv_loop);

  return camluv_copy_idle(camluv_idle);
}

CAMLprim value
camluv_idle_start(value idle, value idle_cb)
{
  CAMLparam2(idle, idle_cb);
  int rc = -1;

  camluv_idle_t *camluv_idle = camluv_idle_struct_val(idle);
  camluv_idle->idle_cb = idle_cb;
  rc = uv_idle_start(&(camluv_idle->uv_idle), camluv_idle_cb);

  return Val_int(rc);
}

CAMLprim value
camluv_idle_stop(value idle)
{
  CAMLparam1(idle);
  int rc = -1;

  camluv_idle_t *camluv_idle = camluv_idle_struct_val(idle);
  rc = uv_idle_stop(&(camluv_idle->uv_idle));

  return Val_int(rc);

}

