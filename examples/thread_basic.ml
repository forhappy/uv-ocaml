open Camluv

let thread_cb arg = print_string arg;;

let thread1 = Thread.create thread_cb "hello world\n";;
let thread2 = Thread.create thread_cb "hello world2\n";;
let thread3 = Thread.create thread_cb "hello world3\n";;

Thread.join thread1;;
Thread.join thread2;;
Thread.join thread3;;
