open Camluv

let fs_event_cb handle filename event status =
    print_string "Something happened.\n";;


let loop = Loop.default ();;
let fs_event = FsEvent.init loop;;

FsEvent.start fs_event fs_event_cb "/tmp/xxx" 0;;

let rc = Loop.run loop UV_RUN_DEFAULT;;

