open Camluv

let fs_poll_cb handle status prev curr =
    Printf.printf "Something happened.%!\n";;


let loop = Loop.default ();;
let fs_poll = FsPoll.init loop;;

FsPoll.start fs_poll fs_poll_cb "/tmp/xxx" 1;;

let rc = Loop.run loop UV_RUN_DEFAULT;;

