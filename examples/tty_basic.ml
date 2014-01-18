open Camluv

let tty_write_cb handle status =
    Printf.printf "Status %d.%!\n" (status);;

let buf1 = {base = "\033[41;37m"; len = 8}
let buf2 = {base = "Hello TTY\n"; len = 10}
let bufs1 = [|buf1|]
let bufs2 = [|buf2|]

let loop = Loop.default ();;

let tty = TTY.init loop 1 0;;
let _ = TTY.set_mode tty 0;;
let _ = TTY.write tty bufs1 tty_write_cb;;
let _ = TTY.write tty bufs2 tty_write_cb;;
let _ = TTY.reset_mode ();;
let _ = Loop.run loop UV_RUN_DEFAULT;;

