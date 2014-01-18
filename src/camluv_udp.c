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
#include "camluv_udp.h"

static const uv_membership CAMLUV_MEMBERSHIP_TABLE[] = {
  UV_LEAVE_GROUP,
  UV_JOIN_GROUP
};

#if defined(CAMLUV_USE_CUMSTOM_OPERATIONS)
/**
 * TODO: we will use ocaml cumstom operations later to support
 * user-provided finalization, comparision, hashing.
 */
static void
camluv_udp_struct_finalize(value v)
{
}

static int
camluv_udp_struct_compare(value v1, value v2)
{
  camluv_udp_t *udp1 = camluv_udp_struct_val(v1);
  camluv_udp_t *udp2 = camluv_udp_struct_val(v2);
  if (udp1 == udp2) return 0;
  else if (udp1 < udp2) return -1;
  return 1;
}

static long
camluv_udp_struct_hash(value v)
{
  return (long)camluv_udp_struct_val(v);
}
#endif /* CAMLUV_USE_CUMSTOM_OPERATIONS */

static struct custom_operations camluv_udp_struct_ops = {
  "camluv.udp",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

static uv_run_mode
camluv_membership_ml2c(value v)
{
  CAMLparam1(v);

  uv_membership membership = CAMLUV_MEMBERSHIP_TABLE[Long_val(v)];

  CAMLreturn(membership);
}

static value
CAMLUV_udp_flags_c2ml(enum uv_udp_flags flags)
{
  CAMLparam0();
  CAMLlocal1(camluv_flags);

  switch(flags) {
  case UV_UDP_IPV6ONLY:
    camluv_flags = Val_int(0);
    break;
  case UV_UDP_PARTIAL:
    camluv_flags = Val_int(1);
    break;
  }

  CAMLreturn(camluv_flags);
}

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
camluv_copy_udp(camluv_udp_t *camluv_udp)
{
  CAMLparam0();
  CAMLlocal1(udp);

  udp = caml_alloc_custom(&camluv_udp_struct_ops,
           sizeof(camluv_udp_t *), 0, 1);
  camluv_udp_struct_val(udp) = camluv_udp;

  CAMLreturn(udp);
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

  camluv_udp_t *udp = (camluv_udp_t *)(stream->data);
  close_cb = udp->close_cb;
  camluv_server = camluv_copy_udp(udp);

  callback(close_cb, camluv_server);

  camluv_leave_callback();
}

static void
camluv_udp_send_cb(uv_udp_send_t *req, int status)
{
  camluv_enter_callback();

  CAMLlocal3(udp_send_cb, camluv_server, camluv_status);

  camluv_udp_send_ctx_t *ctx = (camluv_udp_send_ctx_t *)(req->data);
  camluv_udp_t *udp = (camluv_udp_t *)(req->data);
  camluv_server = camluv_copy_udp(udp);
  udp_send_cb = ctx->udp_send_cb;
  camluv_status = Val_int(status);

  if (ctx->bufs != NULL) free(ctx->bufs);

  callback2(udp_send_cb, camluv_server, camluv_status);


  camluv_leave_callback();
}

static void camluv_udp_recv_cb(uv_udp_t *handle,
                           ssize_t nread,
                           const uv_buf_t *buf,
                           const struct sockaddr *addr,
                           unsigned int flags)
{
  camluv_enter_callback();

  CAMLlocal1(udp_recv_cb);
  CAMLlocal5(camluv_server,
             camluv_nread,
             camluv_buf,
             camluv_addr,
             camluv_flags);

  CAMLlocalN(args, 5);

  int sockaddr_len = sizeof(struct sockaddr_storage);
  camluv_udp_t *udp = (camluv_udp_t *)(handle->data);
  udp_recv_cb = udp->udp_recv_cb;
  camluv_server = camluv_copy_udp(udp);

  // TODO: nread has type ssize_t, maybe we need
  // higher integer precisive conversion here.
  camluv_nread = Val_int(nread);
  camluv_buf = camluv_make_uv_buf(buf);
  camluv_addr = camluv_make_sockaddr((struct sockaddr *)addr,
                                     sockaddr_len);
  camluv_flags = CAMLUV_udp_flags_c2ml(flags);

  Store_field(args, 0, camluv_server);
  Store_field(args, 1, camluv_nread);
  Store_field(args, 2, camluv_buf);
  Store_field(args, 3, camluv_addr);
  Store_field(args, 4, camluv_flags);

  callbackN(udp_recv_cb, 5, args);

  camluv_leave_callback();
}

static camluv_udp_t *
camluv_udp_new(void)
{
  camluv_udp_t *udp = (camluv_udp_t *)malloc(sizeof(camluv_udp_t));
  if (!udp) return NULL;

  return udp;
}

CAMLprim value
camluv_udp_init(value loop)
{
  CAMLparam1(loop);
  CAMLlocal1(udp);

  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  camluv_udp_t *camluv_udp = camluv_udp_new();

  int rc = uv_udp_init(camluv_loop->uv_loop, &(camluv_udp->uv_udp));
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  // TODO: other initialization here.
  //
  (camluv_udp->uv_udp).data = camluv_udp;
  camluv_udp->camluv_loop = camluv_loop;

  udp = camluv_copy_udp(camluv_udp);

  CAMLreturn(udp);
}

CAMLprim value
camluv_udp_open(value udp, value fd)
{
  CAMLparam2(udp, fd);
  CAMLlocal1(camluv_rc);

  camluv_udp_t *camluv_udp = camluv_udp_struct_val(udp);

  int rc = uv_udp_open(&(camluv_udp->uv_udp), Int_val(fd));
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  camluv_rc = camluv_errno_c2ml(camluv_rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_udp_set_multicast_loop(value udp, value on)
{
  CAMLparam2(udp, on);
  CAMLlocal1(camluv_rc);

  camluv_udp_t *camluv_udp = camluv_udp_struct_val(udp);

  int rc = uv_udp_set_multicast_loop(&(camluv_udp->uv_udp), Int_val(on));
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  camluv_rc = camluv_errno_c2ml(camluv_rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_udp_set_multicast_ttl(value udp, value ttl)
{
  CAMLparam2(udp, ttl);
  CAMLlocal1(camluv_rc);

  camluv_udp_t *camluv_udp = camluv_udp_struct_val(udp);

  int rc = uv_udp_set_multicast_ttl(&(camluv_udp->uv_udp), Int_val(ttl));
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  camluv_rc = camluv_errno_c2ml(camluv_rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_udp_set_broadcast(value udp, value on)
{
  CAMLparam2(udp, on);
  CAMLlocal1(camluv_rc);

  camluv_udp_t *camluv_udp = camluv_udp_struct_val(udp);

  int rc = uv_udp_set_broadcast(&(camluv_udp->uv_udp), Int_val(on));
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  camluv_rc = camluv_errno_c2ml(camluv_rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_udp_set_ttl(value udp, value ttl)
{
  CAMLparam2(udp, ttl);
  CAMLlocal1(camluv_rc);

  camluv_udp_t *camluv_udp = camluv_udp_struct_val(udp);

  int rc = uv_udp_set_ttl(&(camluv_udp->uv_udp), Int_val(ttl));
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  camluv_rc = camluv_errno_c2ml(camluv_rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_udp_set_membership(value udp,
                          value multicast_addr,
                          value interface_addr,
                          value membership)
{
  CAMLparam4(udp, multicast_addr, interface_addr, membership);
  CAMLlocal1(camluv_rc);

  camluv_udp_t *camluv_udp = camluv_udp_struct_val(udp);

  int rc = uv_udp_set_membership(&(camluv_udp->uv_udp),
                                 String_val(multicast_addr),
                                 String_val(interface_addr),
                                 camluv_membership_ml2c(membership));
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  camluv_rc = camluv_errno_c2ml(camluv_rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_udp_bind(value udp, value addr, value flags)
{
  CAMLparam2(udp, addr);
  CAMLlocal1(camluv_rc);

  struct sockaddr_storage ss;
  camluv_udp_t *camluv_udp = camluv_udp_struct_val(udp);
  camluv_parse_sockaddr(addr, &ss);

  int rc = uv_udp_bind(&(camluv_udp->uv_udp),
                       (struct sockaddr *)&ss,
                       Int_val(flags));
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  camluv_rc = camluv_errno_c2ml(camluv_rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_udp_getsockname(value udp)
{
  CAMLparam1(udp);
  CAMLlocal1(addr);

  struct sockaddr_storage ss;
  int sockaddr_len = sizeof(struct sockaddr_storage);
  camluv_udp_t *camluv_udp = camluv_udp_struct_val(udp);

  int rc = uv_udp_getsockname(&(camluv_udp->uv_udp),
                              (struct sockaddr *)&ss,
                              &sockaddr_len);
  if (rc != UV_OK) {
    // TODO: error handling.
  }
  addr = camluv_make_sockaddr((struct sockaddr*)&ss, sockaddr_len);

  CAMLreturn(addr);
}

CAMLprim value
camluv_udp_close(value server, value close_cb)
{
  CAMLparam2(server, close_cb);

  camluv_udp_t *camluv_server = camluv_udp_struct_val(server);

  uv_close((uv_handle_t *)&(camluv_server->uv_udp), camluv_close_cb);

  CAMLreturn(Val_unit);
}

CAMLprim value
camluv_udp_send(value udp, value addr, value bufs, value udp_send_cb)
{
  CAMLparam2(udp, udp_send_cb);
  CAMLlocal1(camluv_rc);

  int nbufs = 0;
  struct sockaddr_storage ss;

  camluv_udp_t *camluv_server = camluv_udp_struct_val(udp);
  camluv_parse_sockaddr(addr, &ss);
  uv_buf_t *camluv_bufs = camluv_parse_uv_bufs(bufs, &nbufs);

  camluv_udp_send_ctx_t *udp_send_req =
          (camluv_udp_send_ctx_t *)malloc(sizeof(camluv_udp_send_ctx_t));
  // TODO: free udp_send_req in camluv_udp_send_cb.
  udp_send_req->uv_udp_send.data = udp_send_req;
  udp_send_req->handle = camluv_server;
  udp_send_req->udp_send_cb = udp_send_cb;
  udp_send_req->bufs = camluv_bufs;

  int rc = uv_udp_send(&(udp_send_req->uv_udp_send),
                       (uv_udp_t *)&(camluv_server->uv_udp),
                       camluv_bufs,
                       nbufs,
                       (const struct sockaddr *)&ss,
                       camluv_udp_send_cb);
  if (rc != UV_OK) {
    // TODO: error handling.
  }
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_udp_start_recv(value udp, value udp_recv_cb)
{
  CAMLparam2(udp, udp_recv_cb);
  CAMLlocal1(camluv_rc);

  camluv_udp_t *camluv_udp = camluv_udp_struct_val(udp);
  camluv_udp->udp_recv_cb = udp_recv_cb;

  int rc = uv_udp_recv_start((uv_udp_t *)&(camluv_udp->uv_udp),
                         camluv_alloc_cb,
                         camluv_udp_recv_cb);
  if (rc != UV_OK) {
    // TODO: error handling.
  }
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_udp_start_stop(value udp)
{
  CAMLparam1(udp);
  CAMLlocal1(camluv_rc);

  camluv_udp_t *camluv_udp = camluv_udp_struct_val(udp);

  int rc = uv_udp_recv_stop((uv_udp_t *)&(camluv_udp->uv_udp));
  if (rc != UV_OK) {
    // TODO: error handling.
  }
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_udp_is_closing(value udp)
{
  CAMLparam1(udp);
  CAMLlocal1(is_closing);

  camluv_udp_t *camluv_udp = camluv_udp_struct_val(udp);

  if (uv_is_closing((uv_handle_t *)&(camluv_udp->uv_udp))) {
    is_closing = Val_int(1);
    CAMLreturn(is_closing);
  } else {
    is_closing = Val_int(0);
    CAMLreturn(is_closing);
  }
}

CAMLprim value
camluv_udp_is_active(value udp)
{
  CAMLparam1(udp);
  CAMLlocal1(is_active);

  camluv_udp_t *camluv_udp = camluv_udp_struct_val(udp);

  if (uv_is_active((uv_handle_t *)&(camluv_udp->uv_udp))) {
    is_active = Val_int(1);
    CAMLreturn(is_active);
  } else {
    is_active = Val_int(0);
    CAMLreturn(is_active);
  }
}

CAMLprim value
camluv_udp_ref(value udp)
{
  CAMLparam1(udp);

  camluv_udp_t *camluv_udp = camluv_udp_struct_val(udp);

  uv_ref((uv_handle_t *)&(camluv_udp->uv_udp));

  CAMLreturn(Val_unit);
}

CAMLprim value
camluv_udp_unref(value udp)
{
  CAMLparam1(udp);

  camluv_udp_t *camluv_udp = camluv_udp_struct_val(udp);

  uv_unref((uv_handle_t *)&(camluv_udp->uv_udp));

  CAMLreturn(Val_unit);
}

CAMLprim value
camluv_udp_has_ref(value udp)
{
  CAMLparam1(udp);
  CAMLlocal1(has_ref);

  camluv_udp_t *camluv_udp = camluv_udp_struct_val(udp);

  has_ref = Val_int(uv_has_ref((uv_handle_t *)&(camluv_udp->uv_udp)));

  CAMLreturn(has_ref);
}

CAMLprim value
camluv_udp_loop(value udp)
{
  CAMLparam1(udp);
  CAMLlocal1(loop);

  camluv_udp_t *camluv_udp = camluv_udp_struct_val(udp);
  loop = camluv_copy_loop(camluv_udp->camluv_loop);

  CAMLreturn(loop);
}

