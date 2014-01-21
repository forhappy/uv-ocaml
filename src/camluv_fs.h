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

#ifndef _CAMLUV_FS_H_
#define _CAMLUV_FS_H_

#include <caml/mlvalues.h>

#include "camluv_loop.h"
#include "camluv_request.h"

#define camluv_fs_struct_val(v) \
  (*(camluv_fs_t **)Data_custom_val(v))

/* camluv_fs_t */
typedef struct camluv_fs_s_ camluv_fs_t;
struct camluv_fs_s_ {
  camluv_request_t camluv_request;
  uv_fs_t uv_fs;
  camluv_loop_t *camluv_loop;
  value fs_cb;
  value result;
  value error;
  value path;
};

#endif /* _CAMLUV_FS_H_ */
