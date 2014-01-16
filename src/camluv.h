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

#ifndef _CAMLUV_H_
#define _CAMLUV_H_

#include <uv.h>

#include <caml/threads.h>

#define CAMLUV_SLAB_SIZE  65535

#define CAMLUV_UNUSED_ARG(arg)  (void)arg

#ifndef UV_OK
  #define UV_OK 0
#endif

#if !defined(CAMLUV_THREADED_CALLBACK_GUARD)
 #define camluv_enter_callback() \
   do {\
     caml_acquire_runtime_system();\
   } while(0)
 #define camluv_leave_callback() \
   do {\
     caml_release_runtime_system();\
   } while(0)
#else
 #define camluv_enter_callback() \
   int camluv_c_thread_registered = caml_c_thread_register(); \
   if (camluv_c_thread_registered) caml_acquire_runtime_system()
 #define camluv_leave_callback() \
   do {\
     if (camluv_c_thread_registered) { \
       caml_release_runtime_system(); \
       caml_c_thread_unregister(); \
     } \
   } while(0)
#endif

/**
 * Comment CAMLUV_USE_CUMSTOM_OPERATIONS out to use user-provided
 * ocaml custom block operations.
 */
/* #define CAMLUV_USE_CUMSTOM_OPERATIONS */

#define camluv_table_len(v) \
  sizeof(v) / sizeof(v[0])

uv_errno_t camluv_errno_ml2c(value v);

value camluv_errno_c2ml(uv_errno_t error);

#endif /* _CAMLUV_H_ */

