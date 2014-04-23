type uv_buffer = { base : string; len : int; }

type uv_sockaddr = {
  host : string;
  port : int;
  scope_id : int;
  flowinfo : int;
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

type uv_stdio_flags =
    UV_IGNORE
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
    val create : unit -> t
    val default : unit -> t
    val delete : t -> unit
    val run : t -> run_mode -> uv_errno
    val stop : t -> unit
    val now : t -> int64
    val update_time : t -> unit
    val backend_fd : t -> int32
    val backend_timeout : t -> int32
    val walk : t -> Handle.cb_walk -> string -> unit
    val queue_work : t -> cb_work -> cb_after_work -> uv_errno
  end
and Handle :
  sig
    type t
    type t_type =
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
    type cb_walk = t -> string -> unit
    type cb_close = t -> unit
    val close : t -> cb_close -> unit
    val closed : t -> int
    val active : t -> int
    val ref : t -> unit
    val unref : t -> unit
    val has_ref : t -> int
    val loop : t -> Loop.t
  end
module Idle :
  sig
    type t
    type cb = t -> int -> unit
    val init : Loop.t -> t 
    val create : Loop.t -> t 
    val start : t -> cb -> uv_errno 
    val stop : t -> uv_errno 
  end
module Timer :
  sig
    type t
    type cb = t -> int -> unit
    val init : Loop.t -> t 
    val create : Loop.t -> t 
    val start : t -> cb -> int64 -> int64 -> uv_errno
    val stop : t -> uv_errno 
    val again : t -> uv_errno 
    val set_repeat : t -> int64 -> unit 
    val get_repeat : t -> int64 
  end
module Async :
  sig
    type t
    type cb = t -> int -> unit
    val init : Loop.t -> t 
    val create : Loop.t -> t 
    val start : t -> cb -> uv_errno 
    val stop : t -> uv_errno 
  end
module Check :
  sig
    type t
    type cb = t -> int -> unit
    val init : Loop.t -> t 
    val create : Loop.t -> t 
    val start : t -> cb -> uv_errno 
    val stop : t -> uv_errno 
  end
module Prepare :
  sig
    type t
    type cb = t -> int -> unit
    val init : Loop.t -> t 
    val create : Loop.t -> t 
    val start : t -> cb -> uv_errno 
    val stop : t -> uv_errno 
  end
module Poll :
  sig
    type t
    type event = UV_READABLE | UV_WRITABLE | UV_RDWRABLE
    type cb = t -> int -> event -> unit
    val init : Loop.t -> int -> t 
    val create : Loop.t -> int -> t 
    val start : t -> event -> cb -> uv_errno 
    val fileno : t -> int 
    val stop : t -> uv_errno 
  end
module Fs :
  sig
    type t
    type cb = t -> unit
    type timestamp = { tv_sec : int32; tv_nsec : int32; }
    type stat = {
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
      st_atim : timestamp;
      st_mtim : timestamp;
      st_ctim : timestamp;
      st_birthtim : timestamp;
    }
    val close : Loop.t -> int -> cb -> unit 
    val openfile : Loop.t -> string -> int -> int -> cb -> uv_errno 
    val read : Loop.t -> int -> int -> int -> cb -> string 
    val unlink : Loop.t -> string -> cb -> uv_errno 
    val write : Loop.t -> int -> string -> int -> int -> cb -> uv_errno 
    val mkdir : Loop.t -> string -> int -> cb -> uv_errno 
    val rmdir : Loop.t -> string -> cb -> uv_errno 
    val readdir : Loop.t -> string -> int -> cb -> uv_errno 
    val stat : Loop.t -> string -> cb -> uv_errno 
    val fstat : Loop.t -> int -> cb -> uv_errno 
    val rename : Loop.t -> string -> string -> cb -> uv_errno 
    val fsync : Loop.t -> int -> cb -> uv_errno 
    val fdatasync : Loop.t -> int -> cb -> uv_errno 
    val ftruncate : Loop.t -> int -> int -> cb -> uv_errno 
    val sendfile : Loop.t -> int -> int -> int -> int -> cb -> uv_errno 
    val chmod : Loop.t -> string -> int -> cb -> uv_errno 
    val utime : Loop.t -> string -> float -> float -> cb -> uv_errno 
    val futime : Loop.t -> int -> float -> float -> cb -> uv_errno 
    val lstat : Loop.t -> string -> cb -> uv_errno 
    val link : Loop.t -> string -> string -> cb -> uv_errno 
    val symlink : Loop.t -> string -> string -> int -> cb -> uv_errno 
    val readlink : Loop.t -> string -> cb -> uv_errno 
    val fchmod : Loop.t -> int -> int -> cb -> uv_errno 
    val chown : Loop.t -> string -> int -> int -> cb -> uv_errno 
    val fchown : Loop.t -> int -> int -> int -> cb -> uv_errno 
    val get_result : t -> int 
    val get_path : t -> string 
    val get_stat : t -> stat 
    val get_loop : t -> Loop.t 
    val clean : t -> unit 
  end
module Signal :
  sig
    type t
    type cb = t -> int -> unit
    val init : Loop.t -> t 
    val create : Loop.t -> t 
    val start : t -> cb -> int -> uv_errno 
    val stop : t -> uv_errno 
  end
module FsEvent :
  sig
    type t
    type cb = t -> string -> int -> int -> unit
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
    type uv_fs_event_flags =
        UV_FS_EVENT_WATCH_ENTRY
      | UV_FS_EVENT_STAT
      | UV_FS_EVENT_RECURSIVE
    val init : Loop.t -> t 
    val create : Loop.t -> t 
    val start : t -> cb -> string -> int -> uv_errno 
    val stop : t -> uv_errno 
  end
module FsPoll :
  sig
    type t
    type cb = t -> int -> Fs.stat -> Fs.stat -> unit
    val init : Loop.t -> t 
    val create : Loop.t -> t 
    val start : t -> cb -> string -> int -> uv_errno 
    val stop : t -> uv_errno 
  end
module TCP :
  sig
    type t
    type cb_connect = t -> int -> unit
    type cb_connection = t -> int -> unit
    type cb_close = t -> unit
    type cb_shutdown = t -> int -> unit
    type cb_read = t -> int -> uv_buffer array -> unit
    type cb_write = t -> int -> unit
    val init : Loop.t -> t 
    val create : Loop.t -> t 
    val openfd : t -> int -> uv_errno 
    val bind : t -> uv_sockaddr -> uv_errno 
    val listen : t -> int -> cb_connection -> uv_errno 
    val accept : t -> t 
    val connect : t -> uv_sockaddr -> cb_connect -> uv_errno 
    val getsockname : t -> uv_sockaddr 
    val getpeername : t -> uv_sockaddr 
    val nodelay : t -> int -> uv_errno 
    val keepalive : t -> int -> int -> uv_errno 
    val simultaneous_accepts : Loop.t -> int -> uv_errno 
    val start_read : t -> cb_read -> uv_errno 
    val stop_read : t -> uv_errno 
    val write : t -> uv_buffer array -> cb_write -> uv_errno 
    val close : t -> cb_close -> unit 
    val shutdown : t -> cb_shutdown -> uv_errno 
    val is_readable : t -> int 
    val is_writable : t -> int 
    val write_queue_size : t -> int 
    val set_blocking : t -> int 
    val is_closing : t -> int 
    val is_active : t -> int 
    val ref : t -> unit 
    val unref : t -> unit 
    val has_ref : t -> int 
    val loop : t -> Loop.t 
  end
module UDP :
  sig
    type t
    type flags = UV_UDP_IPV6ONLY | UV_UDP_PARTIAL
    type membership = UV_LEAVE_GROUP | UV_JOIN_GROUP
    type cb_close = t -> unit
    type cb_send = t -> int -> unit
    type cb_recv =
        t -> int -> uv_buffer array -> uv_sockaddr -> flags -> unit
    val init : Loop.t -> t 
    val create : Loop.t -> t 
    val openfd : t -> int -> uv_errno 
    val bind : t -> uv_sockaddr -> uv_errno 
    val getsockname : t -> uv_sockaddr 
    val set_multicast_loop : t -> int -> uv_errno 
    val set_multicast_ttl : t -> int -> uv_errno 
    val set_broadcast : t -> int -> uv_errno 
    val set_ttl : t -> int -> uv_errno 
    val set_membership : t -> string -> string -> membership -> uv_errno 
    val start_recv : t -> cb_recv -> uv_errno 
    val stop_recv : t -> uv_errno 
    val send : t -> uv_sockaddr -> uv_buffer array -> cb_send -> uv_errno 
    val close : t -> cb_close -> unit 
    val is_closing : t -> int 
    val is_active : t -> int 
    val ref : t -> unit 
    val unref : t -> unit 
    val has_ref : t -> int 
    val loop : t -> Loop.t 
  end
module Pipe :
  sig
    type t
    type cb_connect = t -> int -> unit
    type cb_connection = t -> int -> unit
    type cb_close = t -> unit
    type cb_shutdown = t -> int -> unit
    type cb_read = t -> int -> uv_buffer array -> unit
    type cb_read2 = t -> int -> uv_buffer array -> Handle.t -> unit
    type cb_write = t -> int -> unit
    val init : Loop.t -> t 
    val create : Loop.t -> t 
    val openfd : t -> int -> uv_errno 
    val bind : t -> string -> uv_errno 
    val listen : t -> int -> cb_connection -> uv_errno 
    val accept : t -> t 
    val connect : t -> string -> cb_connect -> uv_errno 
    val getsockname : t -> uv_sockaddr 
    val getpeername : t -> uv_sockaddr 
    val nodelay : t -> int -> uv_errno 
    val keepalive : t -> int -> int -> uv_errno 
    val simultaneous_accepts : Loop.t -> int -> uv_errno 
    val start_read : t -> cb_read -> uv_errno 
    val start_read2 : t -> cb_read2 -> uv_errno 
    val stop_read : t -> uv_errno 
    val write : t -> uv_buffer array -> cb_write -> uv_errno 
    val write_tcp : t -> uv_buffer array -> TCP.t -> cb_write -> uv_errno 
    val write_pipe : t -> uv_buffer array -> TCP.t -> cb_write -> uv_errno 
    val close : t -> cb_close -> unit 
    val shutdown : t -> cb_shutdown -> uv_errno 
    val is_readable : t -> int 
    val is_writable : t -> int 
    val write_queue_size : t -> int 
    val set_blocking : t -> int 
    val is_closing : t -> int 
    val is_active : t -> int 
    val ref : t -> unit 
    val unref : t -> unit 
    val has_ref : t -> int 
    val loop : t -> Loop.t 
  end
module TTY :
  sig
    type t
    type cb_read = t -> int -> uv_buffer array -> unit
    type cb_write = t -> int -> unit
    type cb_close = t -> unit
    type cb_shutdown = t -> int -> unit
    val init : Loop.t -> int -> int -> t 
    val create : Loop.t -> int -> int 
    val set_mode : t -> int -> uv_errno 
    val reset_mode : unit -> uv_errno 
    val start_read : t -> cb_read -> uv_errno 
    val stop_read : t -> uv_errno 
    val write : t -> uv_buffer array -> cb_write -> uv_errno 
    val window_size : t -> int * int 
    val close : t -> cb_close -> unit 
    val shutdown : t -> cb_shutdown -> uv_errno 
    val is_readable : t -> int 
    val is_writable : t -> int 
    val write_queue_size : t -> int 
    val set_blocking : t -> int 
    val is_closing : t -> int 
    val is_active : t -> int 
    val ref : t -> unit 
    val unref : t -> unit 
    val has_ref : t -> int 
    val loop : t -> Loop.t 
  end
module Thread :
  sig
    type t
    val init : ('a -> unit) -> 'a -> t 
    val create : ('a -> unit) -> 'a -> t 
    val self : unit -> int 
    val join : t -> uv_errno 
  end
module Key :
  sig
    type t
    val init : unit -> t 
    val create : unit -> t 
    val delete : t -> unit 
    val get : t -> 'a 
    val set : t -> 'a -> unit 
  end
module Mutex :
  sig
    type t
    val init : unit -> t 
    val create : unit -> t 
    val destroy : t -> unit 
    val lock : t -> unit 
    val unlock : t -> unit 
    val trylock : t -> int 
  end
module RWlock :
  sig
    type t
    val init : unit -> t 
    val create : unit -> t 
    val destroy : t -> unit 
    val rdlock : t -> unit 
    val rdunlock : t -> unit 
    val tryrdlock : t -> int 
    val wrlock : t -> unit 
    val wrunlock : t -> unit 
    val trywrlock : t -> int 
  end
module Semaphore :
  sig
    type t
    val init : int -> t 
    val create : int -> t 
    val destroy : t -> unit 
    val post : t -> unit 
    val wait : t -> unit 
    val trywait : t -> int 
  end
module Condition :
  sig
    type t
    val init : unit -> t 
    val create : unit -> t 
    val destroy : t -> unit 
    val signal : t -> unit 
    val broadcast : t -> unit 
    val wait : t -> Mutex.t -> unit 
  end
module Barrier :
  sig
    type t
    val init : int -> t 
    val create : int -> t 
    val destroy : t -> unit 
    val wait : t -> unit 
  end
module Process :
  sig
    type t
    type cb = t -> int64 -> int -> unit
    type flags =
        UV_PROCESS_SETUID
      | UV_PROCESS_SETGID
      | UV_PROCESS_WINDOWS_VERBATIM_ARGUMENTS
      | UV_PROCESS_DETACHED
      | UV_PROCESS_WINDOWS_HIDE
    type opts = {
      exit_cb : cb;
      file : string;
      args : string array;
      env : string array;
      cwd : string;
      flags : int;
      uid : int;
      gid : int;
    }
    val init : Loop.t -> opts -> t 
    val create : Loop.t -> opts -> t 
    val spawn : Loop.t -> opts -> t 
    val getpid : t -> int 
    val kill : t -> int -> uv_errno 
    val kill2 : int -> int -> uv_errno 
  end
module Util :
  sig
    val strerror : int -> string 
    val err_name : int -> string 
  end
