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

#include <caml/callback.h>

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

#define camluv_table_len(v) \
  sizeof(v) / sizeof(v[0])

#endif /* _CAMLUV_H_ */

