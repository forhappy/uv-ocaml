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
#include "camluv_pipe.h"
#include "camluv_tcp.h"
#include "camluv_udp.h"

#if defined(CAMLUV_USE_CUMSTOM_OPERATIONS)
/**
 * TODO: we will use ocaml cumstom operations later to support
 * user-provided finalization, comparision, hashing.
 */
static void
camluv_pipe_struct_finalize(value v)
{
}

static int
camluv_pipe_struct_compare(value v1, value v2)
{
  camluv_pipe_t *pipe1 = camluv_pipe_struct_val(v1);
  camluv_pipe_t *pipe2 = camluv_pipe_struct_val(v2);
  if (pipe1 == pipe2) return 0;
  else if (pipe1 < pipe2) return -1;
  return 1;
}

static long
camluv_pipe_struct_hash(value v)
{
  return (long)camluv_pipe_struct_val(v);
}
#endif /* CAMLUV_USE_CUMSTOM_OPERATIONS */

static struct custom_operations camluv_pipe_struct_ops = {
  "camluv.pipe",
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
camluv_copy_pipe(camluv_pipe_t *camluv_pipe)
{
  CAMLparam0();
  CAMLlocal1(pipe);

  pipe = caml_alloc_custom(&camluv_pipe_struct_ops,
           sizeof(camluv_pipe_t *), 0, 1);
  camluv_pipe_struct_val(pipe) = camluv_pipe;

  CAMLreturn(pipe);
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
camluv_connect_cb(uv_connect_t *req, int status)
{
  camluv_enter_callback();

  CAMLlocal3(connect_cb, camluv_server, camluv_status);

  camluv_pipe_connect_ctx_t *ctx = (camluv_pipe_connect_ctx_t *)(req->data);
  camluv_pipe_t *pipe = ctx->handle;
  camluv_server = camluv_copy_pipe(pipe);
  connect_cb = ctx->connect_cb;
  camluv_status = Val_int(status);

  callback2(connect_cb, camluv_server, camluv_status);

  camluv_leave_callback();
}

static void
camluv_close_cb(uv_handle_t* stream)
{
  camluv_enter_callback();

  CAMLlocal2(close_cb, camluv_server);

  camluv_pipe_t *pipe = (camluv_pipe_t *)(stream->data);
  close_cb = pipe->close_cb;
  camluv_server = camluv_copy_pipe(pipe);

  callback(close_cb, camluv_server);

  camluv_leave_callback();
}

static void
camluv_shutdown_cb(uv_shutdown_t *req, int status)
{
  camluv_enter_callback();

  CAMLlocal3(shutdown_cb, camluv_server, camluv_status);

  camluv_pipe_shutdown_ctx_t *ctx = (camluv_pipe_shutdown_ctx_t *)(req->data);
  camluv_pipe_t *pipe = (camluv_pipe_t *)(req->data);
  camluv_server = camluv_copy_pipe(pipe);
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

  camluv_pipe_t *pipe = (camluv_pipe_t *)(stream->data);
  read_cb = pipe->read_cb;
  camluv_server = camluv_copy_pipe(pipe);

  // TODO: nread has type ssize_t, maybe we need
  // higher integer precisive conversion here.
  camluv_nread = Int_val(nread);
  camluv_buf = camluv_make_uv_buf(buf);

  callback3(read_cb, camluv_server, camluv_nread, camluv_buf);

  camluv_leave_callback();
}

static void camluv_read2_cb(uv_pipe_t *stream,
                           ssize_t nread,
                           const uv_buf_t *buf,
                           uv_handle_type pending)
{
  camluv_enter_callback();

  CAMLlocal5(read2_cb,
             camluv_pipe,
             camluv_nread,
             camluv_buf,
             camluv_pending);

  CAMLlocalN(args, 4);

  camluv_pipe_t *pipe = (camluv_pipe_t *)(stream->data);
  read2_cb = pipe->read2_cb;
  camluv_pipe= camluv_copy_pipe(pipe);

  // TODO: nread has type ssize_t, maybe we need
  // higher integer precisive conversion here.
  camluv_nread = Int_val(nread);
  camluv_buf = camluv_make_uv_buf(buf);
  camluv_pending = Int_val(pending); // TODO: parse uv_handle_type.

  Store_field(args, 0, camluv_pipe);
  Store_field(args, 1, camluv_nread);
  Store_field(args, 2, camluv_buf);
  Store_field(args, 3, camluv_pending);

  caml_callbackN(read2_cb, 4, args);

  camluv_leave_callback();
}


static void camluv_write_cb(uv_write_t *req, int status)
{
  camluv_enter_callback();

  CAMLlocal3(write_cb, camluv_pipe, camluv_status);

  camluv_pipe_write_ctx_t *ctx = (camluv_pipe_write_ctx_t *)req->data;
  camluv_pipe_t *pipe = ctx->handle;
  write_cb = ctx->write_cb;
  camluv_pipe = camluv_copy_pipe(pipe);
  camluv_status = Val_int(status);
  if (ctx->bufs != NULL) free(ctx->bufs);

  callback2(write_cb, camluv_pipe, camluv_status);

  camluv_leave_callback();
}

static void
camluv_connection_cb(uv_stream_t *server, int status)
{
  camluv_enter_callback();
  CAMLlocal3(connection_cb, camluv_server, camluv_status);

  camluv_pipe_t *pipe = (camluv_pipe_t *)(server->data);
  camluv_server = camluv_copy_pipe(pipe);
  connection_cb = pipe->connection_cb;
  camluv_status = Val_int(status);

  callback2(connection_cb, camluv_server, camluv_status);

  camluv_leave_callback();
}

static camluv_pipe_t *
camluv_pipe_new(void)
{
  camluv_pipe_t *pipe = (camluv_pipe_t *)malloc(sizeof(camluv_pipe_t));
  if (!pipe) return NULL;

  return pipe;
}

CAMLprim value
camluv_pipe_init(value loop, value ipc)
{
  CAMLparam2(loop, ipc);
  CAMLlocal1(pipe);

  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  camluv_pipe_t *camluv_pipe = camluv_pipe_new();

  int rc = uv_pipe_init(camluv_loop->uv_loop,
                        &(camluv_pipe->uv_pipe),
                        Int_val(ipc));
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  // TODO: other initialization here.
  //
  (camluv_pipe->uv_pipe).data = camluv_pipe;
  camluv_pipe->camluv_loop = camluv_loop;
  camluv_pipe->ipc = Int_val(ipc);

  pipe = camluv_copy_pipe(camluv_pipe);

  CAMLreturn(pipe);
}

CAMLprim value
camluv_pipe_open(value pipe, value fd)
{
  CAMLparam2(pipe, fd);
  CAMLlocal1(camluv_rc);

  camluv_pipe_t *camluv_pipe = camluv_pipe_struct_val(pipe);

  int rc = uv_pipe_open(&(camluv_pipe->uv_pipe), Int_val(fd));
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  camluv_rc = camluv_errno_c2ml(camluv_rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_pipe_bind(value pipe, value name)
{
  CAMLparam2(pipe, name);
  CAMLlocal1(camluv_rc);

  const char *camluv_name = String_val(name);
  camluv_pipe_t *camluv_pipe = camluv_pipe_struct_val(pipe);

  int rc = uv_pipe_bind(&(camluv_pipe->uv_pipe), camluv_name);
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  camluv_rc = camluv_errno_c2ml(camluv_rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_pipe_listen(value server, value backlog, value connection_cb)
{
  CAMLparam3(server, backlog, connection_cb);
  CAMLlocal1(camluv_rc);

  camluv_pipe_t *camluv_server = camluv_pipe_struct_val(server);
  camluv_server->connection_cb = connection_cb;

  int rc = uv_listen((uv_stream_t *)&(camluv_server->uv_pipe),
                     Int_val(backlog),
                     camluv_connection_cb);
  if (rc != UV_OK) {
    // TODO: error handling.
  }
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_pipe_accept(value server)
{
  CAMLparam1(server);
  CAMLlocal1(client);

  camluv_pipe_t *camluv_server = camluv_pipe_struct_val(server);
  camluv_pipe_t *camluv_client = camluv_pipe_new();

  int rc = uv_accept((uv_stream_t *)&(camluv_server->uv_pipe),
                     (uv_stream_t *)&(camluv_client->uv_pipe));
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  // TODO: other initialization here.
  //
  (camluv_client->uv_pipe).data = camluv_client;

  client = camluv_copy_pipe(camluv_client);

  CAMLreturn(client);
}

CAMLprim value
camluv_pipe_connect(value server, value name, value connect_cb)
{
  CAMLparam3(server, name, connect_cb);
  CAMLlocal1(camluv_rc);

  const char *camluv_name = String_val(name);
  camluv_pipe_t *camluv_server = camluv_pipe_struct_val(server);

  camluv_pipe_connect_ctx_t *connect_req =
          (camluv_pipe_connect_ctx_t *)malloc(sizeof(camluv_pipe_connect_ctx_t));
  // TODO: free connect_req in camluv_connect_cb.
  connect_req->uv_connect.data = connect_req;
  connect_req->handle = camluv_server;
  connect_req->connect_cb = connect_cb;

  uv_pipe_connect(&(connect_req->uv_connect),
                          (uv_pipe_t *)&(camluv_server->uv_pipe),
                          camluv_name,
                          camluv_connect_cb);

  CAMLreturn(Val_unit);
}

CAMLprim value
camluv_pipe_close(value server, value close_cb)
{
  CAMLparam2(server, close_cb);

  camluv_pipe_t *camluv_server = camluv_pipe_struct_val(server);

  uv_close((uv_handle_t *)&(camluv_server->uv_pipe), camluv_close_cb);

  CAMLreturn(Val_unit);
}

CAMLprim value
camluv_pipe_shutdown(value pipe, value shutdown_cb)
{
  CAMLparam2(pipe, shutdown_cb);
  CAMLlocal1(camluv_rc);

  camluv_pipe_t *camluv_server = camluv_pipe_struct_val(pipe);

  camluv_pipe_shutdown_ctx_t *shutdown_req =
          (camluv_pipe_shutdown_ctx_t *)malloc(sizeof(camluv_pipe_shutdown_ctx_t));
  // TODO: free shutdown_req in camluv_shutdown_cb.
  shutdown_req->uv_shutdown.data = shutdown_req;
  shutdown_req->handle = camluv_server;
  shutdown_req->shutdown_cb = shutdown_cb;

  int rc = uv_shutdown(&(shutdown_req->uv_shutdown),
                       (uv_stream_t *)&(camluv_server->uv_pipe),
                       camluv_shutdown_cb);
  if (rc != UV_OK) {
    // TODO: error handling.
  }
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_pipe_start_read(value pipe, value read_cb)
{
  CAMLparam2(pipe, read_cb);
  CAMLlocal1(camluv_rc);

  camluv_pipe_t *camluv_pipe = camluv_pipe_struct_val(pipe);
  camluv_pipe->read_cb = read_cb;

  int rc = uv_read_start((uv_stream_t *)&(camluv_pipe->uv_pipe),
                         camluv_alloc_cb,
                         camluv_read_cb);
  if (rc != UV_OK) {
    // TODO: error handling.
  }
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_pipe_start_read2(value pipe, value read2_cb)
{
  CAMLparam2(pipe, read2_cb);
  CAMLlocal1(camluv_rc);

  camluv_pipe_t *camluv_pipe = camluv_pipe_struct_val(pipe);
  camluv_pipe->read2_cb = read2_cb;

  int rc = uv_read2_start((uv_stream_t *)&(camluv_pipe->uv_pipe),
                         camluv_alloc_cb,
                         camluv_read2_cb);
  if (rc != UV_OK) {
    // TODO: error handling.
  }
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_pipe_start_stop(value pipe)
{
  CAMLparam1(pipe);
  CAMLlocal1(camluv_rc);

  camluv_pipe_t *camluv_pipe = camluv_pipe_struct_val(pipe);

  int rc = uv_read_stop((uv_stream_t *)&(camluv_pipe->uv_pipe));
  if (rc != UV_OK) {
    // TODO: error handling.
  }
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_pipe_write(value pipe, value bufs, value write_cb)
{
  CAMLparam3(pipe, bufs, write_cb);
  CAMLlocal1(camluv_rc);

  int nbufs = 0;
  camluv_pipe_t *camluv_pipe = camluv_pipe_struct_val(pipe);
  uv_buf_t *camluv_bufs = camluv_parse_uv_bufs(bufs, &nbufs);

  camluv_pipe_write_ctx_t *write_req =
          (camluv_pipe_write_ctx_t *)malloc(sizeof(camluv_pipe_write_ctx_t));
  // TODO: free write_req in camluv_write_cb.
  write_req->uv_write.data = write_req;
  write_req->handle = camluv_pipe;
  write_req->write_cb = write_cb;
  write_req->bufs = camluv_bufs;

  int rc = uv_write(&(write_req->uv_write),
                    (uv_stream_t *)&(camluv_pipe->uv_pipe),
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
camluv_pipe_write2_pipe(value pipe,
                   value bufs,
                   value send_handle_pipe,
                   value write_cb)
{
  CAMLparam3(pipe, bufs, write_cb);
  CAMLlocal1(camluv_rc);

  int nbufs = 0;
  camluv_pipe_t *camluv_pipe = camluv_pipe_struct_val(pipe);
  camluv_pipe_t *camluv_send_handle = camluv_pipe_struct_val(send_handle_pipe);
  uv_buf_t *camluv_bufs = camluv_parse_uv_bufs(bufs, &nbufs);

  camluv_pipe_write_ctx_t *write_req =
          (camluv_pipe_write_ctx_t *)malloc(sizeof(camluv_pipe_write_ctx_t));
  // TODO: free write_req in camluv_write_cb.
  write_req->uv_write.data = write_req;
  write_req->handle = camluv_pipe;
  write_req->write_cb = write_cb;
  write_req->bufs = camluv_bufs;

  int rc = uv_write2(&(write_req->uv_write),
                    (uv_stream_t *)&(camluv_pipe->uv_pipe),
                    camluv_bufs,
                    nbufs,
                    (uv_stream_t *)&(camluv_send_handle->uv_pipe),
                    camluv_write_cb);
  if (rc != UV_OK) {
    // TODO: error handling.
  }
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_pipe_write2_tcp(value pipe,
                   value bufs,
                   value send_handle_tcp,
                   value write_cb)
{
  CAMLparam3(pipe, bufs, write_cb);
  CAMLlocal1(camluv_rc);

  int nbufs = 0;
  camluv_pipe_t *camluv_pipe = camluv_pipe_struct_val(pipe);
  camluv_tcp_t *camluv_send_handle = camluv_tcp_struct_val(send_handle_tcp);
  uv_buf_t *camluv_bufs = camluv_parse_uv_bufs(bufs, &nbufs);

  camluv_pipe_write_ctx_t *write_req =
          (camluv_pipe_write_ctx_t *)malloc(sizeof(camluv_pipe_write_ctx_t));
  // TODO: free write_req in camluv_write_cb.
  write_req->uv_write.data = write_req;
  write_req->handle = camluv_pipe;
  write_req->write_cb = write_cb;
  write_req->bufs = camluv_bufs;

  int rc = uv_write2(&(write_req->uv_write),
                    (uv_stream_t *)&(camluv_pipe->uv_pipe),
                    camluv_bufs,
                    nbufs,
                    (uv_stream_t *)&(camluv_send_handle->uv_tcp),
                    camluv_write_cb);
  if (rc != UV_OK) {
    // TODO: error handling.
  }
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_pipe_is_readable(value pipe)
{
  CAMLparam1(pipe);
  CAMLlocal1(camluv_rc);

  camluv_pipe_t *camluv_pipe = camluv_pipe_struct_val(pipe);

  int rc = uv_is_readable((uv_stream_t *)&(camluv_pipe->uv_pipe));
  if (rc != UV_OK) {
    // TODO: error handling.
  }
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_pipe_is_writable(value pipe)
{
  CAMLparam1(pipe);
  CAMLlocal1(camluv_rc);

  camluv_pipe_t *camluv_pipe = camluv_pipe_struct_val(pipe);

  int rc = uv_is_writable((uv_stream_t *)&(camluv_pipe->uv_pipe));
  if (rc != UV_OK) {
    // TODO: error handling.
  }
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_pipe_write_queue_size(value pipe)
{
  CAMLparam1(pipe);
  CAMLlocal1(write_queue_size);

  camluv_pipe_t *camluv_pipe = camluv_pipe_struct_val(pipe);

  write_queue_size =
          Val_int((camluv_pipe->uv_pipe).write_queue_size);

  CAMLreturn(write_queue_size);
}

CAMLprim value
camluv_pipe_set_blocking(value pipe, value blocking)
{
  CAMLparam1(pipe);
  CAMLlocal1(camluv_rc);

  camluv_pipe_t *camluv_pipe = camluv_pipe_struct_val(pipe);

  int rc = uv_stream_set_blocking((uv_stream_t *)&(camluv_pipe->uv_pipe),
                                  Int_val(blocking));
  if (rc != UV_OK) {
    // TODO: error handling.
  }
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_pipe_is_closing(value pipe)
{
  CAMLparam1(pipe);
  CAMLlocal1(is_closing);

  camluv_pipe_t *camluv_pipe = camluv_pipe_struct_val(pipe);

  if (uv_is_closing((uv_handle_t *)&(camluv_pipe->uv_pipe))) {
    is_closing = Val_int(1);
    CAMLreturn(is_closing);
  } else {
    is_closing = Val_int(0);
    CAMLreturn(is_closing);
  }
}

CAMLprim value
camluv_pipe_is_active(value pipe)
{
  CAMLparam1(pipe);
  CAMLlocal1(is_active);

  camluv_pipe_t *camluv_pipe = camluv_pipe_struct_val(pipe);

  if (uv_is_active((uv_handle_t *)&(camluv_pipe->uv_pipe))) {
    is_active = Val_int(1);
    CAMLreturn(is_active);
  } else {
    is_active = Val_int(0);
    CAMLreturn(is_active);
  }
}

CAMLprim value
camluv_pipe_ref(value pipe)
{
  CAMLparam1(pipe);

  camluv_pipe_t *camluv_pipe = camluv_pipe_struct_val(pipe);

  uv_ref((uv_handle_t *)&(camluv_pipe->uv_pipe));

  CAMLreturn(Val_unit);
}

CAMLprim value
camluv_pipe_unref(value pipe)
{
  CAMLparam1(pipe);

  camluv_pipe_t *camluv_pipe = camluv_pipe_struct_val(pipe);

  uv_unref((uv_handle_t *)&(camluv_pipe->uv_pipe));

  CAMLreturn(Val_unit);
}

CAMLprim value
camluv_pipe_has_ref(value pipe)
{
  CAMLparam1(pipe);
  CAMLlocal1(has_ref);

  camluv_pipe_t *camluv_pipe = camluv_pipe_struct_val(pipe);

  has_ref = Val_int(uv_has_ref((uv_handle_t *)&(camluv_pipe->uv_pipe)));

  CAMLreturn(has_ref);
}

CAMLprim value
camluv_pipe_loop(value pipe)
{
  CAMLparam1(pipe);
  CAMLlocal1(loop);

  camluv_pipe_t *camluv_pipe = camluv_pipe_struct_val(pipe);
  loop = camluv_copy_loop(camluv_pipe->camluv_loop);

  CAMLreturn(loop);
}

