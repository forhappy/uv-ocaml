open Uv

let fs_poll_cb handle status prev curr =
  Printf.printf "Something happened.%!\n"

let () =
  let loop = Loop.default () in
  let fs_poll = FsPoll.init loop in
  let _ = FsPoll.start fs_poll fs_poll_cb "/tmp/xxx" 1 in
  Loop.run loop Loop.UV_RUN_DEFAULT

