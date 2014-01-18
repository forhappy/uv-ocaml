type loop
type handle
type idle
type timer
type async
type check
type prepare
type poll
type signal
type fs_event
type fs_poll
type tcp
type udp
type pipe
type tty
type thread
type key
type mutex
type rwlock
type sem
type condition
type barrier
type uv_buffer = { base : string; len : int; }
type uv_sockaddr = {
  host : string;
  port : int;
  scope_id : int;
  flowinfo : int;
}
type uv_buffer_array = uv_buffer array
type uv_win_size = int * int
type uv_timestamp = { tv_sec : int32; tv_nsec : int32; }
type uv_stat = {
  st_dev : int64;
  st_mode : int64;
  st_nlink : int64;
  st_uid : int64;
  st_gid : int64;
  st_rdev : int64;
  st_ino : int64;
  st_size : int64;
  st_blksize : int64;
  st_blocks : int64;
  st_flags : int64;
  st_gen : int64;
  st_atim : uv_timestamp;
  st_mtim : uv_timestamp;
  st_ctim : uv_timestamp;
  st_birthtim : uv_timestamp;
}
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
type uv_fs_type =
    UV_FS_UNKNOWN
  | UV_FS_CUSTOM
  | UV_FS_OPEN
  | UV_FS_CLOSE
  | UV_FS_READ
  | UV_FS_WRITE
  | UV_FS_SENDFILE
  | UV_FS_STAT
  | UV_FS_LSTAT
  | UV_FS_FSTAT
  | UV_FS_FTRUNCATE
  | UV_FS_UTIME
  | UV_FS_FUTIME
  | UV_FS_CHMOD
  | UV_FS_FCHMOD
  | UV_FS_FSYNC
  | UV_FS_FDATASYNC
  | UV_FS_UNLINK
  | UV_FS_RMDIR
  | UV_FS_MKDIR
  | UV_FS_RENAME
  | UV_FS_READDIR
  | UV_FS_LINK
  | UV_FS_SYMLINK
  | UV_FS_READLINK
  | UV_FS_CHOWN
  | UV_FS_FCHOWN
type uv_handle_type =
    UV_UNKNOWN_HANDLE
  | UV_ASYNC
  | UV_CHECK
  | UV_FS_EVENT
  | UV_FS_POLL
  | UV_HANDLE
  | UV_IDLE
  | UV_NAMED_PIPE
  | UV_POLL
  | UV_PREPARE
  | UV_PROCESS
  | UV_STREAM
  | UV_TCP
  | UV_TIMER
  | UV_TTY
  | UV_UDP
  | UV_SIGNAL
  | UV_FILE
  | UV_HANDLE_TYPE_MAX
type uv_run_mode = UV_RUN_DEFAULT | UV_RUN_ONCE | UV_RUN_NOWAIT
type uv_udp_flags = UV_UDP_IPV6ONLY | UV_UDP_PARTIAL
type uv_membership = UV_LEAVE_GROUP | UV_JOIN_GROUP
type uv_poll_event = UV_READABLE | UV_WRITABLE | UV_RDWRABLE
type uv_fs_event_flags =
    UV_FS_EVENT_WATCH_ENTRY
  | UV_FS_EVENT_STAT
  | UV_FS_EVENT_RECURSIVE
type uv_process_flags =
    UV_PROCESS_SETUID
  | UV_PROCESS_SETGID
  | UV_PROCESS_WINDOWS_VERBATIM_ARGUMENTS
  | UV_PROCESS_DETACHED
  | UV_PROCESS_WINDOWS_HIDE
type uv_walk_cb = handle -> string -> unit
type uv_close_cb = handle -> unit
type uv_idle_cb = idle -> int -> unit
type uv_timer_cb = timer -> int -> unit
type uv_async_cb = async -> int -> unit
type uv_check_cb = check -> int -> unit
type uv_prepare_cb = prepare -> int -> unit
type uv_poll_cb = poll -> int -> uv_poll_event -> unit
type uv_signal_cb = signal -> int -> unit
type uv_fs_event_cb = fs_event -> string -> int -> int -> unit
type uv_fs_poll_cb = fs_poll -> int -> uv_stat -> uv_stat -> unit
type uv_tcp_connect_cb = tcp -> int -> unit
type uv_tcp_connection_cb = tcp -> int -> unit
type uv_tcp_close_cb = tcp -> unit
type uv_tcp_shutdown_cb = tcp -> int -> unit
type uv_tcp_read_cb = tcp -> int -> uv_buffer_array -> unit
type uv_tcp_write_cb = tcp -> int -> unit
type uv_udp_close_cb = udp -> unit
type uv_udp_send_cb = udp -> int -> unit
type uv_udp_recv_cb =
    udp -> int -> uv_buffer_array -> uv_sockaddr -> uv_udp_flags -> unit
type uv_pipe_connect_cb = pipe -> int -> unit
type uv_pipe_connection_cb = pipe -> int -> unit
type uv_pipe_close_cb = pipe -> unit
type uv_pipe_shutdown_cb = pipe -> int -> unit
type uv_pipe_read_cb = pipe -> int -> uv_buffer_array -> unit
type uv_pipe_read2_cb =
    pipe -> int -> uv_buffer_array -> uv_handle_type -> unit
type uv_pipe_write_cb = pipe -> int -> unit
type uv_tty_read_cb = tty -> int -> uv_buffer_array -> unit
type uv_tty_write_cb = tty -> int -> unit
type uv_tty_close_cb = tty -> unit
type uv_tty_shutdown_cb = tty -> int -> unit
type uv_work_cb = unit -> unit
type uv_after_work_cb = int -> unit
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
    external queue_work : loop -> uv_work_cb -> uv_after_work_cb -> uv_errno
      = "camluv_loop_queue_work"
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
module Poll :
  sig
    external init : loop -> int -> poll = "camluv_poll_init"
    external create : loop -> int -> poll = "camluv_poll_init"
    external start : poll -> uv_poll_event -> uv_poll_cb -> uv_errno
      = "camluv_poll_start"
    external fileno : poll -> int = "camluv_poll_fileno"
    external stop : poll -> uv_errno = "camluv_poll_stop"
  end
module Signal :
  sig
    external init : loop -> signal = "camluv_signal_init"
    external create : loop -> signal = "camluv_signal_init"
    external start : signal -> uv_signal_cb -> int -> uv_errno
      = "camluv_signal_start"
    external stop : signal -> uv_errno = "camluv_signal_stop"
  end
module FsEvent :
  sig
    external init : loop -> fs_event = "camluv_fs_event_init"
    external create : loop -> fs_event = "camluv_fs_event_init"
    external start : fs_event -> uv_fs_event_cb -> string -> int -> uv_errno
      = "camluv_fs_event_start"
    external stop : fs_event -> uv_errno = "camluv_fs_event_stop"
  end
module FsPoll :
  sig
    external init : loop -> fs_poll = "camluv_fs_poll_init"
    external create : loop -> fs_poll = "camluv_fs_poll_init"
    external start : fs_poll -> uv_fs_poll_cb -> string -> int -> uv_errno
      = "camluv_fs_poll_start"
    external stop : fs_poll -> uv_errno = "camluv_fs_poll_stop"
  end
module TCP :
  sig
    external init : loop -> int -> int -> tcp = "camluv_tcp_init"
    external create : loop -> int -> int = "camluv_tcp_init"
    external start_read : tcp -> uv_tcp_read_cb -> uv_errno
      = "camluv_tcp_start_read"
    external stop_read : tcp -> uv_errno = "camluv_tcp_stop_read"
    external write : tcp -> uv_buffer_array -> uv_tcp_write_cb -> uv_errno
      = "camluv_tcp_start_write"
    external window_size : tcp -> uv_win_size = "camluv_tcp_get_winsize"
    external close : tcp -> uv_tcp_close_cb -> unit = "camluv_tcp_close"
    external shutdown : tcp -> uv_tcp_shutdown_cb -> uv_errno
      = "camluv_tcp_shutdown"
    external is_readable : tcp -> int = "camluv_tcp_is_readable"
    external is_writable : tcp -> int = "camluv_tcp_is_writable"
    external write_queue_size : tcp -> int = "camluv_tcp_write_queue_size"
    external set_blocking : tcp -> int = "camluv_tcp_set_blocking"
    external is_closing : tcp -> int = "camluv_tcp_is_closing"
    external is_active : tcp -> int = "camluv_tcp_is_active"
    external ref : tcp -> unit = "camluv_tcp_ref"
    external unref : tcp -> unit = "camluv_tcp_unref"
    external has_ref : tcp -> int = "camluv_tcp_has_ref"
    external loop : tcp -> loop = "camluv_tcp_loop"
  end
module TTY :
  sig
    external init : loop -> int -> int -> tty = "camluv_tty_init"
    external create : loop -> int -> int = "camluv_tty_init"
    external set_mode : tty -> int -> uv_errno = "camluv_tty_set_mode"
    external reset_mode : unit -> uv_errno = "camluv_tty_reset_mode"
    external start_read : tty -> uv_tty_read_cb -> uv_errno
      = "camluv_tty_start_read"
    external stop_read : tty -> uv_errno = "camluv_tty_stop_read"
    external write : tty -> uv_buffer_array -> uv_tty_write_cb -> uv_errno
      = "camluv_tty_start_write"
    external window_size : tty -> uv_win_size = "camluv_tty_get_winsize"
    external close : tty -> uv_tty_close_cb -> unit = "camluv_tty_close"
    external shutdown : tty -> uv_tty_shutdown_cb -> uv_errno
      = "camluv_tty_shutdown"
    external is_readable : tty -> int = "camluv_tty_is_readable"
    external is_writable : tty -> int = "camluv_tty_is_writable"
    external write_queue_size : tty -> int = "camluv_tty_write_queue_size"
    external set_blocking : tty -> int = "camluv_tty_set_blocking"
    external is_closing : tty -> int = "camluv_tty_is_closing"
    external is_active : tty -> int = "camluv_tty_is_active"
    external ref : tty -> unit = "camluv_tty_ref"
    external unref : tty -> unit = "camluv_tty_unref"
    external has_ref : tty -> int = "camluv_tty_has_ref"
    external loop : tty -> loop = "camluv_tty_loop"
  end
module Thread :
  sig
    external init : ('a -> unit) -> 'a -> thread = "camluv_thread_init"
    external create : ('a -> unit) -> 'a -> thread = "camluv_thread_init"
    external self : unit -> int = "camluv_thread_self"
    external join : thread -> uv_errno = "camluv_thread_join"
  end
module Key :
  sig
    external init : unit -> key = "camluv_key_init"
    external create : unit -> key = "camluv_key_init"
    external delete : key -> unit = "camluv_key_delete"
    external get : key -> 'a = "camluv_key_get"
    external set : key -> 'a -> unit = "camluv_key_set"
  end
module Mutex :
  sig
    external init : unit -> mutex = "camluv_mutex_init"
    external create : unit -> mutex = "camluv_mutex_init"
    external destroy : mutex -> unit = "camluv_mutex_destroy"
    external lock : mutex -> unit = "camluv_mutex_lock"
    external unlock : mutex -> unit = "camluv_mutex_unlock"
    external trylock : mutex -> int = "camluv_mutex_trylock"
  end
module RWlock :
  sig
    external init : unit -> rwlock = "camluv_rwlock_init"
    external create : unit -> rwlock = "camluv_rwlock_init"
    external destroy : rwlock -> unit = "camluv_rwlock_destroy"
    external rdlock : rwlock -> unit = "camluv_rwlock_rdlock"
    external rdunlock : rwlock -> unit = "camluv_rwlock_rdunlock"
    external tryrdlock : rwlock -> int = "camluv_rwlock_tryrdlock"
    external wrlock : rwlock -> unit = "camluv_rwlock_wrlock"
    external wrunlock : rwlock -> unit = "camluv_rwlock_wrunlock"
    external trywrlock : rwlock -> int = "camluv_rwlock_trywrlock"
  end
module Semaphore :
  sig
    external init : int -> sem = "camluv_sem_init"
    external create : int -> sem = "camluv_sem_init"
    external destroy : sem -> unit = "camluv_sem_destroy"
    external post : sem -> unit = "camluv_sem_post"
    external wait : sem -> unit = "camluv_sem_wait"
    external trywait : sem -> int = "camluv_sem_trywait"
  end
module Condition :
  sig
    external init : unit -> condition = "camluv_condition_init"
    external create : unit -> condition = "camluv_condition_init"
    external destroy : condition -> unit = "camluv_condition_destroy"
    external signal : condition -> unit = "camluv_condition_signal"
    external broadcast : condition -> unit = "camluv_condition_broadcast"
    external wait : condition -> mutex -> unit = "camluv_condition_wait"
  end
module Barrier :
  sig
    external init : int -> barrier = "camluv_barrier_init"
    external create : int -> barrier = "camluv_barrier_init"
    external destroy : barrier -> unit = "camluv_barrier_destroy"
    external wait : barrier -> unit = "camluv_barrier_wait"
  end
module Util :
  sig
    external strerror : int -> string = "camluv_strerror"
    external err_name : int -> string = "camluv_err_name"
  end
