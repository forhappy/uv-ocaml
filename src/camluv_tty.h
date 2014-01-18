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

#ifndef _CAMLUV_TTY_H_
#define _CAMLUV_TTY_H_

#include <caml/mlvalues.h>

#include <uv.h>

#define camluv_tty_struct_val(v) \
  (*(camluv_tty_t **)Data_custom_val(v))

/* camluv_tty_t */
typedef struct camluv_tty_s_ camluv_tty_t;
struct camluv_tty_s_ {
  uv_tty_t uv_tty;
  camluv_loop_t *camluv_loop;
  value connection_cb;
  value read_cb;
  value close_cb;
};

typedef struct camluv_tty_connect_ctx_s_ camluv_tty_connect_ctx_t;
struct camluv_tty_connect_ctx_s_ {
    uv_connect_t uv_connect;
    camluv_tty_t *handle;
    value connect_cb;
};

typedef struct camluv_tty_shutdown_ctx_s_ camluv_tty_shutdown_ctx_t;
struct camluv_tty_shutdown_ctx_s_ {
    uv_shutdown_t uv_shutdown;
    camluv_tty_t *handle;
    value shutdown_cb;
};

typedef struct camluv_tty_write_ctx_s_ camluv_tty_write_ctx_t;
struct camluv_tty_write_ctx_s_ {
    uv_write_t uv_write;
    uv_buf_t *bufs;
    camluv_tty_t *send_handle;
    camluv_tty_t *handle;
    value write_cb;
};

#endif /* _CAMLUV_TTY_H_*/
