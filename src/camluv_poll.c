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
#include "camluv_poll.h"


static const enum uv_poll_event CAMLUV_POLL_EVENT_TABLE[] = {
  UV_READABLE,
  UV_WRITABLE,
  UV_READABLE | UV_WRITABLE
};

#if defined(CAMLUV_USE_CUMSTOM_OPERATIONS)
/**
 * TODO: we will use ocaml cumstom operations later to support
 * user-provided finalization, comparision, hashing.
 */
static void
camluv_poll_struct_finalize(value v)
{
}

static int
camluv_poll_struct_compare(value v1, value v2)
{
  camluv_poll_t *poll1 = camluv_poll_struct_val(v1);
  camluv_poll_t *poll2 = camluv_poll_struct_val(v2);
  if (poll1 == poll2) return 0;
  else if (poll1 < poll2) return -1;
  return 1;
}

static long
camluv_poll_struct_hash(value v)
{
  return (long)camluv_poll_struct_val(v);
}
#endif /* CAMLUV_USE_CUMSTOM_OPERATIONS */

static struct custom_operations camluv_poll_struct_ops = {
  "camluv.poll",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

static enum uv_poll_event
camluv_poll_event_ml2c(value v)
{
  CAMLparam1(v);

  enum uv_poll_event event = CAMLUV_POLL_EVENT_TABLE[Long_val(v)];

  CAMLreturn(event);
}

value
camluv_poll_event_c2ml(enum uv_poll_event event)
{
  CAMLparam0();
  CAMLlocal1(v);
  int i = 0, index = -1;

  for (; i < camluv_table_len(CAMLUV_POLL_EVENT_TABLE); i++) {
    if (event == CAMLUV_POLL_EVENT_TABLE[i]) {
      index = i;
      break;
    }
  }
  v = Val_int(index);

  CAMLreturn(v);
}

static value
camluv_copy_poll(camluv_poll_t *camluv_poll)
{
  CAMLparam0();
  CAMLlocal1(poll);

  poll = caml_alloc_custom(&camluv_poll_struct_ops,
           sizeof(camluv_poll_t *), 0, 1);
  camluv_poll_struct_val(poll) = camluv_poll;

  CAMLreturn(poll);
}

static void
camluv_poll_cb(uv_poll_t* uv_handle, int status, int events)
{
  camluv_enter_callback();

  CAMLlocal4(poll_cb, camluv_handle, camluv_status, camluv_events);

  camluv_poll_t *camluv_poll =(camluv_poll_t *)(uv_handle->data);
  poll_cb = camluv_poll->poll_cb;
  camluv_handle = camluv_copy_poll(camluv_poll);
  camluv_status = Val_int(status);
  camluv_events = camluv_poll_event_c2ml(events);

  callback3(poll_cb, camluv_handle, camluv_status, camluv_events);

  camluv_leave_callback();
}

static camluv_poll_t *
camluv_poll_new(void)
{
  camluv_poll_t *poll = (camluv_poll_t *)malloc(sizeof(camluv_poll_t));
  if (!poll) return NULL;

  return poll;
}

CAMLprim value
camluv_poll_init(value loop, value fd)
{
  CAMLparam2(loop, fd);
  CAMLlocal1(poll);

  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  camluv_poll_t *camluv_poll = camluv_poll_new();

  int rc = uv_poll_init(camluv_loop->uv_loop,
                        &(camluv_poll->uv_poll),
                        camluv_poll_event_ml2c(fd));
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  camluv_poll->fd = Int_val(fd);
  (camluv_poll->uv_poll).data = camluv_poll;

  ((camluv_handle_t *)camluv_poll)->uv_handle =
                              (uv_handle_t *)&(camluv_poll->uv_poll);
  camluv_init_handle_with_loop((camluv_handle_t *)
                               (&(camluv_poll->camluv_handle)),
                               camluv_loop);
  poll = camluv_copy_poll(camluv_poll);

  CAMLreturn(poll);
}

CAMLprim value
camluv_poll_start(value poll, value events, value poll_cb)
{
  CAMLparam3(poll, events, poll_cb);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_poll_t *camluv_poll = camluv_poll_struct_val(poll);
  camluv_poll->poll_cb = poll_cb;
  rc = uv_poll_start(&(camluv_poll->uv_poll),
                     Int_val(events),
                     camluv_poll_cb);
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_poll_fileno(value poll)
{
  CAMLparam1(poll);
  CAMLlocal1(fd);

  camluv_poll_t *camluv_poll = camluv_poll_struct_val(poll);
  fd = Val_int(camluv_poll->fd);

  CAMLreturn(fd);
}

CAMLprim value
camluv_poll_stop(value poll)
{
  CAMLparam1(poll);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_poll_t *camluv_poll = camluv_poll_struct_val(poll);
  rc = uv_poll_stop(&(camluv_poll->uv_poll));
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

