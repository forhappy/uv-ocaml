open Camluv
open Int64

let lp = Loop.default ();;

print_string "hello world\n";;

print_string (to_string (Loop.now lp));;

let rc = Loop.run lp UV_RUN_DEFAULT;;
