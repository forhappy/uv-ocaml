open Camluv

let close_cb fs =
  Fs.clean fs;
  Printf.printf "Close file OK\n%!";;

let write_cb fs =
  Fs.clean fs;
  let _ = Printf.printf "Write result: %d\n%!" (Fs.get_result fs) in
    Fs.close (Loop.default ()) (Fs.get_result fs) close_cb;;

let open_cb fs =
  let _ = Printf.printf "Open file : %s, %!" (Fs.get_path fs) in
    let _ = Printf.printf "result: %d\n%!" (Fs.get_result fs) in
      let _ = Fs.write (Loop.default ()) (Fs.get_result fs) ("hello world") 11 0 write_cb in
        Fs.clean fs;;

let _ = Fs.openfile (Loop.default ()) "/tmp/xxx" 1(*O_WRONLY*) 0 open_cb in
  Loop.run (Loop.default ()) UV_RUN_DEFAULT;;

