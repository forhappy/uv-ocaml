open Camluv

let rename_cb fs =
  let _ = Printf.printf "Rename file : %s, %!" (Fs.get_path fs) in
    let _ = Printf.printf "result: %d\n%!" (Fs.get_result fs) in
      Fs.clean fs;;

let loop = Loop.default () in
  let _ = Fs.rename loop "/tmp/xxx" "/tmp/yyy" rename_cb in
    Loop.run loop UV_RUN_DEFAULT;;

