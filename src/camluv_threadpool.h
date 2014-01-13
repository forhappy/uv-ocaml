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

#ifndef _CAMLUV_THREADPOOL_H_
#define _CAMLUV_THREADPOOL_H_

#include <caml/mlvalues.h>

#include "camluv_loop.h"
#include "camluv_request.h"

#define camluv_threadpool_struct_val(v) \
  (*(camluv_threadpool_t **)Data_custom_val(v))

/* camluv_threadpool_t */
typedef struct camluv_threadpool_s_ camluv_threadpool_t;
struct camluv_threadpool_s_ {
  camluv_request_t camluv_request;
  uv_work_t uv_work;
  value work_cb;
  value after_work_cb;
};

#endif /* _CAMLUV_THREADPOOL_H_ */
