open Camluv

let fs_event_cb handle filename event status =
    Printf.printf "Event %d happened on file: %s with status %d.%!\n" (event) (filename) (status);;

let loop = Loop.default () in
  let fs_event = FsEvent.init loop in
    let _ = FsEvent.start fs_event fs_event_cb "/tmp/xxx" 0 in
      Loop.run loop UV_RUN_DEFAULT;;

