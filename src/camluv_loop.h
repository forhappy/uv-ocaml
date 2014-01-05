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

#ifndef _CAMLUV_LOOP_H_
#define _CAMLUV_LOOP_H_

#include <caml/mlvalues.h>

#define camluv_loop_struct_val(v) \
  (*(camluv_loop_t **)Data_custom_val(v))

/* camluv_loop_t */
typedef struct camluv_loop_s_ camluv_loop_t;
struct camluv_loop_s_ {
  uv_loop_t *uv_loop;
  int is_default;
};

/* camluv_walk_cb_ctx */
typedef struct camluv_walk_cb_ctx_s_ camluv_walk_cb_ctx_t;
struct camluv_walk_cb_ctx_s_ {
  value walk_cb;
  void *arg;
};

external camluv_loop_t *default_loop;

#endif /* _CAMLUV_LOOP_H_ */
