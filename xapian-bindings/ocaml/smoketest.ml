open Swig
open Xapian

let _ = 
  begin
    assert (compare Xapian.module_name "xapian" == 0);
    assert (compare (_version_string '() as string) "1.2.5" == 0);
  end
;;

print_endline "All tests passed."
