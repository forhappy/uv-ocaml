open Camluv

let rmdir_cb fs =
  let _ = Printf.printf "Rmdir: %s, %!" (Fs.get_path fs) in
    let _ = Printf.printf "result: %d\n%!" (Fs.get_result fs) in
      Fs.clean fs;;

let loop = Loop.default () in
  let _ = Fs.rmdir loop "/tmp/xxx" rmdir_cb in
    Loop.run loop UV_RUN_DEFAULT;;

