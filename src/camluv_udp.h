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

#ifndef _CAMLUV_UDP_H_
#define _CAMLUV_UDP_H_

#include <caml/mlvalues.h>

#include <uv.h>

#define camluv_udp_struct_val(v) \
  (*(camluv_udp_t **)Data_custom_val(v))

/* camluv_udp_t */
typedef struct camluv_udp_s_ camluv_udp_t;
struct camluv_udp_s_ {
  uv_udp_t uv_udp;
  camluv_loop_t *camluv_loop;
  value udp_recv_cb;
  value close_cb;
};

typedef struct camluv_udp_connect_ctx_s_ camluv_udp_connect_ctx_t;
struct camluv_udp_connect_ctx_s_ {
    uv_connect_t uv_connect;
    camluv_udp_t *handle;
    value connect_cb;
};

typedef struct camluv_udp_send_ctx_s_ camluv_udp_send_ctx_t;
struct camluv_udp_send_ctx_s_ {
    uv_udp_send_t uv_udp_send;
    camluv_udp_t *handle;
    uv_buf_t *bufs;
    value udp_send_cb;
};

#endif /* _CAMLUV_UDP_H_*/
