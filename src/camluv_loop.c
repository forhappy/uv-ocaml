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

static camluv_loop_t *default_loop = NULL;

static const uv_run_mode UV_RUN_MODE_TABLE[] = {
  UV_RUN_DEFAULT,
  UV_RUN_ONCE,
  UV_RUN_NOWAIT
};

static uv_run_mode
camluv_uv_run_mode_ml2c(value v)
{
  CAMLparam1(v);
  return UV_RUN_MODE_TABLE[Long_val(v)];
}

static void
camluv_walk_cb(uv_handle_t* handle, void* arg)
{
  camluv_enter_callback();

  CAMLlocal3(walk_cb, walk_cb_handle, walk_cb_arg);

  camluv_walk_cb_ctx_t *walk_cb_ctx = (camluv_walk_cb_ctx_t *)arg;
  walk_cb = walk_cb_ctx->walk_cb;
  walk_cb_handle = camluv_copy_handle2(handle,
                                       0,
                                       1,
                                       walk_cb_ctx->camluv_loop,
                                       Val_unit);
  walk_cb_arg = caml_copy_string(walk_cb_ctx->arg);

  callback2(walk_cb, walk_cb_handle, walk_cb_arg);

  camluv_leave_callback();
}

static void
camluv_loop_struct_finalize(value v)
{
}

static int
camluv_loop_struct_compare(value v1, value v2)
{
  camluv_loop_t *loop1 = camluv_loop_struct_val(v1);
  camluv_loop_t *loop2 = camluv_loop_struct_val(v2);
  if (loop1 == loop2) return 0;
  else if (loop1 < loop2) return -1;
  return 1;
}

static long
camluv_loop_struct_hash(value v)
{
  return (long)camluv_loop_struct_val(v);
}

static struct custom_operations camluv_loop_struct_ops = {
  "camluv.loop",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

static value
camluv_copy_loop(camluv_loop_t *camluv_loop)
{
  CAMLparam0();
  CAMLlocal1(loop);

  loop = caml_alloc_custom(&camluv_loop_struct_ops,
           sizeof(camluv_loop_t *), 0, 1);
  camluv_loop_struct_val(loop) = camluv_loop;

  CAMLreturn(loop);
}

static value
camluv_copy_loop2(uv_loop_t *uv_loop, int is_default)
{
  CAMLparam0();
  CAMLlocal1(loop);

  camluv_loop_t *camluv_loop = (camluv_loop_t *)
      malloc(sizeof(camluv_loop_t));
  uv_loop->data = camluv_loop;
  camluv_loop->uv_loop = uv_loop;
  camluv_loop->is_default = is_default;

  loop = caml_alloc_custom(&camluv_loop_struct_ops,
          sizeof(camluv_loop_t *), 0, 1);
  camluv_loop_struct_val(loop) = camluv_loop;

  CAMLreturn(loop);
}

static int
camluv_init_loop(camluv_loop_t *loop, int is_default)
{
  uv_loop_t *uv_loop;

  uv_loop = NULL;

  if (is_default) {
    uv_loop = uv_default_loop();
  } else {
    uv_loop = uv_loop_new();
  }

  if (!uv_loop) {
      return -1;
  }
  uv_loop->data = loop;

  loop->uv_loop = uv_loop;
  loop->is_default = is_default;

  return 0;
}

static camluv_loop_t*
camluv_new_loop(int is_default)
{
  if (is_default) {
      default_loop = (camluv_loop_t *)malloc(sizeof(camluv_loop_t));
      if (!default_loop) return NULL;
      if (camluv_init_loop(default_loop, is_default) != 0) return NULL;
      return default_loop;
  } else {
      camluv_loop_t *camluv_loop =
        (camluv_loop_t *)malloc(sizeof(camluv_loop_t));
      if (!camluv_loop) return NULL;
      if (camluv_init_loop(camluv_loop, is_default) != 0) return NULL;
      return camluv_loop;
  }
}

CAMLprim value
camluv_loop_new(value unit)
{
  camluv_loop_t *camluv_loop = camluv_new_loop(0);
  if (!camluv_loop) return Val_unit;

  return camluv_copy_loop(camluv_loop);
}

CAMLprim value
camluv_loop_default(value unit)
{
  camluv_loop_t *camluv_loop = camluv_new_loop(1);
  if (!camluv_loop) return Val_unit;

  return camluv_copy_loop(camluv_loop);
}

CAMLprim value
camluv_loop_delete(value loop)
{
  CAMLparam1(loop);

  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    camluv_loop->uv_loop->data = NULL;
    uv_loop_delete(camluv_loop->uv_loop);
  }

  return Val_unit;
}

CAMLprim value
camluv_loop_run(value loop, value mode)
{
  CAMLparam2(loop, mode);

  int rc = -1;
  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    rc = uv_run(camluv_loop->uv_loop, camluv_uv_run_mode_ml2c(mode));
  }

  return Val_int(rc);
}

CAMLprim value
camluv_loop_stop(value loop)
{
  CAMLparam1(loop);

  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    uv_stop(camluv_loop->uv_loop);
  }

  return Val_unit;
}

CAMLprim value
camluv_loop_update_time(value loop)
{
  CAMLparam1(loop);

  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    uv_update_time(camluv_loop->uv_loop);
  }

  return Val_unit;
}

CAMLprim value
camluv_loop_now(value loop)
{
  CAMLparam1(loop);
  uint64 now = 0;

  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    now = uv_now(camluv_loop->uv_loop);
  }

  return caml_copy_int64(now);
}

CAMLprim value
camluv_loop_backend_fd(value loop)
{
  CAMLparam1(loop);
  int fd = -1;

  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    fd = uv_now(camluv_loop->uv_loop);
  }

  return caml_copy_int32(fd);
}

CAMLprim value
camluv_loop_backend_timeout(value loop)
{
  CAMLparam1(loop);
  int timeout = -1;

  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    timeout = uv_now(camluv_loop->uv_loop);
  }

  return caml_copy_int32(timeout);
}

CAMLprim value
camluv_loop_walk(value loop, value callback, value arg)
{
  CAMLparam3(loop, callback, arg);

  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    camluv_walk_cb_ctx_t *walk_cb_ctx =
      (camluv_walk_cb_ctx_t *)malloc(sizeof(camluv_walk_cb_ctx_t));
    walk_cb_ctx->walk_cb = callback;
    walk_cb_ctx->camluv_loop = camluv_loop;
    walk_cb_ctx->arg = strdup(String_val(arg));

    uv_walk(camluv_loop->uv_loop, camluv_walk_cb, walk_cb_ctx);
  }

  return Val_unit;
}

