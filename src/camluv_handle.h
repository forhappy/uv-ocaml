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

#ifndef _CAMLUV_HANDLE_H_
#define _CAMLUV_HANDLE_H_

#include <caml/mlvalues.h>

#include "camluv_loop.h"

#define camluv_handle_struct_val(v) \
  (*(camluv_handle_t **)Data_custom_val(v))

/* camluv_handle_t */
typedef struct camluv_handle_s_ camluv_handle_t;
struct camluv_handle_s_ {
  uv_handle_t *uv_handle;
  int flags;
  int initialized; /* non-zero if the handle is initialized. */
  camluv_loop_t *camluv_loop;
  value close_cb;
};

value camluv_copy_handle2(uv_handle_t *uv_handle,
                          int flags,
                          int initialized,
                          camluv_loop_t *camluv_loop,
                          value on_close_cb);

void camluv_handle_init(camluv_handle_t *camluv_handle,
                        camluv_loop_t *camluv_loop);

#endif /* _CAMLUV_HANDLE_H_*/
