type loop

type handle

type idle

type timer

type uv_run_mode =
    UV_RUN_DEFAULT
  | UV_RUN_ONCE
  | UV_RUN_NOWAIT

type uv_walk_cb = handle -> string -> unit

type uv_close_cb = handle -> unit

type uv_idle_cb = idle -> int -> unit

type uv_timer_cb = timer -> int -> unit

module Loop =
  struct
    external create: unit->loop = "camluv_loop_new"
    external default: unit->loop = "camluv_loop_default"
    external delete: loop -> unit = "camluv_loop_delete"
    external run: loop -> uv_run_mode -> int32 = "camluv_loop_run"
    external stop: loop -> unit = "camluv_loop_stop"
    external now: loop -> int64 = "camluv_loop_now"
    external update_time: loop -> unit = "camluv_loop_update_time"
    external backend_fd: loop -> int32 = "camluv_loop_backend_fd"
    external backend_timeout: loop -> int32 = "camluv_loop_backend_timeout"
    external walk: loop -> uv_walk_cb -> string -> unit = "camluv_loop_walk"
  end

module Handle =
  struct
    external close: handle -> uv_close_cb -> unit = "camluv_close"
    external closed: handle -> int = "camluv_is_closing"
    external active: handle -> int = "camluv_is_active"
    external ref: handle -> unit = "camluv_ref"
    external unref: handle -> unit = "camluv_unref"
    external has_ref: handle -> int = "camluv_has_ref"
    external loop: handle -> loop = "camluv_loop"
  end

module Idle =
  struct
    external init: loop -> idle = "camluv_idle_init"
    external create: loop -> idle = "camluv_idle_init"
    external start: idle -> uv_idle_cb -> int = "camluv_idle_start"
    external stop: idle -> int = "camluv_idle_stop"
  end

module Timer =
  struct
    external init: loop -> timer = "camluv_timer_init"
    external create: loop -> timer = "camluv_timer_init"
    external start: timer -> uv_timer_cb -> int64 -> int64 -> int = "camluv_timer_start"
    external stop: timer -> int = "camluv_timer_stop"
    external again: timer -> int = "camluv_timer_again"
    external set_repeat: timer -> int64 -> unit = "camluv_timer_set_repeat"
    external get_repeat: timer -> int64 = "camluv_timer_get_repeat"
  end

