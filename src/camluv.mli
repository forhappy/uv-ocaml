type loop
type handle
type idle
type timer
type async
type check
type prepare
type uv_errno =
    UV_OK
  | UV_E2BIG
  | UV_EACCES
  | UV_EADDRINUSE
  | UV_EADDRNOTAVAIL
  | UV_EAFNOSUPPORT
  | UV_EAGAIN
  | UV_EAI_ADDRFAMILY
  | UV_EAI_AGAIN
  | UV_EAI_BADFLAGS
  | UV_EAI_BADHINTS
  | UV_EAI_CANCELED
  | UV_EAI_FAIL
  | UV_EAI_FAMILY
  | UV_EAI_MEMORY
  | UV_EAI_NODATA
  | UV_EAI_NONAME
  | UV_EAI_OVERFLOW
  | UV_EAI_PROTOCOL
  | UV_EAI_SERVICE
  | UV_EAI_SOCKTYPE
  | UV_EAI_SYSTEM
  | UV_EALREADY
  | UV_EBADF
  | UV_EBUSY
  | UV_ECANCELED
  | UV_ECHARSET
  | UV_ECONNABORTED
  | UV_ECONNREFUSED
  | UV_ECONNRESET
  | UV_EDESTADDRREQ
  | UV_EEXIST
  | UV_EFAULT
  | UV_EHOSTUNREACH
  | UV_EINTR
  | UV_EINVAL
  | UV_EIO
  | UV_EISCONN
  | UV_EISDIR
  | UV_ELOOP
  | UV_EMFILE
  | UV_EMSGSIZE
  | UV_ENAMETOOLONG
  | UV_ENETDOWN
  | UV_ENETUNREACH
  | UV_ENFILE
  | UV_ENOBUFS
  | UV_ENODEV
  | UV_ENOENT
  | UV_ENOMEM
  | UV_ENONET
  | UV_ENOSPC
  | UV_ENOSYS
  | UV_ENOTCONN
  | UV_ENOTDIR
  | UV_ENOTEMPTY
  | UV_ENOTSOCK
  | UV_ENOTSUP
  | UV_EPERM
  | UV_EPIPE
  | UV_EPROTO
  | UV_EPROTONOSUPPORT
  | UV_EPROTOTYPE
  | UV_EROFS
  | UV_ESHUTDOWN
  | UV_ESPIPE
  | UV_ESRCH
  | UV_ETIMEDOUT
  | UV_EXDEV
  | UV_UNKNOWN
  | UV_EOF
type uv_run_mode = UV_RUN_DEFAULT | UV_RUN_ONCE | UV_RUN_NOWAIT
type uv_walk_cb = handle -> string -> unit
type uv_close_cb = handle -> unit
type uv_idle_cb = idle -> int -> unit
type uv_timer_cb = timer -> int -> unit
type uv_async_cb = async -> int -> unit
type uv_check_cb = check -> int -> unit
type uv_prepare_cb = prepare -> int -> unit
module Loop :
  sig
    external create : unit -> loop = "camluv_loop_new"
    external default : unit -> loop = "camluv_loop_default"
    external delete : loop -> unit = "camluv_loop_delete"
    external run : loop -> uv_run_mode -> uv_errno = "camluv_loop_run"
    external stop : loop -> unit = "camluv_loop_stop"
    external now : loop -> int64 = "camluv_loop_now"
    external update_time : loop -> unit = "camluv_loop_update_time"
    external backend_fd : loop -> int32 = "camluv_loop_backend_fd"
    external backend_timeout : loop -> int32 = "camluv_loop_backend_timeout"
    external walk : loop -> uv_walk_cb -> string -> unit = "camluv_loop_walk"
  end
module Handle :
  sig
    external close : handle -> uv_close_cb -> unit = "camluv_close"
    external closed : handle -> int = "camluv_is_closing"
    external active : handle -> int = "camluv_is_active"
    external ref : handle -> unit = "camluv_ref"
    external unref : handle -> unit = "camluv_unref"
    external has_ref : handle -> int = "camluv_has_ref"
    external loop : handle -> loop = "camluv_loop"
  end
module Idle :
  sig
    external init : loop -> idle = "camluv_idle_init"
    external create : loop -> idle = "camluv_idle_init"
    external start : idle -> uv_idle_cb -> uv_errno = "camluv_idle_start"
    external stop : idle -> uv_errno = "camluv_idle_stop"
  end
module Timer :
  sig
    external init : loop -> timer = "camluv_timer_init"
    external create : loop -> timer = "camluv_timer_init"
    external start : timer -> uv_timer_cb -> int64 -> int64 -> uv_errno
      = "camluv_timer_start"
    external stop : timer -> uv_errno = "camluv_timer_stop"
    external again : timer -> uv_errno = "camluv_timer_again"
    external set_repeat : timer -> int64 -> unit = "camluv_timer_set_repeat"
    external get_repeat : timer -> int64 = "camluv_timer_get_repeat"
  end
module Async :
  sig
    external init : loop -> async = "camluv_async_init"
    external create : loop -> async = "camluv_async_init"
    external start : async -> uv_async_cb -> uv_errno = "camluv_async_start"
    external stop : async -> uv_errno = "camluv_async_stop"
  end
module Check :
  sig
    external init : loop -> check = "camluv_check_init"
    external create : loop -> check = "camluv_check_init"
    external start : check -> uv_check_cb -> uv_errno = "camluv_check_start"
    external stop : check -> uv_errno = "camluv_check_stop"
  end
module Prepare :
  sig
    external init : loop -> prepare = "camluv_prepare_init"
    external create : loop -> prepare = "camluv_prepare_init"
    external start : prepare -> uv_prepare_cb -> uv_errno
      = "camluv_prepare_start"
    external stop : prepare -> uv_errno = "camluv_prepare_stop"
  end
