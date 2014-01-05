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
camluv_idle_cb(uv_handle_t* handle, int status)
{
  camluv_enter_callback();
  camluv_leave_callback();
}

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

static value
camluv_copy_idle2(uv_idle_t *uv_idle, int is_default)
{
  CAMLparam0();
  CAMLlocal1(idle);

  camluv_idle_t *camluv_idle = (camluv_idle_t *)
      malloc(sizeof(camluv_idle_t));
  uv_idle->data = camluv_idle;
  camluv_idle->uv_idle = uv_idle;
  camluv_idle->is_default = is_default;

  idle = caml_alloc_custom(&camluv_idle_struct_ops,
          sizeof(camluv_idle_t *), 0, 1);
  camluv_idle_struct_val(idle) = camluv_idle;

  CAMLreturn(idle);
}

static int
camluv_init_idle(camluv_idle_t *idle, int is_default)
{
  uv_idle_t *uv_idle;

  uv_idle = NULL;

  if (is_default) {
    uv_idle = uv_default_idle();
  } else {
    uv_idle = uv_idle_new();
  }

  if (!uv_idle) {
      return -1;
  }
  uv_idle->data = idle;

  idle->uv_idle = uv_idle;
  idle->is_default = is_default;

  return 0;
}

static camluv_idle_t*
camluv_new_idle(int is_default)
{
  if (is_default) {
      default_idle = (camluv_idle_t *)malloc(sizeof(camluv_idle_t));
      if (!default_idle) return NULL;
      if (camluv_init_idle(default_idle, is_default) != 0) return NULL;
      return default_idle;
  } else {
      camluv_idle_t *camluv_idle =
        (camluv_idle_t *)malloc(sizeof(camluv_idle_t));
      if (!camluv_idle) return NULL;
      if (camluv_init_idle(camluv_idle, is_default) != 0) return NULL;
      return camluv_idle;
  }
}

CAMLprim value
camluv_idle_new(value unit)
{
  camluv_idle_t *camluv_idle = camluv_new_idle(0);
  if (!camluv_idle) return Val_unit;

  return camluv_copy_idle(camluv_idle);
}

CAMLprim value
camluv_idle_default(value unit)
{
  camluv_idle_t *camluv_idle = camluv_new_idle(1);
  if (!camluv_idle) return Val_unit;

  return camluv_copy_idle(camluv_idle);
}

CAMLprim value
camluv_idle_delete(value idle)
{
  CAMLparam1(idle);

  camluv_idle_t *camluv_idle = camluv_idle_struct_val(idle);
  if (camluv_idle->uv_idle != NULL) {
    camluv_idle->uv_idle->data = NULL;
    uv_idle_delete(camluv_idle->uv_idle);
  }

  return Val_unit;
}

CAMLprim value
camluv_idle_run(value idle, value mode)
{
  CAMLparam2(idle, mode);

  int rc = -1;
  camluv_idle_t *camluv_idle = camluv_idle_struct_val(idle);
  if (camluv_idle->uv_idle != NULL) {
    rc = uv_run(camluv_idle->uv_idle, camluv_uv_run_mode_ml2c(mode));
  }

  return Val_int(rc);
}

CAMLprim value
camluv_idle_stop(value idle)
{
  CAMLparam1(idle);

  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    uv_stop(camluv_loop->uv_loop);
  }

  return Val_unit;
}
