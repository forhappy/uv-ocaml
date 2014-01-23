open Camluv

let write_cb fs =
  let _ = Printf.printf "Write file: %s OK\n%!" (Fs.get_path fs) in
    Fs.clean fs;;

let open_cb fs =
  let _ = Printf.printf "Open file : %s, %!" (Fs.get_path fs) in
    let _ = Printf.printf "result: %d\n%!" (Fs.get_result fs) in
      Fs.clean fs;;

let loop = Loop.default ();;
let _ = Fs.openfile loop "/tmp/xxx" 0(*O_RDONLY*) 0 open_cb;;
let _ = Fs.write loop 9 ("hello world") (11) (0) (write_cb);;
let _ = Loop.run loop UV_RUN_DEFAULT;;

