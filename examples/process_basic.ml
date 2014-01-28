open Camluv

let process_exit_cb process exit_status term_signal =
  Printf.printf "Process %d exited.\n%!" (Process.getpid process);;

let options = {
  exit_cb = process_exit_cb;
  file = "mkdir";
  args = [|"mkdir"; "-p"; "/tmp/camluv/xxx/"|];
  env = [|"CAMLUV_DIR="|];
  cwd = ".";
  flags = 0;
  uid = 0;
  gid = 0;
};;

let _ = Process.spawn (Loop.default ()) options in
  Loop.run (Loop.default ()) UV_RUN_DEFAULT;;

