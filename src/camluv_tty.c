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
#include "camluv_tty.h"

#if defined(CAMLUV_USE_CUMSTOM_OPERATIONS)
/**
 * TODO: we will use ocaml cumstom operations later to support
 * user-provided finalization, comparision, hashing.
 */
static void
camluv_tty_struct_finalize(value v)
{
}

static int
camluv_tty_struct_compare(value v1, value v2)
{
  camluv_tty_t *tty1 = camluv_tty_struct_val(v1);
  camluv_tty_t *tty2 = camluv_tty_struct_val(v2);
  if (tty1 == tty2) return 0;
  else if (tty1 < tty2) return -1;
  return 1;
}

static long
camluv_tty_struct_hash(value v)
{
  return (long)camluv_tty_struct_val(v);
}
#endif /* CAMLUV_USE_CUMSTOM_OPERATIONS */

static struct custom_operations camluv_tty_struct_ops = {
  "camluv.tty",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

static uv_buf_t *
camluv_parse_uv_bufs(value bufs, int *nbufs)
{
  int i = 0;
  uv_buf_t *pbufs = NULL;
  int buf_size= Wosize_val(bufs);

  if (buf_size== 0) {
    // TODO: error handling, no buf here.
  }
  *nbufs = buf_size;
  pbufs = (uv_buf_t *)malloc(sizeof(uv_buf_t) * buf_size);
  if (pbufs == NULL) {
    // TODO: error handling, out of memory.
  }

  for (i = 0; i < buf_size; i++) {
    pbufs[i].base = String_val(Field(Field(bufs, i), 0));
    pbufs[i].len  = Int_val(Field(Field(bufs, i), 1));
  }

  return pbufs;
}

static value
camluv_make_uv_buf(const uv_buf_t *buf)
{
  CAMLparam0();
  CAMLlocal1(camluv_buf);

  camluv_buf = caml_alloc(2, 0);

  Store_field(camluv_buf, 0, caml_copy_string(buf->base));
  Store_field(camluv_buf, 1, Val_int(buf->len));

  CAMLreturn(camluv_buf);
}

static value
camluv_copy_tty(camluv_tty_t *camluv_tty)
{
  CAMLparam0();
  CAMLlocal1(tty);

  tty = caml_alloc_custom(&camluv_tty_struct_ops,
           sizeof(camluv_tty_t *), 0, 1);
  camluv_tty_struct_val(tty) = camluv_tty;

  CAMLreturn(tty);
}

static void
camluv_alloc_cb(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
  camluv_enter_callback();

  CAMLUV_UNUSED_ARG(handle);
  static char slab[CAMLUV_SLAB_SIZE];
  buf->base = slab;
  buf->len = sizeof(slab);

  camluv_leave_callback();
}

static void
camluv_close_cb(uv_handle_t* stream)
{
  camluv_enter_callback();

  CAMLlocal2(close_cb, camluv_server);

  camluv_tty_t *tty = (camluv_tty_t *)(stream->data);
  close_cb = tty->close_cb;
  camluv_server = camluv_copy_tty(tty);

  callback(close_cb, camluv_server);

  camluv_leave_callback();
}

static void
camluv_shutdown_cb(uv_shutdown_t *req, int status)
{
  camluv_enter_callback();

  CAMLlocal3(shutdown_cb, camluv_server, camluv_status);

  camluv_tty_shutdown_ctx_t *ctx = (camluv_tty_shutdown_ctx_t *)(req->data);
  camluv_tty_t *tty = (camluv_tty_t *)(req->data);
  camluv_server = camluv_copy_tty(tty);
  shutdown_cb = ctx->shutdown_cb;
  camluv_status = Val_int(status);

  callback2(shutdown_cb, camluv_server, camluv_status);

  camluv_leave_callback();
}

static void camluv_read_cb(uv_stream_t *stream,
                           ssize_t nread,
                           const uv_buf_t *buf)
{
  camluv_enter_callback();

  CAMLlocal4(read_cb, camluv_server, camluv_nread, camluv_buf);

  camluv_tty_t *tty = (camluv_tty_t *)(stream->data);
  read_cb = tty->read_cb;
  camluv_server = camluv_copy_tty(tty);

  // TODO: nread has type ssize_t, maybe we need
  // higher integer precisive conversion here.
  camluv_nread = Val_int(nread);
  camluv_buf = camluv_make_uv_buf(buf);

  callback3(read_cb, camluv_server, camluv_nread, camluv_buf);

  camluv_leave_callback();
}

static void camluv_write_cb(uv_write_t *req, int status)
{
  camluv_enter_callback();

  CAMLlocal3(write_cb, camluv_tty, camluv_status);

  camluv_tty_write_ctx_t *ctx = (camluv_tty_write_ctx_t *)req->data;
  camluv_tty_t *tty = ctx->handle;
  write_cb = ctx->write_cb;
  camluv_tty = camluv_copy_tty(tty);
  camluv_status = Val_int(status);

  if (ctx->bufs != NULL) free(ctx->bufs);

  callback2(write_cb, camluv_tty, camluv_status);

  camluv_leave_callback();
}

static camluv_tty_t *
camluv_tty_new(void)
{
  camluv_tty_t *tty = (camluv_tty_t *)malloc(sizeof(camluv_tty_t));
  if (!tty) return NULL;

  return tty;
}

CAMLprim value
camluv_tty_init(value loop, value fd, value readable)
{
  CAMLparam3(loop, fd, readable);
  CAMLlocal1(tty);

  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  camluv_tty_t *camluv_tty = camluv_tty_new();

  int rc = uv_tty_init(camluv_loop->uv_loop,
                       &(camluv_tty->uv_tty),
                       Int_val(fd),
                       Int_val(readable));
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  // TODO: other initialization here.
  //
  (camluv_tty->uv_tty).data = camluv_tty;
  camluv_tty->camluv_loop = camluv_loop;

  tty = camluv_copy_tty(camluv_tty);

  CAMLreturn(tty);
}

CAMLprim value
camluv_tty_set_mode(value tty, value mode)
{
  CAMLparam2(tty, mode);
  CAMLlocal1(camluv_rc);

  camluv_tty_t *camluv_tty = camluv_tty_struct_val(tty);

  int rc = uv_tty_set_mode(&(camluv_tty->uv_tty), Int_val(mode));
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  camluv_rc = camluv_errno_c2ml(camluv_rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_tty_reset_mode(value unit)
{
  CAMLparam1(unit);
  CAMLlocal1(camluv_rc);

  int rc = uv_tty_reset_mode();
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  camluv_rc = camluv_errno_c2ml(camluv_rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_tty_get_winsize(value tty)
{
  CAMLparam1(tty);
  CAMLlocal1(winsize);

  int width = 0, height = 0;
  camluv_tty_t *camluv_tty = camluv_tty_struct_val(tty);

  int rc = uv_tty_get_winsize(&(camluv_tty->uv_tty), &width, &height);
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  winsize = caml_alloc(2, 0);
  Store_field(winsize, 0, Val_int(width));
  Store_field(winsize, 1, Val_int(height));

  CAMLreturn(winsize);
}

CAMLprim value
camluv_tty_close(value server, value close_cb)
{
  CAMLparam2(server, close_cb);

  camluv_tty_t *camluv_server = camluv_tty_struct_val(server);

  uv_close((uv_handle_t *)&(camluv_server->uv_tty), camluv_close_cb);

  CAMLreturn(Val_unit);
}

CAMLprim value
camluv_tty_shutdown(value tty, value shutdown_cb)
{
  CAMLparam2(tty, shutdown_cb);
  CAMLlocal1(camluv_rc);

  camluv_tty_t *camluv_tty = camluv_tty_struct_val(tty);

  camluv_tty_shutdown_ctx_t *shutdown_req =
          (camluv_tty_shutdown_ctx_t *)malloc(sizeof(camluv_tty_shutdown_ctx_t));
  // TODO: free shutdown_req in camluv_shutdown_cb.
  shutdown_req->uv_shutdown.data = shutdown_req;
  shutdown_req->handle = camluv_tty;
  shutdown_req->shutdown_cb = shutdown_cb;

  int rc = uv_shutdown(&(shutdown_req->uv_shutdown),
                       (uv_stream_t *)&(camluv_tty->uv_tty),
                       camluv_shutdown_cb);
  if (rc != UV_OK) {
    // TODO: error handling.
  }
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_tty_start_read(value tty, value read_cb)
{
  CAMLparam2(tty, read_cb);
  CAMLlocal1(camluv_rc);

  camluv_tty_t *camluv_tty = camluv_tty_struct_val(tty);
  camluv_tty->read_cb = read_cb;

  int rc = uv_read_start((uv_stream_t *)&(camluv_tty->uv_tty),
                         camluv_alloc_cb,
                         camluv_read_cb);
  if (rc != UV_OK) {
    // TODO: error handling.
  }
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_tty_stop_read(value tty)
{
  CAMLparam1(tty);
  CAMLlocal1(camluv_rc);

  camluv_tty_t *camluv_tty = camluv_tty_struct_val(tty);

  int rc = uv_read_stop((uv_stream_t *)&(camluv_tty->uv_tty));
  if (rc != UV_OK) {
    // TODO: error handling.
  }
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_tty_start_write(value tty, value bufs, value write_cb)
{
  CAMLparam3(tty, bufs, write_cb);
  CAMLlocal1(camluv_rc);

  int nbufs = 0;
  camluv_tty_t *camluv_tty = camluv_tty_struct_val(tty);
  uv_buf_t *camluv_bufs = camluv_parse_uv_bufs(bufs, &nbufs);

  camluv_tty_write_ctx_t *write_req =
          (camluv_tty_write_ctx_t *)malloc(sizeof(camluv_tty_write_ctx_t));
  // TODO: free write_req in camluv_write_cb.
  write_req->uv_write.data = write_req;
  write_req->handle = camluv_tty;
  write_req->write_cb = write_cb;
  write_req->bufs = camluv_bufs;

  int rc = uv_write(&(write_req->uv_write),
                    (uv_stream_t *)&(camluv_tty->uv_tty),
                    camluv_bufs,
                    nbufs,
                    camluv_write_cb);
  if (rc != UV_OK) {
    // TODO: error handling.
  }
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_tty_is_readable(value tty)
{
  CAMLparam1(tty);
  CAMLlocal1(camluv_rc);

  camluv_tty_t *camluv_tty = camluv_tty_struct_val(tty);

  int rc = uv_is_readable((uv_stream_t *)&(camluv_tty->uv_tty));
  if (rc != UV_OK) {
    // TODO: error handling.
  }
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_tty_is_writable(value tty)
{
  CAMLparam1(tty);
  CAMLlocal1(camluv_rc);

  camluv_tty_t *camluv_tty = camluv_tty_struct_val(tty);

  int rc = uv_is_writable((uv_stream_t *)&(camluv_tty->uv_tty));
  if (rc != UV_OK) {
    // TODO: error handling.
  }
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_tty_write_queue_size(value tty)
{
  CAMLparam1(tty);
  CAMLlocal1(write_queue_size);

  camluv_tty_t *camluv_tty = camluv_tty_struct_val(tty);

  write_queue_size =
          Val_int((camluv_tty->uv_tty).write_queue_size);

  CAMLreturn(write_queue_size);
}

CAMLprim value
camluv_tty_set_blocking(value tty, value blocking)
{
  CAMLparam1(tty);
  CAMLlocal1(camluv_rc);

  camluv_tty_t *camluv_tty = camluv_tty_struct_val(tty);

  int rc = uv_stream_set_blocking((uv_stream_t *)&(camluv_tty->uv_tty),
                                  Int_val(blocking));
  if (rc != UV_OK) {
    // TODO: error handling.
  }
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_tty_is_closing(value tty)
{
  CAMLparam1(tty);
  CAMLlocal1(is_closing);

  camluv_tty_t *camluv_tty = camluv_tty_struct_val(tty);

  if (uv_is_closing((uv_handle_t *)&(camluv_tty->uv_tty))) {
    is_closing = Val_int(1);
    CAMLreturn(is_closing);
  } else {
    is_closing = Val_int(0);
    CAMLreturn(is_closing);
  }
}

CAMLprim value
camluv_tty_is_active(value tty)
{
  CAMLparam1(tty);
  CAMLlocal1(is_active);

  camluv_tty_t *camluv_tty = camluv_tty_struct_val(tty);

  if (uv_is_active((uv_handle_t *)&(camluv_tty->uv_tty))) {
    is_active = Val_int(1);
    CAMLreturn(is_active);
  } else {
    is_active = Val_int(0);
    CAMLreturn(is_active);
  }
}

CAMLprim value
camluv_tty_ref(value tty)
{
  CAMLparam1(tty);

  camluv_tty_t *camluv_tty = camluv_tty_struct_val(tty);

  uv_ref((uv_handle_t *)&(camluv_tty->uv_tty));

  CAMLreturn(Val_unit);
}

CAMLprim value
camluv_tty_unref(value tty)
{
  CAMLparam1(tty);

  camluv_tty_t *camluv_tty = camluv_tty_struct_val(tty);

  uv_unref((uv_handle_t *)&(camluv_tty->uv_tty));

  CAMLreturn(Val_unit);
}

CAMLprim value
camluv_tty_has_ref(value tty)
{
  CAMLparam1(tty);
  CAMLlocal1(has_ref);

  camluv_tty_t *camluv_tty = camluv_tty_struct_val(tty);

  has_ref = Val_int(uv_has_ref((uv_handle_t *)&(camluv_tty->uv_tty)));

  CAMLreturn(has_ref);
}

CAMLprim value
camluv_tty_loop(value tty)
{
  CAMLparam1(tty);
  CAMLlocal1(loop);

  camluv_tty_t *camluv_tty = camluv_tty_struct_val(tty);
  loop = camluv_copy_loop(camluv_tty->camluv_loop);

  CAMLreturn(loop);
}

