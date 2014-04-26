/* Copyright (c) 2013-2014 Fu Haiping <haipingf@gmail.com>
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

static const uv_errno_t CAMLUV_ERRNO_TABLE[] = {
  UV_OK,
  UV_E2BIG,
  UV_EACCES,
  UV_EADDRINUSE,
  UV_EADDRNOTAVAIL,
  UV_EAFNOSUPPORT,
  UV_EAGAIN,
  UV_EAI_ADDRFAMILY,
  UV_EAI_AGAIN,
  UV_EAI_BADFLAGS,
  UV_EAI_BADHINTS,
  UV_EAI_CANCELED,
  UV_EAI_FAIL,
  UV_EAI_FAMILY,
  UV_EAI_MEMORY,
  UV_EAI_NODATA,
  UV_EAI_NONAME,
  UV_EAI_OVERFLOW,
  UV_EAI_PROTOCOL,
  UV_EAI_SERVICE,
  UV_EAI_SOCKTYPE,
  UV_EAI_SYSTEM,
  UV_EALREADY,
  UV_EBADF,
  UV_EBUSY,
  UV_ECANCELED,
  UV_ECHARSET,
  UV_ECONNABORTED,
  UV_ECONNREFUSED,
  UV_ECONNRESET,
  UV_EDESTADDRREQ,
  UV_EEXIST,
  UV_EFAULT,
  UV_EHOSTUNREACH,
  UV_EINTR,
  UV_EINVAL,
  UV_EIO,
  UV_EISCONN,
  UV_EISDIR,
  UV_ELOOP,
  UV_EMFILE,
  UV_EMSGSIZE,
  UV_ENAMETOOLONG,
  UV_ENETDOWN,
  UV_ENETUNREACH,
  UV_ENFILE,
  UV_ENOBUFS,
  UV_ENODEV,
  UV_ENOENT,
  UV_ENOMEM,
  UV_ENONET,
  UV_ENOSPC,
  UV_ENOSYS,
  UV_ENOTCONN,
  UV_ENOTDIR,
  UV_ENOTEMPTY,
  UV_ENOTSOCK,
  UV_ENOTSUP,
  UV_EPERM,
  UV_EPIPE,
  UV_EPROTO,
  UV_EPROTONOSUPPORT,
  UV_EPROTOTYPE,
  UV_EROFS,
  UV_ESHUTDOWN,
  UV_ESPIPE,
  UV_ESRCH,
  UV_ETIMEDOUT,
  UV_EXDEV,
  UV_UNKNOWN,
  UV_EOF
};

uv_errno_t
camluv_errno_ml2c(value v)
{
  return CAMLUV_ERRNO_TABLE[Long_val(v)];
}

value
camluv_errno_c2ml(uv_errno_t error)
{
  int i = 0, index = -1;
  for (; i < camluv_table_len(CAMLUV_ERRNO_TABLE); i++) {
    if (error == CAMLUV_ERRNO_TABLE[i]) {
      index = i;
      break;
    }
  }
  return Val_int(index);
}

CAMLprim value
camluv_err_strerror(value err)
{
  CAMLparam1(err);
  CAMLlocal1(strerr);

  const char *strerror = uv_strerror(Int_val(err));
  strerr = caml_copy_string(strerror);

  CAMLreturn(strerr);
}

CAMLprim value
camluv_err_name(value err)
{
  CAMLparam1(err);
  CAMLlocal1(errname);

  const char *strname = uv_err_name(Int_val(err));
  errname = caml_copy_string(strname);

  CAMLreturn(errname);
}

