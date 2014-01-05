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

static void
camluv_handle_struct_finalize(value v)
{
}

static int
camluv_handle_struct_compare(value v1, value v2)
{
  camluv_handle_t *handle1 = camluv_handle_struct_val(v1);
  camluv_handle_t *handle2 = camluv_handle_struct_val(v2);
  if (handle1 == handle2) return 0;
  else if (handle1 < handle2) return -1;
  return 1;
}

static long
camluv_handle_struct_hash(value v)
{
  return (long)camluv_handle_struct_val(v);
}

static struct custom_operations camluv_handle_struct_ops = {
  "camluv.handle",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

static int
camluv_handle_initialized(camluv_handle_t *handle)
{
  return handle->initialized != 0;
}

void
camluv_handle_init(camluv_handle_t *camluv_handle,
                   camluv_loop_t *camluv_loop)
{
  camluv_handle->camluv_loop = camluv_loop;
  camluv_handle->flags = 0;
  camluv_handle->initialized = 1;
}

static value
camluv_copy_handle(camluv_handle_t *camluv_handle)
{
  CAMLparam0();
  CAMLlocal1(handle);

  handle = caml_alloc_custom(&camluv_handle_struct_ops,
           sizeof(camluv_handle_t *), 0, 1);
  camluv_handle_struct_val(handle) = camluv_handle;

  CAMLreturn(handle);
}

value
camluv_copy_handle2(uv_handle_t *uv_handle,
                    int flags,
                    int initialized,
                    camluv_loop_t *camluv_loop,
                    value close_cb)
{
  CAMLparam0();
  CAMLlocal1(handle);

  camluv_handle_t *camluv_handle = (camluv_handle_t *)
      malloc(sizeof(camluv_handle_t));
  camluv_handle->uv_handle = uv_handle;
  camluv_handle->flags = flags;
  camluv_handle->initialized = initialized;
  camluv_handle->camluv_loop = camluv_loop;
  camluv_handle->close_cb = close_cb;

  handle = caml_alloc_custom(&camluv_handle_struct_ops,
          sizeof(camluv_handle_t *), 0, 1);
  camluv_handle_struct_val(handle) = camluv_handle;

  CAMLreturn(handle);
}

static void
camluv_close_cb(uv_handle_t* uv_handle)
{
  camluv_enter_callback();

  CAMLlocal2(close_cb, handle);

  camluv_handle_t *camluv_handle = uv_handle->data;
  close_cb = camluv_handle->close_cb;
  handle = camluv_copy_handle(camluv_handle);

  callback(close_cb, handle);

  camluv_leave_callback();
}

CAMLprim value
camluv_close(value handle, value close_cb)
{
  CAMLparam2(handle, close_cb);

  camluv_handle_t *camluv_handle = camluv_handle_struct_val(handle);
  if (camluv_handle_initialized(camluv_handle)) {
    // TODO: this handle is not initialized.
    return Val_unit;
  }
  if (uv_is_closing(camluv_handle->uv_handle)) {
    // TODO: this handle is closing.
    return Val_unit;
  }

  camluv_handle->uv_handle->data = camluv_handle;
  camluv_handle->close_cb = close_cb;
  uv_close(camluv_handle->uv_handle, camluv_close_cb);

  return Val_unit;
}

CAMLprim value
camluv_is_closing(value handle)
{
  CAMLparam1(handle);

  camluv_handle_t *camluv_handle = camluv_handle_struct_val(handle);
  if (uv_is_closing(camluv_handle->uv_handle)) {
    return Val_int(1);
  }

  return Val_int(0);
}

CAMLprim value
camluv_is_active(value handle)
{
  CAMLparam1(handle);

  camluv_handle_t *camluv_handle = camluv_handle_struct_val(handle);
  if (uv_is_active(camluv_handle->uv_handle)) {
    return Val_int(1);
  }

  return Val_int(0);
}

CAMLprim value
camluv_ref(value handle)
{
  CAMLparam1(handle);

  camluv_handle_t *camluv_handle = camluv_handle_struct_val(handle);
  if (camluv_handle_initialized(camluv_handle)) {
    // TODO: this handle is not initialized.
    return Val_unit;
  }
  if (uv_is_closing(camluv_handle->uv_handle)) {
    // TODO: this handle is closing.
    return Val_unit;
  }

  uv_ref(camluv_handle->uv_handle);

  return Val_unit;
}

CAMLprim value
camluv_unref(value handle)
{
  CAMLparam1(handle);

  camluv_handle_t *camluv_handle = camluv_handle_struct_val(handle);
  if (camluv_handle_initialized(camluv_handle)) {
    // TODO: this handle is not initialized.
    return Val_unit;
  }
  if (uv_is_closing(camluv_handle->uv_handle)) {
    // TODO: this handle is closing.
    return Val_unit;
  }

  uv_unref(camluv_handle->uv_handle);

  return Val_unit;
}

CAMLprim value
camluv_has_ref(value handle)
{
  CAMLparam1(handle);

  camluv_handle_t *camluv_handle = camluv_handle_struct_val(handle);
  if (camluv_handle_initialized(camluv_handle)) {
    // TODO: this handle is not initialized.
    return Val_int(0);
  }
  if (uv_is_closing(camluv_handle->uv_handle)) {
    // TODO: this handle is closing.
    return Val_int(0);
  }

  return Val_int(uv_has_ref(camluv_handle->uv_handle));
}

CAMLprim value
camluv_loop(value handle)
{
  CAMLparam1(handle);

  camluv_handle_t *camluv_handle = camluv_handle_struct_val(handle);
  if (camluv_handle_initialized(camluv_handle)) {
    // TODO: this handle is not initialized.
    return Val_unit;
  }
  if (uv_is_closing(camluv_handle->uv_handle)) {
    // TODO: this handle is closing.
    return Val_unit;
  }

  return camluv_copy_loop(camluv_handle->camluv_loop);
}

