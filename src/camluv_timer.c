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
#include "camluv_handle.h"
#include "camluv_loop.h"
#include "camluv_timer.h"

#if defined(CAMLUV_USE_CUMSTOM_OPERATIONS)
/**
 * TODO: we will use ocaml cumstom operations later to support
 * user-provided finalization, comparision, hashing.
 */
static void
camluv_timer_struct_finalize(value v)
{
}

static int
camluv_timer_struct_compare(value v1, value v2)
{
  camluv_timer_t *timer1 = camluv_timer_struct_val(v1);
  camluv_timer_t *timer2 = camluv_timer_struct_val(v2);
  if (timer1 == timer2) return 0;
  else if (timer1 < timer2) return -1;
  return 1;
}

static long
camluv_timer_struct_hash(value v)
{
  return (long)camluv_timer_struct_val(v);
}
#endif /* CAMLUV_USE_CUMSTOM_OPERATIONS */

static struct custom_operations camluv_timer_struct_ops = {
  "camluv.timer",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

static value
camluv_copy_timer(camluv_timer_t *camluv_timer)
{
  CAMLparam0();
  CAMLlocal1(timer);

  timer = caml_alloc_custom(&camluv_timer_struct_ops,
           sizeof(camluv_timer_t *), 0, 1);
  camluv_timer_struct_val(timer) = camluv_timer;

  CAMLreturn(timer);
}

static void
camluv_timer_cb(uv_timer_t* uv_handle, int status)
{
  camluv_enter_callback();

  CAMLlocal3(timer_cb, camluv_handle, camluv_status);

  camluv_timer_t *camluv_timer =(camluv_timer_t *)(uv_handle->data);
  timer_cb = camluv_timer->timer_cb;
  camluv_handle = camluv_copy_timer(camluv_timer);
  camluv_status = Val_int(status);

  callback2(timer_cb, camluv_handle, camluv_status);

  camluv_leave_callback();
}

static camluv_timer_t *
camluv_timer_new(void)
{
  camluv_timer_t *timer = (camluv_timer_t *)malloc(sizeof(camluv_timer_t));
  if (!timer) return NULL;

  return timer;
}

CAMLprim value
camluv_timer_init(value loop)
{
  CAMLparam1(loop);
  CAMLlocal1(timer);

  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  camluv_timer_t *camluv_timer = camluv_timer_new();

  int rc = uv_timer_init(camluv_loop->uv_loop, &(camluv_timer->uv_timer));
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  (camluv_timer->uv_timer).data = camluv_timer;
  ((camluv_handle_t *)camluv_timer)->uv_handle =
                               (uv_handle_t *)&(camluv_timer->uv_timer);
  camluv_init_handle_with_loop((camluv_handle_t *)
                               (&(camluv_timer->camluv_handle)),
                               camluv_loop);
  timer = camluv_copy_timer(camluv_timer);

  CAMLreturn(timer);
}

CAMLprim value
camluv_timer_start(value timer,
                   value timer_cb,
                   value timeout,
                   value repeat)
{
  CAMLparam4(timer, timer_cb, timeout, repeat);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_timer_t *camluv_timer = camluv_timer_struct_val(timer);
  camluv_timer->timer_cb = timer_cb;
  uint64_t timeout64 = Int64_val(timeout);
  uint64_t repeat64 = Int64_val(repeat);
  rc = uv_timer_start(&(camluv_timer->uv_timer),
                      camluv_timer_cb,
                      timeout64,
                      repeat64);
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_timer_stop(value timer)
{
  CAMLparam1(timer);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_timer_t *camluv_timer = camluv_timer_struct_val(timer);
  rc = uv_timer_stop(&(camluv_timer->uv_timer));
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_timer_again(value timer)
{
  CAMLparam1(timer);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_timer_t *camluv_timer = camluv_timer_struct_val(timer);
  rc = uv_timer_again(&(camluv_timer->uv_timer));
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_timer_set_repeat(value timer, value repeat)
{
  CAMLparam2(timer, repeat);
  uint64_t repeat64 = Int64_val(repeat);

  camluv_timer_t *camluv_timer = camluv_timer_struct_val(timer);
  uv_timer_set_repeat(&(camluv_timer->uv_timer), repeat64);

  CAMLreturn(Val_unit);
}

CAMLprim value
camluv_timer_get_repeat(value timer)
{
  CAMLparam1(timer);
  CAMLlocal1(camluv_repeat);

  uint64_t repeat = 0;
  camluv_timer_t *camluv_timer = camluv_timer_struct_val(timer);
  repeat = uv_timer_get_repeat(&(camluv_timer->uv_timer));
  camluv_repeat = caml_copy_int64(repeat);

  CAMLreturn(camluv_repeat);
}

