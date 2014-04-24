open Uv
open Int64

let () =
  let lp = Loop.default () in
  print_string "hello world\n";
  print_string (to_string (Loop.now lp));
  let rc = Loop.run lp Loop.UV_RUN_DEFAULT in
  ()
