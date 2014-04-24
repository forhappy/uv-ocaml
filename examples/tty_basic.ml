open Uv

let () =
  let tty_write_cb handle status = Printf.printf "Status %d.%!\n" status in
  let bufs1 = [| {base = "\033[41;37m"; len = 8} |] in
  let bufs2 = [| {base = "Hello TTY\n"; len = 10} |] in
  let loop = Loop.default () in
  let tty = TTY.init loop 1 0 in
  let _ = TTY.set_mode tty 0 in
  let _ = TTY.write tty bufs1 tty_write_cb in
  let _ = TTY.write tty bufs2 tty_write_cb in
  let _ = TTY.reset_mode () in
  let _ = Loop.run loop Loop.UV_RUN_DEFAULT in
  ()

