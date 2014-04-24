
type uv_buffer = {base: string; len: int}

type uv_sockaddr = {host: string; port: int; scope_id: int; flowinfo: int}

type uv_errno =
  | UV_OK
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

type uv_stdio_flags = (** UNUSED **)
  | UV_IGNORE
  | UV_CREATE_PIPE
  | UV_INHERIT_FD
  | UV_INHERIT_STREAM
  | UV_READABLE_PIPE
  | UV_WRITABLE_PIPE

module rec Loop :
  sig 
    type t

    type run_mode = UV_RUN_DEFAULT | UV_RUN_ONCE | UV_RUN_NOWAIT

    type cb_work = unit -> unit
    type cb_after_work = int -> unit

    val create: unit -> t
    val default: unit -> t
    val delete: t -> unit
    val run: t -> run_mode -> uv_errno
    val stop: t -> unit
    val now: t -> int64
    val update_time: t -> unit
    val backend_fd: t -> int32
    val backend_timeout: t -> int32
    val walk: t -> Handle.cb_walk -> string -> unit
    val queue_work: t -> cb_work -> cb_after_work -> uv_errno
  end = struct
    type t

    type run_mode =
      | UV_RUN_DEFAULT
      | UV_RUN_ONCE
      | UV_RUN_NOWAIT

    type cb_work = unit -> unit
    type cb_after_work = int -> unit

    external create: unit->t = "camluv_loop_new"
    external default: unit->t = "camluv_loop_default"
    external delete: t -> unit = "camluv_loop_delete"
    external run: t -> run_mode -> uv_errno = "camluv_loop_run"
    external stop: t -> unit = "camluv_loop_stop"
    external now: t -> int64 = "camluv_loop_now"
    external update_time: t -> unit = "camluv_loop_update_time"
    external backend_fd: t -> int32 = "camluv_loop_backend_fd"
    external backend_timeout: t -> int32 = "camluv_loop_backend_timeout"
    external walk: t -> Handle.cb_walk -> string -> unit = "camluv_loop_walk"
    external queue_work: t -> cb_work -> cb_after_work -> uv_errno = "camluv_loop_queue_work"
  end

and Handle :
  sig
    type t

    type t_type =
      | UV_UNKNOWN_HANDLE
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

    type cb_walk = t -> string -> unit
    type cb_close = t -> unit

    val close: t -> cb_close -> unit
    val closed: t -> int
    val active: t -> int
    val ref: t -> unit
    val unref: t -> unit
    val has_ref: t -> int
    val loop: t -> Loop.t
  end = struct
    type t

    type t_type =
      | UV_UNKNOWN_HANDLE
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

    type cb_walk = t -> string -> unit
    type cb_close = t -> unit

    external close: t -> cb_close -> unit = "camluv_close"
    external closed: t -> int = "camluv_is_closing"
    external active: t -> int = "camluv_is_active"
    external ref: t -> unit = "camluv_ref"
    external unref: t -> unit = "camluv_unref"
    external has_ref: t -> int = "camluv_has_ref"
    external loop: t -> Loop.t = "camluv_loop"
  end

module Idle =
  struct
    type t
  
    type cb = t -> int -> unit

    external init: Loop.t -> t = "camluv_idle_init"
    external create: Loop.t -> t = "camluv_idle_init"
    external start: t -> cb -> uv_errno = "camluv_idle_start"
    external stop: t -> uv_errno = "camluv_idle_stop"
  end

module Timer =
  struct
    type t

    type cb = t -> int -> unit

    external init: Loop.t -> t = "camluv_timer_init"
    external create: Loop.t -> t = "camluv_timer_init"
    external start: t -> cb -> int64 -> int64 -> uv_errno = "camluv_timer_start"
    external stop: t -> uv_errno = "camluv_timer_stop"
    external again: t -> uv_errno  = "camluv_timer_again"
    external set_repeat: t -> int64 -> unit = "camluv_timer_set_repeat"
    external get_repeat: t -> int64 = "camluv_timer_get_repeat"
  end

module Async =
  struct
    type t

    type cb = t -> int -> unit

    external init: Loop.t -> t = "camluv_async_init"
    external create: Loop.t -> t = "camluv_async_init"
    (* Does Not Exist
     * external start: t -> cb -> uv_errno = "camluv_async_start" *)
    external stop: t -> uv_errno = "camluv_async_stop"
  end

module Check =
  struct

    type t

    type cb = t -> int -> unit

    external init: Loop.t -> t = "camluv_check_init"
    external create: Loop.t -> t = "camluv_check_init"
    external start: t -> cb -> uv_errno = "camluv_check_start"
    external stop: t -> uv_errno = "camluv_check_stop"
  end

module Prepare =
  struct

    type t

    type cb = t -> int -> unit

    external init: Loop.t -> t = "camluv_prepare_init"
    external create: Loop.t -> t = "camluv_prepare_init"
    external start: t -> cb -> uv_errno = "camluv_prepare_start"
    external stop: t -> uv_errno = "camluv_prepare_stop"
  end

module Poll =
  struct
    type t

    type event = UV_READABLE | UV_WRITABLE | UV_RDWRABLE

    type cb = t -> int -> event -> unit

    external init: Loop.t -> int -> t = "camluv_poll_init"
    external create: Loop.t -> int -> t = "camluv_poll_init"
    external start: t -> event -> cb -> uv_errno = "camluv_poll_start"
    external fileno: t -> int = "camluv_poll_fileno"
    external stop: t -> uv_errno = "camluv_poll_stop"
  end

module Fs =
  struct
    type t

    type cb = t -> unit

    type timestamp = {
      tv_sec: int32;
      tv_nsec: int32;
    }

    type stat = {
      st_dev: int64;
      st_mode: int64;
      st_nlink: int64;
      st_uid: int64;
      st_gid: int64;
      st_rdev: int64;
      st_ino: int64;
      st_size: int64;
      st_blksize: int64;
      st_blocks: int64;
      st_flags: int64;
      st_gen: int64;
      st_atim: timestamp;
      st_mtim: timestamp;
      st_ctim: timestamp;
      st_birthtim: timestamp;
    }

    external close: Loop.t -> int -> cb -> unit= "camluv_fs_close"
    external openfile: Loop.t -> string -> int -> int -> cb -> uv_errno = "camluv_fs_open_native" "camluv_fs_open_bytecode"
    external read: Loop.t -> int -> int -> int -> cb -> string = "camluv_fs_read_native" "camluv_fs_read_bytecode"
    external unlink: Loop.t -> string -> cb -> uv_errno = "camluv_fs_unlink"
    external write: Loop.t -> int -> string -> int -> int -> cb -> uv_errno = "camluv_fs_write_bytecode" "camluv_fs_write_native"
    external mkdir: Loop.t -> string -> int -> cb -> uv_errno = "camluv_fs_mkdir"
    external rmdir: Loop.t -> string -> cb -> uv_errno = "camluv_fs_rmdir"
    external readdir: Loop.t -> string -> int -> cb -> uv_errno = "camluv_fs_readdir"
    external stat: Loop.t -> string -> cb -> uv_errno = "camluv_fs_stat"
    external fstat: Loop.t -> int -> cb -> uv_errno = "camluv_fs_fstat"
    external rename: Loop.t -> string -> string -> cb -> uv_errno = "camluv_fs_rename"
    external fsync: Loop.t -> int -> cb -> uv_errno = "camluv_fs_fsync"
    external fdatasync: Loop.t -> int -> cb -> uv_errno = "camluv_fs_fdatasync"
    external ftruncate: Loop.t -> int -> int -> cb -> uv_errno = "camluv_fs_ftruncate"
    external sendfile: Loop.t -> int -> int -> int -> int -> cb -> uv_errno = "camluv_fs_sendfile_native" "camluv_fs_sendfile_bytecode"
    external chmod: Loop.t -> string -> int -> cb -> uv_errno = "camluv_fs_chmod"
    external utime: Loop.t -> string -> float -> float -> cb -> uv_errno = "camluv_fs_utime_native" "camluv_fs_utime_bytecode"
    external futime: Loop.t -> int -> float -> float -> cb -> uv_errno = "camluv_fs_futime_native" "camluv_fs_futime_bytecode"
    external lstat: Loop.t -> string -> cb -> uv_errno = "camluv_fs_lstat"
    external link: Loop.t -> string -> string -> cb -> uv_errno = "camluv_fs_link"
    external symlink: Loop.t -> string -> string -> int -> cb -> uv_errno = "camluv_fs_symlink_native" "camluv_fs_symlink_bytecode"
    external readlink: Loop.t -> string -> cb -> uv_errno = "camluv_fs_readlink"
    external fchmod: Loop.t -> int -> int -> cb -> uv_errno = "camluv_fs_fchmod"
    external chown: Loop.t -> string -> int -> int -> cb -> uv_errno = "camluv_fs_chown_native" "camluv_fs_chown_bytecode"
    external fchown: Loop.t -> int -> int -> int -> cb -> uv_errno = "camluv_fs_fchown_native" "camluv_fs_fchown_bytecode"
    external get_result: t -> int = "camluv_fs_get_result"
    external get_path: t -> string = "camluv_fs_get_path"
    external get_stat: t -> stat = "camluv_fs_get_stat"
    external get_loop: t -> Loop.t = "camluv_fs_get_loop"
    external clean: t -> unit = "camluv_fs_req_cleanup"
  end

module Signal =
  struct
    type t

    type cb = t -> int -> unit

    external init: Loop.t -> t = "camluv_signal_init"
    external create: Loop.t -> t = "camluv_signal_init"
    external start: t -> cb -> int -> uv_errno = "camluv_signal_start"
    external stop: t -> uv_errno = "camluv_signal_stop"
  end

module FsEvent =
  struct
    type t

    type cb = t -> string -> int -> int -> unit

    type uv_fs_type = (** UNUSED **)
      | UV_FS_UNKNOWN
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

    type uv_fs_event_flags = (** UNUSED **)
        UV_FS_EVENT_WATCH_ENTRY
      | UV_FS_EVENT_STAT
      | UV_FS_EVENT_RECURSIVE

    external init: Loop.t -> t = "camluv_fs_event_init"
    external create: Loop.t -> t = "camluv_fs_event_init"
    external start: t -> cb -> string -> int -> uv_errno = "camluv_fs_event_start"
    external stop: t -> uv_errno = "camluv_fs_event_stop"
  end

module FsPoll =
  struct
    type t

    type cb = t -> int -> Fs.stat -> Fs.stat -> unit

    external init: Loop.t -> t = "camluv_fs_poll_init"
    external create: Loop.t -> t = "camluv_fs_poll_init"
    external start: t -> cb -> string -> int -> uv_errno = "camluv_fs_poll_start"
    external stop: t -> uv_errno = "camluv_fs_poll_stop"
  end

module TCP =
  struct
    type t

    type cb_connect = t -> int -> unit
    type cb_connection = t -> int -> unit
    type cb_close = t -> unit
    type cb_shutdown = t -> int -> unit
    type cb_read = t -> int -> uv_buffer array-> unit
    type cb_write = t -> int -> unit

    external init: Loop.t -> t = "camluv_tcp_init"
    external create: Loop.t -> t = "camluv_tcp_init"
    external openfd: t -> int -> uv_errno = "camluv_tcp_open"
    external bind: t -> uv_sockaddr -> uv_errno = "camluv_tcp_bind"
    external listen: t -> int -> cb_connection -> uv_errno = "camluv_tcp_listen"
    external accept: t -> t = "camluv_tcp_accept"
    external connect: t -> uv_sockaddr -> cb_connect -> uv_errno = "camluv_tcp_connect"
    external getsockname: t -> uv_sockaddr = "camluv_tcp_getsockname"
    external getpeername: t -> uv_sockaddr = "camluv_tcp_getpeername"
    external nodelay: t -> int -> uv_errno = "camluv_tcp_nodelay"
    external keepalive: t -> int -> int -> uv_errno = "camluv_tcp_nodelay"
    external simultaneous_accepts: Loop.t -> int -> uv_errno = "camluv_tcp_simultaneous_accepts"
    external start_read: t -> cb_read -> uv_errno = "camluv_tcp_start_read"
    (* DOES NOT EXIST
     * external stop_read: t -> uv_errno = "camluv_tcp_stop_read" *)
    external write: t -> uv_buffer array-> cb_write -> uv_errno = "camluv_tcp_start_write"
    external close: t -> cb_close -> unit = "camluv_tcp_close"
    external shutdown: t -> cb_shutdown -> uv_errno = "camluv_tcp_shutdown"
    external is_readable: t -> int = "camluv_tcp_is_readable"
    external is_writable: t -> int = "camluv_tcp_is_writable"
    external write_queue_size: t -> int = "camluv_tcp_write_queue_size"
    external set_blocking: t -> int = "camluv_tcp_set_blocking"
    external is_closing: t -> int = "camluv_tcp_is_closing"
    external is_active: t -> int = "camluv_tcp_is_active"
    external ref: t -> unit = "camluv_tcp_ref"
    external unref: t -> unit = "camluv_tcp_unref"
    external has_ref: t -> int = "camluv_tcp_has_ref"
    external loop: t -> Loop.t = "camluv_tcp_loop"
  end

module UDP =
  struct
    type t

    type flags =
      | UV_UDP_IPV6ONLY
      | UV_UDP_PARTIAL

    type membership =
      | UV_LEAVE_GROUP
      | UV_JOIN_GROUP

    type cb_close = t -> unit
    type cb_send = t -> int -> unit
    type cb_recv = t -> int -> uv_buffer array-> uv_sockaddr -> flags -> unit

    external init: Loop.t -> t = "camluv_udp_init"
    external create: Loop.t -> t = "camluv_udp_init"
    external openfd: t -> int -> uv_errno = "camluv_udp_open"
    external bind: t -> uv_sockaddr -> uv_errno = "camluv_udp_bind"
    external getsockname: t -> uv_sockaddr = "camluv_udp_getsockname"
    external set_multicast_loop: t -> int -> uv_errno = "camluv_udp_set_multicast_loop"
    external set_multicast_ttl: t -> int -> uv_errno = "camluv_udp_set_multicast_ttl"
    external set_broadcast: t -> int -> uv_errno = "camluv_udp_set_broadcast"
    external set_ttl: t -> int -> uv_errno = "camluv_udp_set_ttl"
    external set_membership: t -> string -> string -> membership -> uv_errno = "camluv_udp_set_membership"
    external start_recv: t -> cb_recv -> uv_errno = "camluv_udp_start_recv"
    external stop_recv: t -> uv_errno = "camluv_udp_stop_recv"
    external send: t -> uv_sockaddr -> uv_buffer array-> cb_send -> uv_errno = "camluv_udp_send"
    external close: t -> cb_close -> unit = "camluv_udp_close"
    external is_closing: t -> int = "camluv_udp_is_closing"
    external is_active: t -> int = "camluv_udp_is_active"
    external ref: t -> unit = "camluv_udp_ref"
    external unref: t -> unit = "camluv_udp_unref"
    external has_ref: t -> int = "camluv_udp_has_ref"
    external loop: t -> Loop.t = "camluv_udp_loop"
  end

module Pipe =
  struct

    type t

    type cb_connect = t -> int -> unit
    type cb_connection = t -> int -> unit
    type cb_close = t -> unit
    type cb_shutdown = t -> int -> unit
    type cb_read = t -> int -> uv_buffer array-> unit
    type cb_read2 = t -> int -> uv_buffer array-> Handle.t -> unit
    type cb_write = t -> int -> unit

    external init: Loop.t -> t = "camluv_pipe_init"
    external create: Loop.t -> t = "camluv_pipe_init"
    external openfd: t -> int -> uv_errno = "camluv_pipe_open"
    external bind: t -> string -> uv_errno = "camluv_pipe_bind"
    external listen: t -> int -> cb_connection -> uv_errno = "camluv_pipe_listen"
    external accept: t -> t = "camluv_pipe_accept"
    external connect: t -> string -> cb_connect -> uv_errno = "camluv_pipe_connect"
    external start_read: t -> cb_read -> uv_errno = "camluv_pipe_start_read"
    external start_read2: t -> cb_read2 -> uv_errno = "camluv_pipe_start_read2"
    (* DOES NOT EXIST
     * external getsockname: t -> uv_sockaddr = "camluv_pipe_getsockname"
     * external getpeername: t -> uv_sockaddr = "camluv_pipe_getpeername"
     * external nodelay: t -> int -> uv_errno = "camluv_pipe_nodelay"
     * external keepalive: t -> int -> int -> uv_errno = "camluv_pipe_nodelay"
     * external simultaneous_accepts: Loop.t -> int -> uv_errno = "camluv_pipe_simultaneous_accepts"
     * external stop_read: t -> uv_errno = "camluv_pipe_stop_read"
     * external write: t -> uv_buffer array-> cb_write -> uv_errno = "camluv_pipe_start_write"
     * external write_tcp: t -> uv_buffer array-> TCP.t -> cb_write -> uv_errno = "camluv_pipe_start_write2_tcp"
     * external write_pipe: t -> uv_buffer array-> TCP.t -> cb_write -> uv_errno = "camluv_pipe_start_write" *)
    external close: t -> cb_close -> unit = "camluv_pipe_close"
    external shutdown: t -> cb_shutdown -> uv_errno = "camluv_pipe_shutdown"
    external is_readable: t -> int = "camluv_pipe_is_readable"
    external is_writable: t -> int = "camluv_pipe_is_writable"
    external write_queue_size: t -> int = "camluv_pipe_write_queue_size"
    external set_blocking: t -> int = "camluv_pipe_set_blocking"
    external is_closing: t -> int = "camluv_pipe_is_closing"
    external is_active: t -> int = "camluv_pipe_is_active"
    external ref: t -> unit = "camluv_pipe_ref"
    external unref: t -> unit = "camluv_pipe_unref"
    external has_ref: t -> int = "camluv_pipe_has_ref"
    external loop: t -> Loop.t = "camluv_pipe_loop"
  end

module TTY =
  struct
    type t

    type cb_read = t -> int -> uv_buffer array -> unit
    type cb_write = t -> int -> unit
    type cb_close = t-> unit
    type cb_shutdown = t -> int -> unit

    external init: Loop.t -> int -> int -> t = "camluv_tty_init"
    external create: Loop.t -> int -> int = "camluv_tty_init"
    external set_mode: t -> int -> uv_errno = "camluv_tty_set_mode"
    external reset_mode: unit -> uv_errno = "camluv_tty_reset_mode"
    external start_read: t -> cb_read -> uv_errno = "camluv_tty_start_read"
    external stop_read: t -> uv_errno = "camluv_tty_stop_read"
    external write: t -> uv_buffer array-> cb_write -> uv_errno = "camluv_tty_start_write"
    external window_size: t -> int * int = "camluv_tty_get_winsize"
    external close: t -> cb_close -> unit = "camluv_tty_close"
    external shutdown: t -> cb_shutdown -> uv_errno = "camluv_tty_shutdown"
    external is_readable: t -> int = "camluv_tty_is_readable"
    external is_writable: t -> int = "camluv_tty_is_writable"
    external write_queue_size: t -> int = "camluv_tty_write_queue_size"
    external set_blocking: t -> int = "camluv_tty_set_blocking"
    external is_closing: t -> int = "camluv_tty_is_closing"
    external is_active: t -> int = "camluv_tty_is_active"
    external ref: t -> unit = "camluv_tty_ref"
    external unref: t -> unit = "camluv_tty_unref"
    external has_ref: t -> int = "camluv_tty_has_ref"
    external loop: t -> Loop.t = "camluv_tty_loop"
  end

module Thread =
  struct

    type t

    external init: ('a -> unit) -> 'a -> t = "camluv_thread_init"
    external create: ('a -> unit) -> 'a -> t = "camluv_thread_init"
    external self: unit -> int = "camluv_thread_self"
    external join: t -> uv_errno = "camluv_thread_join"
  end

module Key =
  struct

    type t

    external init: unit -> t = "camluv_key_init"
    external create: unit -> t = "camluv_key_init"
    external delete: t -> unit = "camluv_key_delete"
    external get: t -> 'a = "camluv_key_get"
    external set: t -> 'a -> unit = "camluv_key_set"
  end

module Mutex =
  struct

    type t

    external init: unit -> t = "camluv_mutex_init"
    external create: unit -> t = "camluv_mutex_init"
    external destroy: t -> unit = "camluv_mutex_destroy"
    external lock: t -> unit = "camluv_mutex_lock"
    external unlock: t -> unit = "camluv_mutex_unlock"
    external trylock: t -> int = "camluv_mutex_trylock"
  end

module RWlock =
  struct

    type t

    external init: unit -> t = "camluv_rwlock_init"
    external create: unit -> t = "camluv_rwlock_init"
    external destroy: t -> unit = "camluv_rwlock_destroy"
    external rdlock: t -> unit = "camluv_rwlock_rdlock"
    external rdunlock: t -> unit = "camluv_rwlock_rdunlock"
    external tryrdlock: t -> int = "camluv_rwlock_tryrdlock"
    external wrlock: t -> unit = "camluv_rwlock_wrlock"
    external wrunlock: t -> unit = "camluv_rwlock_wrunlock"
    external trywrlock: t -> int = "camluv_rwlock_trywrlock"
  end

module Semaphore =
  struct

    type t

    external init: int -> t = "camluv_sem_init"
    external create: int -> t = "camluv_sem_init"
    external destroy: t -> unit = "camluv_sem_destroy"
    external post: t -> unit = "camluv_sem_post"
    external wait: t -> unit = "camluv_sem_wait"
    external trywait: t -> int = "camluv_sem_trywait"
  end

module Condition =
  struct

    type t

    external init: unit -> t = "camluv_condition_init"
    external create: unit -> t = "camluv_condition_init"
    external destroy: t -> unit = "camluv_condition_destroy"
    external signal: t -> unit = "camluv_condition_signal"
    external broadcast: t -> unit = "camluv_condition_broadcast"
    external wait: t -> Mutex.t -> unit = "camluv_condition_wait"
  end

module Barrier =
  struct

    type t

    external init: int -> t = "camluv_barrier_init"
    external create: int -> t = "camluv_barrier_init"
    external destroy: t -> unit = "camluv_barrier_destroy"
    external wait: t -> unit = "camluv_barrier_wait"
  end

module Process =
  struct

    type t

    type cb = t -> int64 -> int -> unit

    type flags = (** UNUSED **)
      | UV_PROCESS_SETUID
      | UV_PROCESS_SETGID
      | UV_PROCESS_WINDOWS_VERBATIM_ARGUMENTS
      | UV_PROCESS_DETACHED
      | UV_PROCESS_WINDOWS_HIDE

    type opts = {
      exit_cb: cb;
      file: string;
      args: string array;
      env: string array;
      cwd: string;
      flags: int;
      uid: int;
      gid: int;
    }

    external init: Loop.t -> opts -> t = "camluv_process_spawn"
    external create: Loop.t -> opts -> t = "camluv_process_spawn"
    external spawn: Loop.t -> opts -> t = "camluv_process_spawn"
    external getpid: t -> int = "camluv_process_getpid"
    external kill: t -> int -> uv_errno = "camluv_process_kill"
    external kill2: int -> int -> uv_errno = "camluv_kill"
  end


module Util =
  struct
    (* DOES NOT EXIST
     * external str_error: int -> string = "camluv_strerror" *)
    external err_name: int -> string = "camluv_err_name"
  end

