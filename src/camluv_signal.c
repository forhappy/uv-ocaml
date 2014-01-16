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
#include "camluv_signal.h"

#if defined(CAMLUV_USE_CUMSTOM_OPERATIONS)
/**
 * TODO: we will use ocaml cumstom operations later to support
 * user-provided finalization, comparision, hashing.
 */
static void
camluv_signal_struct_finalize(value v)
{
}

static int
camluv_signal_struct_compare(value v1, value v2)
{
  camluv_signal_t *signal1 = camluv_signal_struct_val(v1);
  camluv_signal_t *signal2 = camluv_signal_struct_val(v2);
  if (signal1 == signal2) return 0;
  else if (signal1 < signal2) return -1;
  return 1;
}

static long
camluv_signal_struct_hash(value v)
{
  return (long)camluv_signal_struct_val(v);
}
#endif /* CAMLUV_USE_CUMSTOM_OPERATIONS */

static struct custom_operations camluv_signal_struct_ops = {
  "camluv.signal",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

static value
camluv_copy_signal(camluv_signal_t *camluv_signal)
{
  CAMLparam0();
  CAMLlocal1(signal);

  signal = caml_alloc_custom(&camluv_signal_struct_ops,
           sizeof(camluv_signal_t *), 0, 1);
  camluv_signal_struct_val(signal) = camluv_signal;

  CAMLreturn(signal);
}

static void
camluv_signal_cb(uv_signal_t* uv_handle, int signum)
{
  camluv_enter_callback();

  CAMLlocal3(signal_cb, camluv_handle, camluv_signum);

  camluv_signal_t *camluv_signal =(camluv_signal_t *)(uv_handle->data);
  signal_cb = camluv_signal->signal_cb;
  camluv_handle = camluv_copy_signal(camluv_signal);
  camluv_signum = Val_int(signum);

  callback2(signal_cb, camluv_handle, camluv_signum);

  camluv_leave_callback();
}

static camluv_signal_t *
camluv_signal_new(void)
{
  camluv_signal_t *signal = (camluv_signal_t *)malloc(sizeof(camluv_signal_t));
  if (!signal) return NULL;

  return signal;
}

CAMLprim value
camluv_signal_init(value loop)
{
  CAMLparam1(loop);
  CAMLlocal1(signal);

  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  camluv_signal_t *camluv_signal = camluv_signal_new();

  int rc = uv_signal_init(camluv_loop->uv_loop, &(camluv_signal->uv_signal));
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  (camluv_signal->uv_signal).data = camluv_signal;

  ((camluv_handle_t *)camluv_signal)->uv_handle =
                              (uv_handle_t *)&(camluv_signal->uv_signal);
  camluv_init_handle_with_loop((camluv_handle_t *)
                               (&(camluv_signal->camluv_handle)),
                               camluv_loop);
  signal = camluv_copy_signal(camluv_signal);

  CAMLreturn(signal);
}

CAMLprim value
camluv_signal_start(value signal, value signal_cb, value signum)
{
  CAMLparam3(signal, signal_cb, signum);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_signal_t *camluv_signal = camluv_signal_struct_val(signal);
  camluv_signal->signal_cb = signal_cb;
  rc = uv_signal_start(&(camluv_signal->uv_signal),
                       camluv_signal_cb,
                       Int_val(signum));
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_signal_stop(value signal)
{
  CAMLparam1(signal);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_signal_t *camluv_signal = camluv_signal_struct_val(signal);
  rc = uv_signal_stop(&(camluv_signal->uv_signal));
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

