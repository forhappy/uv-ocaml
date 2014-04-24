open Uv

let () =
  let thread_cb arg = print_string arg in
  let thread1 = Thread.create thread_cb "hello world\n" in
  let thread2 = Thread.create thread_cb "hello world2\n" in
  let thread3 = Thread.create thread_cb "hello world3\n" in
  Thread.join thread1;
  Thread.join thread2;
  Thread.join thread3;
  ()
