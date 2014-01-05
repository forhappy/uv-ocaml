open Camluv

let counter = ref 0;;

let incr_counter_cb idle status =
  if !counter < 10000 then
    counter := !counter + 1
  else print_int (Idle.stop idle);;

print_endline "The original counter is: ";;
print_int !counter;;
print_endline "\n";;

let loop = Loop.default ();;
let idle = Idle.init loop;;

Idle.start idle incr_counter_cb;;

let rc = Loop.run loop UV_RUN_DEFAULT;;
print_endline "\n";;

print_endline "The last counter is: ";;
print_int !counter;;
print_endline "\n";;
