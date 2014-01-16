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
#include "camluv_loop.h"
#include "camluv_handle.h"
#include "camluv_stream.h"

typedef struct camluv_stream_write_ctx_s_ camluv_stream_write_ctx_t;
struct camluv_stream_write_ctx_s_ {
    uv_write_t uv_write;
    camluv_stream_t *send_handle;
    camluv_stream_t *handle;
    value write_cb;
};

typedef struct camluv_stream_shutdown_ctx_s_ camluv_stream_shutdown_ctx_t;
struct camluv_stream_shutdown_ctx_s_ {
    uv_shutdown_t uv_shutdown;
    camluv_stream_t *handle;
    value shutdown_cb;
};

static void
camluv_stream_struct_finalize(value v)
{
}

static int
camluv_stream_struct_compare(value v1, value v2)
{
  camluv_stream_t *stream1 = camluv_stream_struct_val(v1);
  camluv_stream_t *stream2 = camluv_stream_struct_val(v2);
  if (stream1 == stream2) return 0;
  else if (stream1 < stream2) return -1;
  return 1;
}

static long
camluv_stream_struct_hash(value v)
{
  return (long)camluv_stream_struct_val(v);
}

static struct custom_operations camluv_stream_struct_ops = {
  "camluv.stream",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

static value
camluv_copy_stream(camluv_stream_t *camluv_stream)
{
  CAMLparam0();
  CAMLlocal1(stream);

  stream = caml_alloc_custom(&camluv_stream_struct_ops,
           sizeof(camluv_stream_t *), 0, 1);
  camluv_stream_struct_val(stream) = camluv_stream;

  CAMLreturn(stream);
}

static void
camluv_alloc_cb(uv_stream_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    CAMLUV_UNUSED_ARG(handle);
    static char slab[CAMLUV_SLAB_SIZE];
    buf->base = slab;
    buf->len = sizeof(slab);
}


static void
camluv_shutdown_cb(uv_shutdown_t* uv_shutdown, int status)
{

  camluv_enter_callback();

  CAMLparam0();
  CAMLlocal3(shutdown_cb, shutdown_req, camluv_status);

  camluv_stream_shutdown_ctx_t *shutdown_ctx =
          (camluv_stream_shutdown_ctx_t *)uv_shutdown->data;

  camluv_stream_t *stream = shutdown_ctx->handle;
  shutdown_cb = shutdown_ctx->shutdown_cb;
  camluv_status = Val_int(status);

  camluv_leave_callback();
}

CAMLprim value
camluv_shutdown(value stream, value shutdown_cb)
{

}

