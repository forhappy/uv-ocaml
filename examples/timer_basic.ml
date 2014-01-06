open Camluv
open Int64

let counter = ref 0;;

let incr_counter_cb timer status =
  if !counter < 100 then
    counter := !counter + 1
  else print_int (Timer.stop timer);;

print_endline "The original counter is: ";;
print_int !counter;;
print_endline "\n";;

let loop = Loop.default ();;
let timer = Timer.init loop;;

Timer.start timer incr_counter_cb (of_int 0) (of_int 100);;

let rc = Loop.run loop UV_RUN_DEFAULT;;
print_endline "\n";;

print_endline "The last counter is: ";;
print_int !counter;;
print_endline "\n";;
