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
#include "camluv_loop.h"
#include "camluv_threadpool.h"

#if defined(CAMLUV_USE_CUMSTOM_OPERATIONS)
/**
 * TODO: we will use ocaml cumstom operations later to support
 * user-provided finalization, comparision, hashing.
 */
static void
camluv_threadpool_struct_finalize(value v)
{
}

static int
camluv_threadpool_struct_compare(value v1, value v2)
{
  camluv_threadpool_t *threadpool1 = camluv_threadpool_struct_val(v1);
  camluv_threadpool_t *threadpool2 = camluv_threadpool_struct_val(v2);
  if (threadpool1 == threadpool2) return 0;
  else if (threadpool1 < threadpool2) return -1;
  return 1;
}

static long
camluv_threadpool_struct_hash(value v)
{
  return (long)camluv_threadpool_struct_val(v);
}
#endif /* CAMLUV_USE_CUMSTOM_OPERATIONS */

static struct custom_operations camluv_threadpool_struct_ops = {
  "camluv.threadpool",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

static value
camluv_copy_threadpool(camluv_threadpool_t *camluv_threadpool)
{
  CAMLparam0();
  CAMLlocal1(threadpool);

  threadpool = caml_alloc_custom(&camluv_threadpool_struct_ops,
           sizeof(camluv_threadpool_t *), 0, 1);
  camluv_threadpool_struct_val(threadpool) = camluv_threadpool;

  CAMLreturn(threadpool);
}

static void
camluv_work_cb(uv_work_t * uv_work)
{
  camluv_enter_callback();

  CAMLlocal2(work_cb, threadpool);

  camluv_threadpool_t *camluv_threadpool = (camluv_threadpool_t *)(uv_work->data);
  work_cb = camluv_threadpool->work_cb;
  threadpool = camluv_copy_threadpool(camluv_threadpool);

  callback(work_cb, threadpool);

  camluv_leave_callback();
}

static void
camluv_after_work_cb(uv_work_t * uv_work, int status)
{
  camluv_enter_callback();

  CAMLlocal3(after_work_cb, threadpool, camluv_status);

  camluv_threadpool_t *camluv_threadpool = (camluv_threadpool_t *)(uv_work->data);
  after_work_cb = camluv_threadpool->after_work_cb;
  threadpool = camluv_copy_threadpool(camluv_threadpool);
  camluv_status = Val_int(status);

  callback2(after_work_cb, threadpool, camluv_status);

  camluv_leave_callback();
}

CAMLprim value
camluv_loop_queue_work(value loop, value work_cb, value after_work_cb)
{
  CAMLparam3(loop, work_cb, after_work_cb);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    camluv_threadpool_t *threadpool =
            (camluv_threadpool_t *)malloc(sizeof(camluv_threadpool_t));
    // TODO: where to free this threadpool?

    rc = uv_queue_work(camluv_loop->uv_loop,
                       &(threadpool->uv_work),
                       camluv_work_cb,
                       camluv_after_work_cb);

    camluv_init_request_with_loop((camluv_request_t *)
                                  &(threadpool->camluv_request),
                                  camluv_loop);

    (threadpool->uv_work).data = threadpool;
    ((camluv_request_t *)threadpool)->uv_request =
                              (uv_req_t *)&(threadpool->uv_work);
  }
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

