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
#include "camluv_request.h"
#include "camluv_loop.h"

static void
camluv_request_struct_finalize(value v)
{
}

static int
camluv_request_struct_compare(value v1, value v2)
{
  camluv_request_t *request1 = camluv_request_struct_val(v1);
  camluv_request_t *request2 = camluv_request_struct_val(v2);
  if (request1 == request2) return 0;
  else if (request1 < request2) return -1;
  return 1;
}

static long
camluv_request_struct_hash(value v)
{
  return (long)camluv_request_struct_val(v);
}

static struct custom_operations camluv_request_struct_ops = {
  "camluv.request",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

static int
camluv_request_initialized(camluv_request_t *request)
{
  return request->initialized != 0;
}

void
camluv_init_request_with_loop(camluv_request_t *camluv_request,
                              camluv_loop_t *camluv_loop)
{
  camluv_request->camluv_loop = camluv_loop;
  camluv_request->initialized = 1;
}

static value
camluv_copy_request(camluv_request_t *camluv_request)
{
  CAMLparam0();
  CAMLlocal1(request);

  request = caml_alloc_custom(&camluv_request_struct_ops,
           sizeof(camluv_request_t *), 0, 1);
  camluv_request_struct_val(request) = camluv_request;

  CAMLreturn(request);
}

value
camluv_copy_request2(uv_req_t *uv_request,
                    int initialized,
                    camluv_loop_t *camluv_loop)
{
  CAMLparam0();
  CAMLlocal1(request);

  camluv_request_t *camluv_request = (camluv_request_t *)
      malloc(sizeof(camluv_request_t));
  camluv_request->uv_request = uv_request;
  camluv_request->initialized = initialized;
  camluv_request->camluv_loop = camluv_loop;

  request = caml_alloc_custom(&camluv_request_struct_ops,
          sizeof(camluv_request_t *), 0, 1);
  camluv_request_struct_val(request) = camluv_request;

  CAMLreturn(request);
}

CAMLprim value
camluv_cancel(value request)
{
  CAMLparam1(request);

  camluv_request_t *camluv_request = camluv_request_struct_val(request);
  if (camluv_request_initialized(camluv_request)) {
    // TODO: this request is not initialized.
    return Val_unit;
  }

  int rc = uv_cancel(camluv_request->uv_request);

  return camluv_errno_c2ml(rc);
}

