open Uv

let close_cb fs =
  Fs.clean fs;
  Printf.printf "Close file done\n%!"

let read_cb fs =
  Fs.clean fs;
  let _ = Printf.printf "Read result: %d\n%!" (Fs.get_result fs) in
  Fs.close (Loop.default ()) (Fs.get_result fs) close_cb

let open_cb fs =
  let _ = Printf.printf "Open file : %s, %!" (Fs.get_path fs) in
  let _ = Printf.printf "result: %d\n%!" (Fs.get_result fs) in
  let readbuf = Fs.read (Loop.default ()) (Fs.get_result fs) 1024 0 read_cb in
  Printf.printf "%s%!\n" readbuf;
  Fs.clean fs

let () =
  let _ = Fs.openfile (Loop.default ()) "/tmp/xxx" 0(*O_RDONLY*) 0 open_cb in
  Loop.run (Loop.default ()) Loop.UV_RUN_DEFAULT

