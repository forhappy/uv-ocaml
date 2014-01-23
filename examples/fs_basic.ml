open Camluv

let open_cb fs =
  let _ = Printf.printf "Open file : %s, %!" (Fs.get_path fs) in
    let _ = Printf.printf "result: %d\n%!" (Fs.get_result fs) in
      Fs.clean fs;;

let loop = Loop.default () in
  let _ = Fs.openfile loop "/tmp/xxx" 0(*O_RDONLY*) 0 open_cb in
    Loop.run loop UV_RUN_DEFAULT;;

