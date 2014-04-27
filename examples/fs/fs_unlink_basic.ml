open Uv

let () =
  let unlink_cb fs =
    let _ = Printf.printf "Unlink: %s, %!" (Fs.get_path fs) in
    let _ = Printf.printf "result: %d\n%!" (Fs.get_result fs) in
    Fs.clean fs
  in
  let loop = Loop.default () in
  let _ = Fs.unlink loop "/tmp/xxx" unlink_cb in
  Loop.run loop Loop.UV_RUN_DEFAULT

