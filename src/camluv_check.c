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
#include "camluv_handle.h"
#include "camluv_loop.h"
#include "camluv_check.h"

static void
camluv_check_struct_finalize(value v)
{
}

static int
camluv_check_struct_compare(value v1, value v2)
{
  camluv_check_t *check1 = camluv_check_struct_val(v1);
  camluv_check_t *check2 = camluv_check_struct_val(v2);
  if (check1 == check2) return 0;
  else if (check1 < check2) return -1;
  return 1;
}

static long
camluv_check_struct_hash(value v)
{
  return (long)camluv_check_struct_val(v);
}

static struct custom_operations camluv_check_struct_ops = {
  "camluv.check",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

static value
camluv_copy_check(camluv_check_t *camluv_check)
{
  CAMLparam0();
  CAMLlocal1(check);

  check = caml_alloc_custom(&camluv_check_struct_ops,
           sizeof(camluv_check_t *), 0, 1);
  camluv_check_struct_val(check) = camluv_check;

  CAMLreturn(check);
}

static void
camluv_check_cb(uv_check_t* uv_handle, int status)
{
  camluv_enter_callback();

  CAMLlocal3(check_cb, camluv_handle, camluv_status);

  camluv_check_t *camluv_check =(camluv_check_t *)(uv_handle->data);
  check_cb = camluv_check->check_cb;
  camluv_handle = camluv_copy_check(camluv_check);
  camluv_status = Val_int(status);

  callback2(check_cb, camluv_handle, camluv_status);

  camluv_leave_callback();
}

static camluv_check_t *
camluv_check_new(void)
{
  camluv_check_t *check = (camluv_check_t *)malloc(sizeof(camluv_check_t));
  if (!check) return NULL;

  return check;
}

CAMLprim value
camluv_check_init(value loop)
{
  CAMLparam1(loop);

  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  camluv_check_t *camluv_check = camluv_check_new();

  int rc = uv_check_init(camluv_loop->uv_loop, &(camluv_check->uv_check));
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  (camluv_check->uv_check).data = camluv_check;

  ((camluv_handle_t *)camluv_check)->uv_handle =
                              (uv_handle_t *)&(camluv_check->uv_check);
  camluv_init_handle_with_loop((camluv_handle_t *)
                               (&(camluv_check->camluv_handle)),
                               camluv_loop);

  return camluv_copy_check(camluv_check);
}

CAMLprim value
camluv_check_start(value check, value check_cb)
{
  CAMLparam2(check, check_cb);
  int rc = -1;

  camluv_check_t *camluv_check = camluv_check_struct_val(check);
  camluv_check->check_cb = check_cb;
  rc = uv_check_start(&(camluv_check->uv_check), camluv_check_cb);

  return camluv_errno_c2ml(rc);
}

CAMLprim value
camluv_check_stop(value check)
{
  CAMLparam1(check);
  int rc = -1;

  camluv_check_t *camluv_check = camluv_check_struct_val(check);
  rc = uv_check_stop(&(camluv_check->uv_check));

  return camluv_errno_c2ml(rc);
}

