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
#include "camluv_fs_event.h"

static void
camluv_fs_event_struct_finalize(value v)
{
}

static int
camluv_fs_event_struct_compare(value v1, value v2)
{
  camluv_fs_event_t *fs_event1 = camluv_fs_event_struct_val(v1);
  camluv_fs_event_t *fs_event2 = camluv_fs_event_struct_val(v2);
  if (fs_event1 == fs_event2) return 0;
  else if (fs_event1 < fs_event2) return -1;
  return 1;
}

static long
camluv_fs_event_struct_hash(value v)
{
  return (long)camluv_fs_event_struct_val(v);
}

static struct custom_operations camluv_fs_event_struct_ops = {
  "camluv.fs_event",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

static value
camluv_copy_fs_event(camluv_fs_event_t *camluv_fs_event)
{
  CAMLparam0();
  CAMLlocal1(fs_event);

  fs_event = caml_alloc_custom(&camluv_fs_event_struct_ops,
           sizeof(camluv_fs_event_t *), 0, 1);
  camluv_fs_event_struct_val(fs_event) = camluv_fs_event;

  CAMLreturn(fs_event);
}

static void
camluv_fs_event_cb(uv_fs_event_t* uv_handle,
                   const char *filename,
                   int events,
                   int status)
{
  camluv_enter_callback();

  CAMLlocal5(fs_event_cb,
             camluv_handle,
             camluv_filename,
             camluv_events,
             camluv_status);

  CAMLlocalN(args, 4);

  camluv_fs_event_t *camluv_fs_event =(camluv_fs_event_t *)(uv_handle->data);
  fs_event_cb = camluv_fs_event->fs_event_cb;
  camluv_handle = camluv_copy_fs_event(camluv_fs_event);
  camluv_filename = caml_copy_string(filename);
  camluv_events = Val_int(events);
  camluv_status = Val_int(status);

  Store_field(args, 0, camluv_handle);
  Store_field(args, 1, camluv_filename);
  Store_field(args, 2, camluv_events);
  Store_field(args, 3, camluv_status);

  caml_callbackN(fs_event_cb, 4, args);

  camluv_leave_callback();
}

static camluv_fs_event_t *
camluv_fs_event_new(void)
{
  camluv_fs_event_t *fs_event =
          (camluv_fs_event_t *)malloc(sizeof(camluv_fs_event_t));
  if (!fs_event) return NULL;

  return fs_event;
}

CAMLprim value
camluv_fs_event_init(value loop)
{
  CAMLparam1(loop);

  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  camluv_fs_event_t *camluv_fs_event = camluv_fs_event_new();

  int rc = uv_fs_event_init(camluv_loop->uv_loop, &(camluv_fs_event->uv_fs_event));
  if (rc != UV_OK) {
    // TODO: error handling.
  }

  (camluv_fs_event->uv_fs_event).data = camluv_fs_event;

  ((camluv_handle_t *)camluv_fs_event)->uv_handle =
                               (uv_handle_t *)&(camluv_fs_event->uv_fs_event);
  camluv_init_handle_with_loop((camluv_handle_t *)
                               (&(camluv_fs_event->camluv_handle)),
                               camluv_loop);

  return camluv_copy_fs_event(camluv_fs_event);
}

CAMLprim value
camluv_fs_event_start(value fs_event,
                      value fs_event_cb,
                      value filename,
                      value flags)
{
  CAMLparam4(fs_event, fs_event_cb, filename, flags);
  int rc = -1;

  camluv_fs_event_t *camluv_fs_event = camluv_fs_event_struct_val(fs_event);
  camluv_fs_event->fs_event_cb = fs_event_cb;
  rc = uv_fs_event_start(&(camluv_fs_event->uv_fs_event),
                         camluv_fs_event_cb,
                         String_val(filename),
                         Int_val(flags));

  return camluv_errno_c2ml(rc);
}

CAMLprim value
camluv_fs_event_stop(value fs_event)
{
  CAMLparam1(fs_event);
  int rc = -1;

  camluv_fs_event_t *camluv_fs_event =
          camluv_fs_event_struct_val(fs_event);
  rc = uv_fs_event_stop(&(camluv_fs_event->uv_fs_event));

  return camluv_errno_c2ml(rc);
}

