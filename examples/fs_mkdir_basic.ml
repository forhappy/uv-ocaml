open Camluv

let mkdir_cb fs =
  let _ = Printf.printf "Mkdir: %s, %!" (Fs.get_path fs) in
    let _ = Printf.printf "result: %d\n%!" (Fs.get_result fs) in
      Fs.clean fs;;

let loop = Loop.default () in
  let _ = Fs.mkdir loop "/tmp/xxx" 777 mkdir_cb in
    Loop.run loop UV_RUN_DEFAULT;;

