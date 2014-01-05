type loop
type handle
type uv_run_mode = UV_RUN_DEFAULT | UV_RUN_ONCE | UV_RUN_NOWAIT
type uv_walk_cb = handle -> string -> unit
module Loop :
  sig
    external create : unit -> loop = "camluv_loop_new"
    external default : unit -> loop = "camluv_loop_default"
    external delete : loop -> unit = "camluv_loop_delete"
    external run : loop -> uv_run_mode -> int32 = "camluv_loop_run"
    external stop : loop -> unit = "camluv_loop_stop"
    external now : loop -> int64 = "camluv_loop_now"
    external update_time : loop -> unit = "camluv_loop_update_time"
    external backend_fd : loop -> int32 = "camluv_loop_backend_fd"
    external backend_timeout : loop -> int32 = "camluv_loop_backend_timeout"
    external walk : loop -> uv_walk_cb -> string -> unit = "camluv_loop_walk"
  end
