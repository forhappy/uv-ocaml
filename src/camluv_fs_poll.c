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

#include "camluv_err.h"
#include "camluv_handle.h"
#include "camluv_loop.h"
#include "camluv_fs_poll.h"

#if defined(CAMLUV_USE_CUMSTOM_OPERATIONS)
/**
 * TODO: we will use ocaml cumstom operations later to support
 * user-provided finalization, comparision, hashing.
 */
static void
camluv_fs_poll_struct_finalize(value v)
{
}

static int
camluv_fs_poll_struct_compare(value v1, value v2)
{
  camluv_fs_poll_t *fs_poll1 = camluv_fs_poll_struct_val(v1);
  camluv_fs_poll_t *fs_poll2 = camluv_fs_poll_struct_val(v2);
  if (fs_poll1 == fs_poll2) return 0;
  else if (fs_poll1 < fs_poll2) return -1;
  return 1;
}

static long
camluv_fs_poll_struct_hash(value v)
{
  return (long)camluv_fs_poll_struct_val(v);
}
#endif /* CAMLUV_USE_CUMSTOM_OPERATIONS */

static struct custom_operations camluv_fs_poll_struct_ops = {
  "camluv.fs_poll",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

static value
camluv_build_stat(const uv_stat_t *stat)
{
  CAMLparam0();
  CAMLlocal5(result_stat,
             ts_atim,
             ts_mtim,
             ts_ctim,
             ts_birthtim);

  result_stat = caml_alloc(16, 0);
  Store_field(result_stat, 0, caml_copy_int64(stat->st_dev));
  Store_field(result_stat, 1, caml_copy_int64(stat->st_mode));
  Store_field(result_stat, 2, caml_copy_int64(stat->st_nlink));
  Store_field(result_stat, 3, caml_copy_int64(stat->st_uid));
  Store_field(result_stat, 4, caml_copy_int64(stat->st_gid));
  Store_field(result_stat, 5, caml_copy_int64(stat->st_rdev));
  Store_field(result_stat, 6, caml_copy_int64(stat->st_ino));
  Store_field(result_stat, 7, caml_copy_int64(stat->st_size));
  Store_field(result_stat, 8, caml_copy_int64(stat->st_blksize));
  Store_field(result_stat, 9, caml_copy_int64(stat->st_blocks));
  Store_field(result_stat, 10, caml_copy_int64(stat->st_flags));
  Store_field(result_stat, 11, caml_copy_int64(stat->st_gen));

  ts_atim = caml_alloc(2, 0);
  Store_field(ts_atim, 0, Val_long(stat->st_atim.tv_sec));
  Store_field(ts_atim, 0, Val_long(stat->st_atim.tv_nsec));
  Store_field(result_stat, 12, ts_atim);

  ts_mtim = caml_alloc(2, 0);
  Store_field(ts_mtim, 0, Val_long(stat->st_mtim.tv_sec));
  Store_field(ts_mtim, 0, Val_long(stat->st_mtim.tv_nsec));
  Store_field(result_stat, 13, ts_mtim);

  ts_ctim = caml_alloc(2, 0);
  Store_field(ts_ctim, 0, Val_long(stat->st_ctim.tv_sec));
  Store_field(ts_ctim, 0, Val_long(stat->st_ctim.tv_nsec));
  Store_field(result_stat, 14, ts_ctim);

  ts_birthtim = caml_alloc(2, 0);
  Store_field(ts_birthtim, 0, Val_long(stat->st_birthtim.tv_sec));
  Store_field(ts_birthtim, 0, Val_long(stat->st_birthtim.tv_nsec));
  Store_field(result_stat, 15, ts_birthtim);

  CAMLreturn(result_stat);
}

static value
camluv_copy_fs_poll(camluv_fs_poll_t *camluv_fs_poll)
{
  CAMLparam0();
  CAMLlocal1(fs_poll);

  fs_poll = caml_alloc_custom(&camluv_fs_poll_struct_ops,
           sizeof(camluv_fs_poll_t *), 0, 1);
  camluv_fs_poll_struct_val(fs_poll) = camluv_fs_poll;

  CAMLreturn(fs_poll);
}

static void
camluv_fs_poll_cb(uv_fs_poll_t* uv_handle,
                   int status,
                   const uv_stat_t *prev,
                   const uv_stat_t *curr)
{
  camluv_enter_callback();

  CAMLlocal5(fs_poll_cb,
             camluv_handle,
             camluv_status,
             camluv_prev_stat,
             camluv_curr_stat);

  CAMLlocalN(args, 4);

  camluv_fs_poll_t *camluv_fs_poll =(camluv_fs_poll_t *)(uv_handle->data);
  fs_poll_cb = camluv_fs_poll->fs_poll_cb;
  camluv_handle = camluv_copy_fs_poll(camluv_fs_poll);
  camluv_status = Val_int(status);
  camluv_prev_stat = camluv_build_stat(prev);
  camluv_curr_stat = camluv_build_stat(curr);

  Store_field(args, 0, camluv_handle);
  Store_field(args, 1, camluv_status);
  Store_field(args, 2, camluv_prev_stat);
  Store_field(args, 3, camluv_curr_stat);

  caml_callbackN(fs_poll_cb, 4, args);

  camluv_leave_callback();
}

static camluv_fs_poll_t *
camluv_fs_poll_new(void)
{
  camluv_fs_poll_t *fs_poll =
          (camluv_fs_poll_t *)malloc(sizeof(camluv_fs_poll_t));
  if (!fs_poll) return NULL;

  return fs_poll;
}

CAMLprim value
camluv_fs_poll_init(value loop)
{
  CAMLparam1(loop);
  CAMLlocal1(fs_poll);

  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  camluv_fs_poll_t *camluv_fs_poll = camluv_fs_poll_new();

  int rc = uv_fs_poll_init(camluv_loop->uv_loop, &(camluv_fs_poll->uv_fs_poll));
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  (camluv_fs_poll->uv_fs_poll).data = camluv_fs_poll;

  ((camluv_handle_t *)camluv_fs_poll)->uv_handle =
                               (uv_handle_t *)&(camluv_fs_poll->uv_fs_poll);
  camluv_init_handle_with_loop((camluv_handle_t *)
                               (&(camluv_fs_poll->camluv_handle)),
                               camluv_loop);
  fs_poll = camluv_copy_fs_poll(camluv_fs_poll);

  CAMLreturn(fs_poll);
}

CAMLprim value
camluv_fs_poll_start(value fs_poll,
                      value fs_poll_cb,
                      value path,
                      value interval)
{
  CAMLparam4(fs_poll, fs_poll_cb, path, interval);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_fs_poll_t *camluv_fs_poll = camluv_fs_poll_struct_val(fs_poll);
  camluv_fs_poll->fs_poll_cb = fs_poll_cb;
  rc = uv_fs_poll_start(&(camluv_fs_poll->uv_fs_poll),
                         camluv_fs_poll_cb,
                         String_val(path),
                         Int_val(interval));
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_fs_poll_stop(value fs_poll)
{
  CAMLparam1(fs_poll);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_fs_poll_t *camluv_fs_poll =
          camluv_fs_poll_struct_val(fs_poll);
  rc = uv_fs_poll_stop(&(camluv_fs_poll->uv_fs_poll));
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

