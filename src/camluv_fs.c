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
#include "camluv_fs.h"

#if defined(CAMLUV_USE_CUMSTOM_OPERATIONS)
/**
 * TODO: we will use ocaml cumstom operations later to support
 * user-provided finalization, comparision, hashing.
 */
static void
camluv_fs_struct_finalize(value v)
{
}

static int
camluv_fs_struct_compare(value v1, value v2)
{
  camluv_fs_t *fs1 = camluv_fs_struct_val(v1);
  camluv_fs_t *fs2 = camluv_fs_struct_val(v2);
  if (fs1 == fs2) return 0;
  else if (fs1 < fs2) return -1;
  return 1;
}

static long
camluv_fs_struct_hash(value v)
{
  return (long)camluv_fs_struct_val(v);
}
#endif /* CAMLUV_USE_CUMSTOM_OPERATIONS */

static struct custom_operations camluv_fs_struct_ops = {
  "camluv.fs",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

static value
camluv_copy_fs(camluv_fs_t *camluv_fs)
{
  CAMLparam0();
  CAMLlocal1(fs);

  fs = caml_alloc_custom(&camluv_fs_struct_ops,
           sizeof(camluv_fs_t *), 0, 1);
  camluv_fs_struct_val(fs) = camluv_fs;

  CAMLreturn(fs);
}

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

static void
camluv_fs_cb(uv_fs_t * req)
{
  camluv_enter_callback();
  CAMLlocal2(fs_cb, camluv_req);

  camluv_fs_t *camluv_fs = (camluv_fs_t *)(req->data);
  fs_cb = camluv_fs->fs_cb;
  camluv_req = camluv_copy_fs(camluv_fs);

  callback(fs_cb, camluv_req);

  if (camluv_fs != NULL) free(camluv_fs);

  camluv_leave_callback();
}

CAMLprim value
camluv_fs_req_cleanup(value fs)
{
  CAMLparam1(fs);

  camluv_fs_t *camluv_fs = camluv_fs_struct_val(fs);
  uv_fs_req_cleanup(&(camluv_fs->uv_fs));

  CAMLreturn(Val_unit);
}

CAMLprim value
camluv_fs_get_result(value fs)
{
  CAMLparam1(fs);
  CAMLlocal1(result);

  camluv_fs_t *camluv_fs = camluv_fs_struct_val(fs);
  result = Val_int((camluv_fs->uv_fs).result);

  CAMLreturn(result);
}

CAMLprim value
camluv_fs_get_path(value fs)
{
  CAMLparam1(fs);
  CAMLlocal1(path);

  camluv_fs_t *camluv_fs = camluv_fs_struct_val(fs);
  path = caml_copy_string((camluv_fs->uv_fs).path);

  CAMLreturn(path);
}

CAMLprim value
camluv_fs_get_stat(value fs)
{
  CAMLparam1(fs);
  CAMLlocal1(stat);

  camluv_fs_t *camluv_fs = camluv_fs_struct_val(fs);
  stat = camluv_build_stat(&((camluv_fs->uv_fs).statbuf));

  CAMLreturn(stat);
}

CAMLprim value
camluv_fs_get_loop(value fs)
{
  CAMLparam1(fs);
  CAMLlocal1(loop);

  camluv_fs_t *camluv_fs = camluv_fs_struct_val(fs);
  loop = camluv_copy_loop(camluv_fs->camluv_loop);

  CAMLreturn(loop);
}


CAMLprim value
camluv_fs_close(value loop, value file, value fs_cb)
{
  CAMLparam3(loop, file, fs_cb);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    camluv_fs_t *fs =
            (camluv_fs_t *)malloc(sizeof(camluv_fs_t));
    // TODO: where to free this fs, in the camluv_fs_cb?


    camluv_init_request_with_loop((camluv_request_t *)
                                  &(fs->camluv_request),
                                  camluv_loop);
    (fs->uv_fs).data = fs;
    fs->camluv_loop = camluv_loop;
    fs->fs_cb = fs_cb;
    ((camluv_request_t *)fs)->uv_request =
                              (uv_req_t *)&(fs->uv_fs);

    rc = uv_fs_close(camluv_loop->uv_loop,
                     &(fs->uv_fs),
                     Int_val(file),
                     camluv_fs_cb);
    if (rc != UV_OK) {
      // TODO: error handling.
    }
  }

  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_fs_open_native(value loop,
                      value path,
                      value flags,
                      value mode,
                      value fs_cb)
{
  CAMLparam5(loop, path, flags, mode, fs_cb);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  printf("camluv_loop: %p.\n", camluv_loop);
  if (camluv_loop->uv_loop != NULL) {
    camluv_fs_t *fs =
            (camluv_fs_t *)malloc(sizeof(camluv_fs_t));
    // TODO: where to free this fs, in the camluv_fs_cb?

    camluv_init_request_with_loop((camluv_request_t *)
                                  &(fs->camluv_request),
                                  camluv_loop);
    (fs->uv_fs).data = fs;
    fs->camluv_loop = camluv_loop;
    fs->fs_cb = fs_cb;
    ((camluv_request_t *)fs)->uv_request =
                              (uv_req_t *)&(fs->uv_fs);

    rc = uv_fs_open(camluv_loop->uv_loop,
                    &(fs->uv_fs),
                    String_val(path),
                    Int_val(flags),
                    Int_val(mode),
                    camluv_fs_cb);
    if (rc != UV_OK) {
      // TODO: error handling.
    }
  }

  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_fs_open_bytecode(value *argv, int argn)
{
  return camluv_fs_open_native(argv[0], argv[1], argv[2],
                               argv[3], argv[4]);
}

CAMLprim value
camluv_fs_read_native(value loop,
                      value file,
                      value length_hint,
                      value offset,
                      value fs_cb)
{
  CAMLparam5(loop, file, length_hint, offset, fs_cb);
  CAMLlocal1(camluv_buf);

  int rc = -1;
  char *readbuf = NULL;
  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    camluv_fs_t *fs =
            (camluv_fs_t *)malloc(sizeof(camluv_fs_t));
    readbuf = (char *)malloc(sizeof(char) * length_hint);
    // TODO: where to free this fs, in the camluv_fs_cb?

    camluv_init_request_with_loop((camluv_request_t *)
                                  &(fs->camluv_request),
                                  camluv_loop);
    (fs->uv_fs).data = fs;
    fs->camluv_loop = camluv_loop;
    fs->fs_cb = fs_cb;
    ((camluv_request_t *)fs)->uv_request =
                              (uv_req_t *)&(fs->uv_fs);

    rc = uv_fs_read(camluv_loop->uv_loop,
                    &(fs->uv_fs),
                    Int_val(file),
                    readbuf,
                    Int_val(length_hint),
                    Int_val(offset),
                    camluv_fs_cb);
    if (rc != UV_OK) {
      // TODO: error handling.
    }
  }

  camluv_buf = caml_copy_string(readbuf);
  free(readbuf);

  CAMLreturn(camluv_buf);
}

CAMLprim value
camluv_fs_read_bytecode(value *argv, int argn)
{
  return camluv_fs_read_native(argv[0], argv[1], argv[2],
                               argv[3], argv[4]);
}

CAMLprim value
camluv_fs_unlink(value loop,
                 value path,
                 value fs_cb)
{
  CAMLparam3(loop, path, fs_cb);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    camluv_fs_t *fs =
            (camluv_fs_t *)malloc(sizeof(camluv_fs_t));
    // TODO: where to free this , in the camluv_fs_cb?

    camluv_init_request_with_loop((camluv_request_t *)
                                  &(fs->camluv_request),
                                  camluv_loop);
    (fs->uv_fs).data = fs;
    fs->camluv_loop = camluv_loop;
    fs->fs_cb = fs_cb;
    ((camluv_request_t *)fs)->uv_request =
                              (uv_req_t *)&(fs->uv_fs);

    rc = uv_fs_unlink(camluv_loop->uv_loop,
                      &(fs->uv_fs),
                      String_val(path),
                      camluv_fs_cb);
    if (rc != UV_OK) {
      // TODO: error handling.
    }
  }

  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_fs_write_native(value loop,
                       value file,
                       value buf,
                       value length,
                       value offset,
                       value fs_cb)
{
  CAMLparam5(loop, file, buf, length, offset);
  CAMLxparam1(fs_cb);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  printf("camluv_loop: %p.\n", camluv_loop);
  if (camluv_loop->uv_loop != NULL) {
    camluv_fs_t *fs =
            (camluv_fs_t *)malloc(sizeof(camluv_fs_t));
    // TODO: where to free this fs, in the camluv_fs_cb?

    camluv_init_request_with_loop((camluv_request_t *)
                                  &(fs->camluv_request),
                                  camluv_loop);
    (fs->uv_fs).data = fs;
    fs->camluv_loop = camluv_loop;
    fs->fs_cb = fs_cb;
    ((camluv_request_t *)fs)->uv_request =
                              (uv_req_t *)&(fs->uv_fs);

    rc = uv_fs_write(camluv_loop->uv_loop,
                    &(fs->uv_fs),
                    Int_val(file),
                    String_val(buf),
                    Int_val(length),
                    Int_val(offset),
                    camluv_fs_cb);
    if (rc != UV_OK) {
      // TODO: error handling.
    }
  }

  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_fs_write_bytecode(value *argv, int argn)
{
  return camluv_fs_write_native(argv[0], argv[1], argv[2],
                                argv[3], argv[4], argv[5]);
}

CAMLprim value
camluv_fs_mkdir(value loop,
                value path,
                value mode,
                value fs_cb)
{
  CAMLparam4(loop, path, mode, fs_cb);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    camluv_fs_t *fs =
            (camluv_fs_t *)malloc(sizeof(camluv_fs_t));
    // TODO: where to free this fs, in the camluv_fs_cb?

    camluv_init_request_with_loop((camluv_request_t *)
                                  &(fs->camluv_request),
                                  camluv_loop);
    (fs->uv_fs).data = fs;
    fs->camluv_loop = camluv_loop;
    fs->fs_cb = fs_cb;
    ((camluv_request_t *)fs)->uv_request =
                              (uv_req_t *)&(fs->uv_fs);

    rc = uv_fs_mkdir(camluv_loop->uv_loop,
                     &(fs->uv_fs),
                     String_val(path),
                     Int_val(mode),
                     camluv_fs_cb);
    if (rc != UV_OK) {
      // TODO: error handling.
    }
  }

  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_fs_rmdir(value loop,
                value path,
                value fs_cb)
{
  CAMLparam3(loop, path, fs_cb);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    camluv_fs_t *fs =
            (camluv_fs_t *)malloc(sizeof(camluv_fs_t));
    // TODO: where to free this fs, in the camluv_fs_cb?

    camluv_init_request_with_loop((camluv_request_t *)
                                  &(fs->camluv_request),
                                  camluv_loop);
    (fs->uv_fs).data = fs;
    fs->camluv_loop = camluv_loop;
    fs->fs_cb = fs_cb;
    ((camluv_request_t *)fs)->uv_request =
                              (uv_req_t *)&(fs->uv_fs);

    rc = uv_fs_rmdir(camluv_loop->uv_loop,
                     &(fs->uv_fs),
                     String_val(path),
                     camluv_fs_cb);
    if (rc != UV_OK) {
      // TODO: error handling.
    }
  }

  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_fs_readdir(value loop,
                  value path,
                  value flags,
                  value fs_cb)
{
  CAMLparam4(loop, path, flags, fs_cb);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    camluv_fs_t *fs =
            (camluv_fs_t *)malloc(sizeof(camluv_fs_t));
    // TODO: where to free this fs, in the camluv_fs_cb?

    camluv_init_request_with_loop((camluv_request_t *)
                                  &(fs->camluv_request),
                                  camluv_loop);
    (fs->uv_fs).data = fs;
    fs->camluv_loop = camluv_loop;
    fs->fs_cb = fs_cb;
    ((camluv_request_t *)fs)->uv_request =
                              (uv_req_t *)&(fs->uv_fs);

    rc = uv_fs_readdir(camluv_loop->uv_loop,
                       &(fs->uv_fs),
                       String_val(path),
                       Int_val(flags),
                       camluv_fs_cb);
    if (rc != UV_OK) {
      // TODO: error handling.
    }
  }

  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_fs_stat(value loop,
               value path,
               value fs_cb)
{
  CAMLparam3(loop, path, fs_cb);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    camluv_fs_t *fs =
            (camluv_fs_t *)malloc(sizeof(camluv_fs_t));
    // TODO: where to free this fs, in the camluv_fs_cb?

    camluv_init_request_with_loop((camluv_request_t *)
                                  &(fs->camluv_request),
                                  camluv_loop);
    (fs->uv_fs).data = fs;
    fs->camluv_loop = camluv_loop;
    fs->fs_cb = fs_cb;
    ((camluv_request_t *)fs)->uv_request =
                              (uv_req_t *)&(fs->uv_fs);

    rc = uv_fs_stat(camluv_loop->uv_loop,
                    &(fs->uv_fs),
                    String_val(path),
                    camluv_fs_cb);
    if (rc != UV_OK) {
      // TODO: error handling.
    }
  }

  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_fs_fstat(value loop,
                value file,
                value fs_cb)
{
  CAMLparam3(loop, file, fs_cb);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    camluv_fs_t *fs =
            (camluv_fs_t *)malloc(sizeof(camluv_fs_t));
    // TODO: where to free this fs, in the camluv_fs_cb?

    camluv_init_request_with_loop((camluv_request_t *)
                                  &(fs->camluv_request),
                                  camluv_loop);
    (fs->uv_fs).data = fs;
    fs->camluv_loop = camluv_loop;
    fs->fs_cb = fs_cb;
    ((camluv_request_t *)fs)->uv_request =
                              (uv_req_t *)&(fs->uv_fs);

    rc = uv_fs_fstat(camluv_loop->uv_loop,
                     &(fs->uv_fs),
                     Int_val(file),
                     camluv_fs_cb);
    if (rc != UV_OK) {
      // TODO: error handling.
    }
  }

  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_fs_rename(value loop,
                 value path,
                 value new_path,
                 value fs_cb)
{
  CAMLparam4(loop, path, new_path, fs_cb);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    camluv_fs_t *fs =
            (camluv_fs_t *)malloc(sizeof(camluv_fs_t));
    // TODO: where to free this fs, in the camluv_fs_cb?

    camluv_init_request_with_loop((camluv_request_t *)
                                  &(fs->camluv_request),
                                  camluv_loop);
    (fs->uv_fs).data = fs;
    fs->camluv_loop = camluv_loop;
    fs->fs_cb = fs_cb;
    ((camluv_request_t *)fs)->uv_request =
                              (uv_req_t *)&(fs->uv_fs);

    rc = uv_fs_rename(camluv_loop->uv_loop,
                      &(fs->uv_fs),
                      String_val(path),
                      String_val(new_path),
                      camluv_fs_cb);
    if (rc != UV_OK) {
      // TODO: error handling.
    }
  }

  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_fs_fsync(value loop,
                value file,
                value fs_cb)
{
  CAMLparam3(loop, file, fs_cb);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    camluv_fs_t *fs =
            (camluv_fs_t *)malloc(sizeof(camluv_fs_t));
    // TODO: where to free this fs, in the camluv_fs_cb?

    camluv_init_request_with_loop((camluv_request_t *)
                                  &(fs->camluv_request),
                                  camluv_loop);
    (fs->uv_fs).data = fs;
    fs->camluv_loop = camluv_loop;
    fs->fs_cb = fs_cb;
    ((camluv_request_t *)fs)->uv_request =
                              (uv_req_t *)&(fs->uv_fs);

    rc = uv_fs_fsync(camluv_loop->uv_loop,
                     &(fs->uv_fs),
                     Int_val(file),
                     camluv_fs_cb);
    if (rc != UV_OK) {
      // TODO: error handling.
    }
  }

  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_fs_fdatasync(value loop,
                    value file,
                    value fs_cb)
{
  CAMLparam3(loop, file, fs_cb);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    camluv_fs_t *fs =
            (camluv_fs_t *)malloc(sizeof(camluv_fs_t));
    // TODO: where to free this fs, in the camluv_fs_cb?

    camluv_init_request_with_loop((camluv_request_t *)
                                  &(fs->camluv_request),
                                  camluv_loop);
    (fs->uv_fs).data = fs;
    fs->camluv_loop = camluv_loop;
    fs->fs_cb = fs_cb;
    ((camluv_request_t *)fs)->uv_request =
                              (uv_req_t *)&(fs->uv_fs);

    rc = uv_fs_fdatasync(camluv_loop->uv_loop,
                         &(fs->uv_fs),
                         Int_val(file),
                         camluv_fs_cb);
    if (rc != UV_OK) {
      // TODO: error handling.
    }
  }

  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_fs_ftruncate(value loop,
                    value file,
                    value offset,
                    value fs_cb)
{
  CAMLparam4(loop, file, offset, fs_cb);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    camluv_fs_t *fs =
            (camluv_fs_t *)malloc(sizeof(camluv_fs_t));
    // TODO: where to free this fs, in the camluv_fs_cb?

    camluv_init_request_with_loop((camluv_request_t *)
                                  &(fs->camluv_request),
                                  camluv_loop);
    (fs->uv_fs).data = fs;
    fs->camluv_loop = camluv_loop;
    fs->fs_cb = fs_cb;
    ((camluv_request_t *)fs)->uv_request =
                              (uv_req_t *)&(fs->uv_fs);

    rc = uv_fs_ftruncate(camluv_loop->uv_loop,
                         &(fs->uv_fs),
                         Int_val(file),
                         Int_val(offset),
                         camluv_fs_cb);
    if (rc != UV_OK) {
      // TODO: error handling.
    }
  }

  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_fs_sendfile_native(value loop,
                          value out_fd,
                          value in_fd,
                          value in_offset,
                          value length,
                          value fs_cb)
{
  CAMLparam5(loop, out_fd, in_fd, in_offset, length);
  CAMLxparam1(fs_cb);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    camluv_fs_t *fs =
            (camluv_fs_t *)malloc(sizeof(camluv_fs_t));
    // TODO: where to free this fs, in the camluv_fs_cb?

    camluv_init_request_with_loop((camluv_request_t *)
                                  &(fs->camluv_request),
                                  camluv_loop);
    (fs->uv_fs).data = fs;
    fs->camluv_loop = camluv_loop;
    fs->fs_cb = fs_cb;
    ((camluv_request_t *)fs)->uv_request =
                              (uv_req_t *)&(fs->uv_fs);

    rc = uv_fs_sendfile(camluv_loop->uv_loop,
                        &(fs->uv_fs),
                        Int_val(out_fd),
                        Int_val(in_fd),
                        Int_val(in_offset),
                        Int_val(length),
                        camluv_fs_cb);
    if (rc != UV_OK) {
      // TODO: error handling.
    }
  }

  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);

}

CAMLprim value
camluv_fs_sendfile_bytecode(value *argv, int argn)
{
  return camluv_fs_sendfile_native(argv[0], argv[1], argv[2],
                                   argv[3], argv[4], argv[5]);
}

CAMLprim value
camluv_fs_chmod(value loop,
                value path,
                value mode,
                value fs_cb)
{
  CAMLparam4(loop, path, mode, fs_cb);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    camluv_fs_t *fs =
            (camluv_fs_t *)malloc(sizeof(camluv_fs_t));
    // TODO: where to free this fs, in the camluv_fs_cb?

    camluv_init_request_with_loop((camluv_request_t *)
                                  &(fs->camluv_request),
                                  camluv_loop);
    (fs->uv_fs).data = fs;
    fs->camluv_loop = camluv_loop;
    fs->fs_cb = fs_cb;
    ((camluv_request_t *)fs)->uv_request =
                              (uv_req_t *)&(fs->uv_fs);

    rc = uv_fs_chmod(camluv_loop->uv_loop,
                     &(fs->uv_fs),
                     String_val(path),
                     Int_val(mode),
                     camluv_fs_cb);
    if (rc != UV_OK) {
      // TODO: error handling.
    }
  }

  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_fs_utime_native(value loop,
                       value path,
                       value atime,
                       value mtime,
                       value fs_cb)
{
  CAMLparam5(loop, path, atime, mtime, fs_cb);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    camluv_fs_t *fs =
            (camluv_fs_t *)malloc(sizeof(camluv_fs_t));
    // TODO: where to free this fs, in the camluv_fs_cb?

    camluv_init_request_with_loop((camluv_request_t *)
                                  &(fs->camluv_request),
                                  camluv_loop);
    (fs->uv_fs).data = fs;
    fs->camluv_loop = camluv_loop;
    fs->fs_cb = fs_cb;
    ((camluv_request_t *)fs)->uv_request =
                              (uv_req_t *)&(fs->uv_fs);

    rc = uv_fs_utime(camluv_loop->uv_loop,
                     &(fs->uv_fs),
                     String_val(path),
                     caml_copy_double(atime),
                     caml_copy_double(mtime),
                     camluv_fs_cb);
    if (rc != UV_OK) {
      // TODO: error handling.
    }
  }

  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_fs_utime_bytecode(value *argv, int argn)
{
  return camluv_fs_utime_native(argv[0], argv[1], argv[2],
                                argv[3], argv[4]);
}

CAMLprim value
camluv_fs_futime_native(value loop,
                        value file,
                        value atime,
                        value mtime,
                        value fs_cb)
{
  CAMLparam5(loop, file, atime, mtime, fs_cb);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    camluv_fs_t *fs =
            (camluv_fs_t *)malloc(sizeof(camluv_fs_t));
    // TODO: where to free this fs, in the camluv_fs_cb?

    camluv_init_request_with_loop((camluv_request_t *)
                                  &(fs->camluv_request),
                                  camluv_loop);
    (fs->uv_fs).data = fs;
    fs->camluv_loop = camluv_loop;
    fs->fs_cb = fs_cb;
    ((camluv_request_t *)fs)->uv_request =
                              (uv_req_t *)&(fs->uv_fs);

    rc = uv_fs_futime(camluv_loop->uv_loop,
                      &(fs->uv_fs),
                      Int_val(file),
                      caml_copy_double(atime),
                      caml_copy_double(mtime),
                      camluv_fs_cb);
    if (rc != UV_OK) {
      // TODO: error handling.
    }
  }

  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_fs_futime_bytecode(value *argv, int argn)
{
  return camluv_fs_futime_native(argv[0], argv[1], argv[2],
                                 argv[3], argv[4]);
}

CAMLprim value
camluv_fs_lstat(value loop,
                value path,
                value fs_cb)
{
  CAMLparam3(loop, path, fs_cb);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    camluv_fs_t *fs =
            (camluv_fs_t *)malloc(sizeof(camluv_fs_t));
    // TODO: where to free this fs, in the camluv_fs_cb?

    camluv_init_request_with_loop((camluv_request_t *)
                                  &(fs->camluv_request),
                                  camluv_loop);
    (fs->uv_fs).data = fs;
    fs->camluv_loop = camluv_loop;
    fs->fs_cb = fs_cb;
    ((camluv_request_t *)fs)->uv_request =
                              (uv_req_t *)&(fs->uv_fs);

    rc = uv_fs_lstat(camluv_loop->uv_loop,
                     &(fs->uv_fs),
                     String_val(path),
                     camluv_fs_cb);
    if (rc != UV_OK) {
      // TODO: error handling.
    }
  }

  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_fs_link(value loop,
               value path,
               value new_path,
               value fs_cb)
{
  CAMLparam4(loop, path, new_path, fs_cb);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    camluv_fs_t *fs =
            (camluv_fs_t *)malloc(sizeof(camluv_fs_t));
    // TODO: where to free this fs, in the camluv_fs_cb?

    camluv_init_request_with_loop((camluv_request_t *)
                                  &(fs->camluv_request),
                                  camluv_loop);
    (fs->uv_fs).data = fs;
    fs->camluv_loop = camluv_loop;
    fs->fs_cb = fs_cb;
    ((camluv_request_t *)fs)->uv_request =
                              (uv_req_t *)&(fs->uv_fs);

    rc = uv_fs_link(camluv_loop->uv_loop,
                    &(fs->uv_fs),
                    String_val(path),
                    String_val(new_path),
                    camluv_fs_cb);
    if (rc != UV_OK) {
      // TODO: error handling.
    }
  }

  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_fs_symlink_native(value loop,
                         value path,
                         value new_path,
                         value flags,
                         value fs_cb)
{
  CAMLparam4(loop, path, new_path, fs_cb);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    camluv_fs_t *fs =
            (camluv_fs_t *)malloc(sizeof(camluv_fs_t));
    // TODO: where to free this fs, in the camluv_fs_cb?

    camluv_init_request_with_loop((camluv_request_t *)
                                  &(fs->camluv_request),
                                  camluv_loop);
    (fs->uv_fs).data = fs;
    fs->camluv_loop = camluv_loop;
    fs->fs_cb = fs_cb;
    ((camluv_request_t *)fs)->uv_request =
                              (uv_req_t *)&(fs->uv_fs);

    rc = uv_fs_symlink(camluv_loop->uv_loop,
                       &(fs->uv_fs),
                       String_val(path),
                       String_val(new_path),
                       Int_val(flags),
                       camluv_fs_cb);
    if (rc != UV_OK) {
      // TODO: error handling.
    }
  }

  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_fs_symlink_bytecode(value *argv, int argn)
{
  return camluv_fs_futime_native(argv[0], argv[1], argv[2],
                                 argv[3], argv[4]);
}

CAMLprim value
camluv_fs_readlink(value loop,
                   value path,
                   value fs_cb)
{
  CAMLparam3(loop, path, fs_cb);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    camluv_fs_t *fs =
            (camluv_fs_t *)malloc(sizeof(camluv_fs_t));
    // TODO: where to free this fs, in the camluv_fs_cb?

    camluv_init_request_with_loop((camluv_request_t *)
                                  &(fs->camluv_request),
                                  camluv_loop);
    (fs->uv_fs).data = fs;
    fs->camluv_loop = camluv_loop;
    fs->fs_cb = fs_cb;
    ((camluv_request_t *)fs)->uv_request =
                              (uv_req_t *)&(fs->uv_fs);

    rc = uv_fs_readlink(camluv_loop->uv_loop,
                        &(fs->uv_fs),
                        String_val(path),
                        camluv_fs_cb);
    if (rc != UV_OK) {
      // TODO: error handling.
    }
  }

  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_fs_fchmod(value loop,
                 value file,
                 value mode,
                 value fs_cb)
{
  CAMLparam4(loop, file, mode, fs_cb);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    camluv_fs_t *fs =
            (camluv_fs_t *)malloc(sizeof(camluv_fs_t));
    // TODO: where to free this fs, in the camluv_fs_cb?

    camluv_init_request_with_loop((camluv_request_t *)
                                  &(fs->camluv_request),
                                  camluv_loop);
    (fs->uv_fs).data = fs;
    fs->camluv_loop = camluv_loop;
    fs->fs_cb = fs_cb;
    ((camluv_request_t *)fs)->uv_request =
                              (uv_req_t *)&(fs->uv_fs);

    rc = uv_fs_fchmod(camluv_loop->uv_loop,
                      &(fs->uv_fs),
                      Int_val(file),
                      Int_val(mode),
                      camluv_fs_cb);
    if (rc != UV_OK) {
      // TODO: error handling.
    }
  }

  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_fs_chown_native(value loop,
                       value path,
                       value uid,
                       value gid,
                       value fs_cb)
{
  CAMLparam5(loop, path, uid, gid, fs_cb);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    camluv_fs_t *fs =
            (camluv_fs_t *)malloc(sizeof(camluv_fs_t));
    // TODO: where to free this fs, in the camluv_fs_cb?

    camluv_init_request_with_loop((camluv_request_t *)
                                  &(fs->camluv_request),
                                  camluv_loop);
    (fs->uv_fs).data = fs;
    fs->camluv_loop = camluv_loop;
    fs->fs_cb = fs_cb;
    ((camluv_request_t *)fs)->uv_request =
                              (uv_req_t *)&(fs->uv_fs);

    rc = uv_fs_chown(camluv_loop->uv_loop,
                     &(fs->uv_fs),
                     String_val(path),
                     Int_val(uid),
                     Int_val(gid),
                     camluv_fs_cb);
    if (rc != UV_OK) {
      // TODO: error handling.
    }
  }

  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_fs_chown_bytecode(value *argv, int argn)
{
  return camluv_fs_chown_native(argv[0], argv[1], argv[2],
                                argv[3], argv[4]);
}

CAMLprim value
camluv_fs_fchown_native(value loop,
                        value file,
                        value uid,
                        value gid,
                        value fs_cb)
{
  CAMLparam5(loop, file, uid, gid, fs_cb);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  if (camluv_loop->uv_loop != NULL) {
    camluv_fs_t *fs =
            (camluv_fs_t *)malloc(sizeof(camluv_fs_t));
    // TODO: where to free this fs, in the camluv_fs_cb?

    camluv_init_request_with_loop((camluv_request_t *)
                                  &(fs->camluv_request),
                                  camluv_loop);
    (fs->uv_fs).data = fs;
    fs->camluv_loop = camluv_loop;
    fs->fs_cb = fs_cb;
    ((camluv_request_t *)fs)->uv_request =
                              (uv_req_t *)&(fs->uv_fs);

    rc = uv_fs_fchown(camluv_loop->uv_loop,
                      &(fs->uv_fs),
                      Int_val(file),
                      Int_val(uid),
                      Int_val(gid),
                      camluv_fs_cb);
    if (rc != UV_OK) {
      // TODO: error handling.
    }
  }

  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_fs_fchown_bytecode(value *argv, int argn)
{
  return camluv_fs_fchown_native(argv[0], argv[1], argv[2],
                                 argv[3], argv[4]);
}

