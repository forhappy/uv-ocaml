open Uv
open Int64

let () =
  let counter = ref 0 in
  let incr_counter_cb timer status =
    if !counter < 10
      then counter := !counter + 1
      else
        let _ = Timer.stop timer in
        Printf.printf "Timer stoped.\n%!"
  in
  Printf.printf "The original counter is: %d\n%!" !counter;
  let loop = Loop.default () in
  let timer = Timer.init loop in
  let _ = Timer.start timer incr_counter_cb (of_int 0) (of_int 100) in
  Loop.run loop Loop.UV_RUN_DEFAULT;
  Printf.printf "The last counter is: %d\n%!" !counter
