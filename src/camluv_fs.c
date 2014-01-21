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

static void
camluv_fs_cb(uv_fs_t * req)
{
  camluv_enter_callback();

  camluv_leave_callback();
}

CAMLprim value
camluv_fs_req_cleanup(value fs)
{
}

CAMLprim value
camluv_fs_close(value loop, value file, value fs_cb)
{
}

CAMLprim value
camluv_fs_open_native(value loop,
                      value path,
                      value flags,
                      value mode,
                      value fs_cb)
{
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
                      value buf,
                      value length,
                      value offset,
                      value fs_cb)
{
}

CAMLprim value
camluv_fs_read_bytecode(value *argv, int argn)
{
  return camluv_fs_read_native(argv[0], argv[1], argv[2],
                               argv[3], argv[4], argv[5]);
}

CAMLprim value
camluv_fs_unlink(value loop,
                 value path,
                 value fs_cb)
{
}

CAMLprim value
camluv_fs_write_native(value loop,
                       value file,
                       value buf,
                       value length,
                       value offset,
                       value fs_cb)
{
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
}

CAMLprim value
camluv_fs_rmdir(value loop,
                value path,
                value fs_cb)
{
}

CAMLprim value
camluv_fs_readdir(value loop,
                  value path,
                  value flags,
                  value fs_cb)
{
}

CAMLprim value
camluv_fs_stat(value loop,
               value path,
               value fs_cb)
{
}

CAMLprim value
camluv_fs_fstat(value loop,
                value path,
                value fs_cb)
{
}

CAMLprim value
camluv_fs_rename(value loop,
                 value path,
                 value new_path,
                 value fs_cb)
{
}

CAMLprim value
camluv_fs_fsync(value loop,
                value file,
                value fs_cb)
{
}

CAMLprim value
camluv_fs_fdatasync(value loop,
                    value file,
                    value fs_cb)
{
}

CAMLprim value
camluv_fs_ftruncate(value loop,
                    value file,
                    value offset,
                    value fs_cb)
{
}

CAMLprim value
camluv_fs_sendfile_native(value loop,
                          value out_fd,
                          value in_fd,
                          value in_offset,
                          value length,
                          value fs_cb)
{
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
}

CAMLprim value
camluv_fs_utime_native(value loop,
                       value path,
                       value atime,
                       value mtime,
                       value fs_cb)
{
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
}

CAMLprim value
camluv_fs_link(value loop,
               value path,
               value new_path,
               value fs_cb)
{
}

CAMLprim value
camluv_fs_symlink_native(value loop,
                         value path,
                         value new_path,
                         value flags,
                         value fs_cb)
{
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
}

CAMLprim value
camluv_fs_fchmod(value loop,
                 value file,
                 value mode,
                 value fs_cb)
{
}

CAMLprim value
camluv_fs_chown_native(value loop,
                       value path,
                       value uid,
                       value gid,
                       value fs_cb)
{
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
}

CAMLprim value
camluv_fs_fchown_bytecode(value *argv, int argn)
{
  return camluv_fs_fchown_native(argv[0], argv[1], argv[2],
                                 argv[3], argv[4]);
}

