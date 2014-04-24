open Uv

let () =
  let counter = ref 0 in
  let incr_counter_cb idle status =
    if !counter < 10000
      then counter := !counter + 1
      else print_int (Idle.stop idle)
  in
  print_endline "The original counter is: ";
  print_int !counter;
  print_endline "\n";
  let loop = Loop.default () in
  let idle = Idle.init loop in
  Idle.start idle incr_counter_cb;
  let rc = Loop.run loop Loop.UV_RUN_DEFAULT in
  print_endline "\n";
  print_endline "The last counter is: ";
  print_int !counter;
  print_endline "\n"
