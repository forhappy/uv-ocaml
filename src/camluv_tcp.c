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
#include "camluv_tcp.h"

#if defined(CAMLUV_USE_CUMSTOM_OPERATIONS)
/**
 * TODO: we will use ocaml cumstom operations later to support
 * user-provided finalization, comparision, hashing.
 */
static void
camluv_tcp_struct_finalize(value v)
{
}

static int
camluv_tcp_struct_compare(value v1, value v2)
{
  camluv_tcp_t *tcp1 = camluv_tcp_struct_val(v1);
  camluv_tcp_t *tcp2 = camluv_tcp_struct_val(v2);
  if (tcp1 == tcp2) return 0;
  else if (tcp1 < tcp2) return -1;
  return 1;
}

static long
camluv_tcp_struct_hash(value v)
{
  return (long)camluv_tcp_struct_val(v);
}
#endif /* CAMLUV_USE_CUMSTOM_OPERATIONS */

static struct custom_operations camluv_tcp_struct_ops = {
  "camluv.tcp",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};


/**
 * Parse a OCaml tuple containing host, port, flowinfo, scope_id
 * into a sockaddr struct
 */
static int
camluv_parse_sockaddr(value addr, struct sockaddr_storage *ss)
{
  CAMLparam1(addr);
  char *host;
  int port;
  unsigned int scope_id = 0, flowinfo = 0;

  struct in_addr addr4;
  struct in6_addr addr6;
  struct sockaddr_in *sa4;
  struct sockaddr_in6 *sa6;

  host = String_val(Field(addr, 0));
  port = Int_val(Field(addr, 1));
  scope_id = Int_val(Field(addr, 2));
  flowinfo = Int_val(Field(addr, 3));

  if (port < 0 || port > 0xffff) {
    CAMLreturn(-1);
  }

  if (flowinfo > 0xfffff) {
    CAMLreturn(-1);
  }

  memset(ss, 0, sizeof(struct sockaddr_storage));

  if (uv_inet_pton(AF_INET, host, &addr4) == 0) {
    /* it's an IPv4 address */
    sa4 = (struct sockaddr_in *)ss;
    sa4->sin_family = AF_INET;
    sa4->sin_port = htons((short)port);
    sa4->sin_addr = addr4;
    CAMLreturn(0);
  } else if (uv_inet_pton(AF_INET6, host, &addr6) == 0) {
    /* it's an IPv6 address */
    sa6 = (struct sockaddr_in6 *)ss;
    sa6->sin6_family = AF_INET6;
    sa6->sin6_port = htons((short)port);
    sa6->sin6_addr = addr6;
    sa6->sin6_flowinfo = flowinfo;
    sa6->sin6_scope_id = scope_id;
    CAMLreturn(0);
  } else {
    CAMLreturn(-1);
  }
}

static value
camluv_make_sockaddr(struct sockaddr *addr, int addrlen)
{
  CAMLparam0();
  CAMLlocal1(camluv_addr);

  static char buf[INET6_ADDRSTRLEN + 1];
  struct sockaddr_in *addr4;
  struct sockaddr_in6 *addr6;

  if (addrlen == 0) {
      /* TODO: No address handling. */
  }

  camluv_addr = caml_alloc(4, 0);

  switch (addr->sa_family) {

  case AF_INET:
  {
    addr4 = (struct sockaddr_in*)addr;
    uv_ip4_name(addr4, buf, sizeof(buf));
    Store_field(camluv_addr, 0, caml_copy_string(buf));
    Store_field(camluv_addr, 1, ntohs(addr4->sin_port));
    Store_field(camluv_addr, 2, 0);
    Store_field(camluv_addr, 3, 0);
  }

  case AF_INET6:
  {
    addr6 = (struct sockaddr_in6*)addr;
    uv_ip6_name(addr6, buf, sizeof(buf));
    Store_field(camluv_addr, 0, caml_copy_string(buf));
    Store_field(camluv_addr, 1, ntohs(addr6->sin6_port));
    Store_field(camluv_addr, 2, ntohl(addr6->sin6_flowinfo));
    Store_field(camluv_addr, 3, addr6->sin6_scope_id);
  }

  default:;
    /* TODO: If we don't know the address family. */
  }

  CAMLreturn(camluv_addr);
}

static value
camluv_copy_tcp(camluv_tcp_t *camluv_tcp)
{
  CAMLparam0();
  CAMLlocal1(tcp);

  tcp = caml_alloc_custom(&camluv_tcp_struct_ops,
           sizeof(camluv_tcp_t *), 0, 1);
  camluv_tcp_struct_val(tcp) = camluv_tcp;

  CAMLreturn(tcp);
}

static void
camluv_alloc_cb(uv_tcp_t *handle, size_t suggested_size, uv_buf_t *buf)
{
  camluv_enter_callback();

  CAMLUV_UNUSED_ARG(handle);
  static char slab[CAMLUV_SLAB_SIZE];
  buf->base = slab;
  buf->len = sizeof(slab);

  camluv_leave_callback();
}

static void
camluv_connection_cb(uv_stream_t *server, int status)
{
  camluv_enter_callback();
  CAMLlocal3(connection_cb, camluv_server, camluv_status);

  camluv_tcp_t *tcp = (camluv_tcp_t *)(server->data);
  camluv_server = camluv_copy_tcp(tcp);
  connection_cb = tcp->connection_cb;
  camluv_status = Val_int(status);

  callback2(connection_cb, camluv_server, camluv_status);

  camluv_leave_callback();
}

static void
camluv_connect_cb(uv_connect_t *req, int status)
{
  camluv_enter_callback();
  CAMLlocal3(connect_cb, camluv_server, camluv_status);

  camluv_tcp_t *tcp = (camluv_tcp_t *)(req->data);
  camluv_server = camluv_copy_tcp(tcp);
  connect_cb = tcp->connect_cb;
  camluv_status = Val_int(status);

  callback2(connect_cb, camluv_server, camluv_status);

  camluv_leave_callback();
}

static void
camluv_shutdown_cb(uv_shutdown_t *req, int status)
{
  camluv_enter_callback();
  CAMLlocal3(shutdown_cb, camluv_server, camluv_status);

  camluv_tcp_t *tcp = (camluv_tcp_t *)(req->data);
  camluv_server = camluv_copy_tcp(tcp);
  shutdown_cb = tcp->shutdown_cb;
  camluv_status = Val_int(status);

  callback2(shutdown_cb, camluv_server, camluv_status);

  camluv_leave_callback();
}

static camluv_tcp_t *
camluv_tcp_new(void)
{
  camluv_tcp_t *tcp = (camluv_tcp_t *)malloc(sizeof(camluv_tcp_t));
  if (!tcp) return NULL;

  return tcp;
}

CAMLprim value
camluv_tcp_init(value loop)
{
  CAMLparam1(loop);
  CAMLlocal1(tcp);

  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  camluv_tcp_t *camluv_tcp = camluv_tcp_new();

  int rc = uv_tcp_init(camluv_loop->uv_loop, &(camluv_tcp->uv_tcp));
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  // TODO: other initialization here.
  //
  (camluv_tcp->uv_tcp).data = camluv_tcp;

  tcp = camluv_copy_tcp(camluv_tcp);

  CAMLreturn(tcp);
}

CAMLprim value
camluv_tcp_open(value tcp, value fd)
{
  CAMLparam2(tcp, fd);
  CAMLlocal1(camluv_rc);

  camluv_tcp_t *camluv_tcp = camluv_tcp_struct_val(tcp);

  int rc = uv_tcp_open(&(camluv_tcp->uv_tcp), Int_val(fd));
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  camluv_rc = camluv_errno_c2ml(camluv_rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_tcp_nodelay(value tcp, value enable)
{
  CAMLparam2(tcp, enable);
  CAMLlocal1(camluv_rc);

  camluv_tcp_t *camluv_tcp = camluv_tcp_struct_val(tcp);

  int rc = uv_tcp_nodelay(&(camluv_tcp->uv_tcp), Int_val(enable));
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  camluv_rc = camluv_errno_c2ml(camluv_rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_tcp_keepalive(value tcp, value enable, value delay)
{
  CAMLparam3(tcp, enable, delay);
  CAMLlocal1(camluv_rc);

  camluv_tcp_t *camluv_tcp = camluv_tcp_struct_val(tcp);

  int rc = uv_tcp_keepalive(&(camluv_tcp->uv_tcp),
                            Int_val(enable),
                            Int_val(delay));
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  camluv_rc = camluv_errno_c2ml(camluv_rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_tcp_simultaneous_accepts(value tcp, value enable)
{
  CAMLparam2(tcp, enable);
  CAMLlocal1(camluv_rc);

  camluv_tcp_t *camluv_tcp = camluv_tcp_struct_val(tcp);

  int rc = uv_tcp_simultaneous_accepts(&(camluv_tcp->uv_tcp), Int_val(enable));
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  camluv_rc = camluv_errno_c2ml(camluv_rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_tcp_bind(value tcp, value addr)
{
  CAMLparam2(tcp, addr);
  CAMLlocal1(camluv_rc);

  struct sockaddr_storage ss;
  camluv_tcp_t *camluv_tcp = camluv_tcp_struct_val(tcp);
  camluv_parse_sockaddr(addr, &ss);

  int rc = uv_tcp_bind(&(camluv_tcp->uv_tcp), (struct sockaddr *)&ss);
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  camluv_rc = camluv_errno_c2ml(camluv_rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_tcp_getsockname(value tcp)
{
  CAMLparam1(tcp);
  CAMLlocal1(addr);

  struct sockaddr_storage ss;
  int sockaddr_len = sizeof(struct sockaddr_storage);
  camluv_tcp_t *camluv_tcp = camluv_tcp_struct_val(tcp);

  int rc = uv_tcp_getsockname(&(camluv_tcp->uv_tcp),
                              (struct sockaddr *)&ss,
                              &sockaddr_len);
  if (rc != UV_OK) {
    // TODO: error handling.
  }
  addr = camluv_make_sockaddr((struct sockaddr*)&ss, sockaddr_len);

  CAMLreturn(addr);
}

CAMLprim value
camluv_tcp_getpeername(value tcp)
{
  CAMLparam1(tcp);
  CAMLlocal1(addr);

  struct sockaddr_storage ss;
  int sockaddr_len = sizeof(struct sockaddr_storage);
  camluv_tcp_t *camluv_tcp = camluv_tcp_struct_val(tcp);

  int rc = uv_tcp_getpeername(&(camluv_tcp->uv_tcp),
                              (struct sockaddr *)&ss,
                              &sockaddr_len);
  if (rc != UV_OK) {
    // TODO: error handling.
  }
  addr = camluv_make_sockaddr((struct sockaddr*)&ss, sockaddr_len);

  CAMLreturn(addr);
}

CAMLprim value
camluv_tcp_listen(value server, value backlog, value connection_cb)
{
  CAMLparam3(server, backlog, connection_cb);
  CAMLlocal1(camluv_rc);

  camluv_tcp_t *camluv_server = camluv_tcp_struct_val(server);
  camluv_server->connection_cb = connection_cb;

  int rc = uv_listen((uv_stream_t *)&(camluv_server->uv_tcp),
                     Int_val(backlog),
                     camluv_connection_cb);
  if (rc != UV_OK) {
    // TODO: error handling.
  }
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_tcp_accept(value server)
{
  CAMLparam1(server);
  CAMLlocal1(client);

  camluv_tcp_t *camluv_server = camluv_tcp_struct_val(server);
  camluv_tcp_t *camluv_client = camluv_tcp_new();

  int rc = uv_accept((uv_stream_t *)&(camluv_server->uv_tcp),
                     (uv_stream_t *)&(camluv_client->uv_tcp));
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  // TODO: other initialization here.
  //
  (camluv_client->uv_tcp).data = camluv_client;

  client = camluv_copy_tcp(camluv_client);

  CAMLreturn(client);
}

CAMLprim value
camluv_tcp_connect(value server, value addr, value connect_cb)
{
  CAMLparam3(server, addr, connect_cb);
  CAMLlocal1(camluv_rc);

  struct sockaddr_storage ss;
  uv_connect_t *connect_req = (uv_connect_t *)malloc(sizeof(uv_connect_t));
  camluv_tcp_t *camluv_server = camluv_tcp_struct_val(server);
  camluv_server->connect_cb = connect_cb;
  connect_req->data = camluv_server;

  camluv_parse_sockaddr(addr, &ss);
  int rc = uv_tcp_connect(connect_req,
                          (uv_tcp_t *)&(camluv_server->uv_tcp),
                          (const struct sockaddr*)&ss,
                          camluv_connect_cb);
  if (rc != UV_OK) {
    // TODO: error handling.
  }
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_tcp_shutdown(value tcp, value shutdown_cb)
{
  CAMLparam2(tcp, shutdown_cb);
  CAMLlocal1(camluv_rc);

  uv_shutdown_t *shutdown_req = (uv_shutdown_t *)malloc(sizeof(uv_shutdown_t));
  camluv_tcp_t *camluv_server = camluv_tcp_struct_val(tcp);
  camluv_server->shutdown_cb = shutdown_cb;
  shutdown_req->data = camluv_server;

  int rc = uv_shutdown(shutdown_req,
                       (uv_stream_t *)&(camluv_server->uv_tcp),
                       camluv_shutdown_cb);
  if (rc != UV_OK) {
    // TODO: error handling.
  }
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

