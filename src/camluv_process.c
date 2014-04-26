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
#include "camluv_process.h"

static void
camluv_exit_cb(uv_process_t* uv_handle,
               int64_t exit_status,
               int term_signal);

#if defined(CAMLUV_USE_CUMSTOM_OPERATIONS)
/**
 * TODO: we will use ocaml cumstom operations later to support
 * user-provided finalization, comparision, hashing.
 */
static void
camluv_process_struct_finalize(value v)
{
}

static int
camluv_process_struct_compare(value v1, value v2)
{
  camluv_process_t *process1 = camluv_process_struct_val(v1);
  camluv_process_t *process2 = camluv_process_struct_val(v2);
  if (process1 == process2) return 0;
  else if (process1 < process2) return -1;
  return 1;
}

static long
camluv_process_struct_hash(value v)
{
  return (long)camluv_process_struct_val(v);
}
#endif /* CAMLUV_USE_CUMSTOM_OPERATIONS */

static struct custom_operations camluv_process_struct_ops = {
  "camluv.process",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

static value
camluv_copy_process(camluv_process_t *camluv_process)
{
  CAMLparam0();
  CAMLlocal1(process);

  process = caml_alloc_custom(&camluv_process_struct_ops,
           sizeof(camluv_process_t *), 0, 1);
  camluv_process_struct_val(process) = camluv_process;

  CAMLreturn(process);
}

static int
camluv_parse_process_options(value camluv_options,
                             uv_process_options_t *uv_options)
{
  CAMLparam1(camluv_options);
  CAMLlocal2(camluv_args, camluv_env);

  int i;
  char **args, **env;
  int args_len = 0, env_len = 0;

  if (uv_options == NULL) return -1;

  uv_options->exit_cb = camluv_exit_cb;

  uv_options->file = String_val(Field(camluv_options, 1));

  camluv_args = Field(camluv_options, 2);
  args_len = Wosize_val(camluv_args);
  args = (char **)malloc(sizeof(char *) * (args_len + 1));
  memset(args, 0, sizeof(char *) * (args_len + 1));
  for (i = 0; i < args_len; i++) {
    args[i] = String_val(Field(camluv_args, i));
  }
  uv_options->args = args;

  camluv_env = Field(camluv_options, 3);
  env_len = Wosize_val(camluv_env);
  env = (char **)malloc(sizeof(char *) * env_len);
  memset(env, 0, sizeof(char *) * (env_len + 1));
  for (i = 0; i < env_len; i++) {
    env[i] = String_val(Field(camluv_env, i));
  }
  uv_options->env = env;

  uv_options->cwd = String_val(Field(camluv_options, 4));
  uv_options->flags = Int_val(Field(camluv_options, 5));

  // TODO: parse uid and gid;

  CAMLreturn(0);
}

static void
camluv_exit_cb(uv_process_t* uv_handle, int64_t exit_status, int term_signal)
{
  camluv_enter_callback();

  CAMLlocal4(exit_cb,
             camluv_handle,
             camluv_exit_status,
             camluv_term_signal);

  camluv_process_t *camluv_process =(camluv_process_t *)(uv_handle->data);
  exit_cb = camluv_process->exit_cb;
  camluv_handle = camluv_copy_process(camluv_process);
  camluv_exit_status = caml_copy_int64(exit_status);
  camluv_term_signal = Val_int(term_signal);

  callback3(exit_cb,
            camluv_handle,
            camluv_exit_status,
            camluv_term_signal);

  camluv_leave_callback();
}

static camluv_process_t *
camluv_process_new(void)
{
  camluv_process_t *process = (camluv_process_t *)malloc(sizeof(camluv_process_t));
  if (!process) return NULL;

  return process;
}

CAMLprim value
camluv_process_spawn(value loop, value options)
{
  CAMLparam2(loop, options);
  CAMLlocal1(process);

  uv_process_options_t process_options;
  camluv_loop_t *camluv_loop = camluv_loop_struct_val(loop);
  camluv_process_t *camluv_process = camluv_process_new();

  (camluv_process->uv_process).data = camluv_process;
  camluv_process->exit_cb = Field(options, 0);

  ((camluv_handle_t *)camluv_process)->uv_handle =
                              (uv_handle_t *)&(camluv_process->uv_process);
  camluv_init_handle_with_loop((camluv_handle_t *)
                               (&(camluv_process->camluv_handle)),
                               camluv_loop);

  camluv_parse_process_options(options, &process_options);

  int rc = uv_spawn(camluv_loop->uv_loop,
                    &(camluv_process->uv_process),
                    &process_options);
  if (rc != UV_OK) {
    // TODO: error handling.
  }
  process = camluv_copy_process(camluv_process);

  CAMLreturn(process);
}

CAMLprim value
camluv_process_kill(value process, value signum)
{
  CAMLparam2(process, signum);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  camluv_process_t *camluv_process = camluv_process_struct_val(process);
  rc = uv_process_kill(&(camluv_process->uv_process), Int_val(signum));
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_pid_kill(value pid, value signum)
{
  CAMLparam2(pid, signum);
  CAMLlocal1(camluv_rc);

  int rc = -1;
  rc = uv_kill(Int_val(pid), Int_val(signum));
  camluv_rc = camluv_errno_c2ml(rc);

  CAMLreturn(camluv_rc);
}

CAMLprim value
camluv_process_getpid(value process)
{
  CAMLparam1(process);
  CAMLlocal1(camluv_pid);

  camluv_process_t *camluv_process = camluv_process_struct_val(process);

  camluv_pid = Val_int((camluv_process->uv_process).pid);

  CAMLreturn(camluv_pid);
}

