open Swig
open Xapian
open Unix

(* Quick test to ensure the module is loaded *)
let _ = 
  assert (Xapian.module_name =  "xapian");
  assert ((Xapian.version_string '() as string) = "1.2.5")
;;

(* Basis document test *)

let _ = 
  let doc = new_Document '() in
    ignore (add_terms doc ["hello"]);
    assert ((doc -> termlist_count() as int) = 1);
    ignore (doc -> set_data ("something"));
    assert ((doc -> get_data() as string) = "something");
    let tb = doc -> termlist_begin() in
    let te = doc -> termlist_end() in
      assert (tb -> equals(te) as bool == false);
      assert ((tb -> get_term() as string) = "hello")
;;

(* simple database test *)
let _ = 
  let db = writable_db "test.db" 1 in
  let doc = new_Document '() in
    ignore (add_terms doc ["hello"]);
    ignore (doc -> set_data("goodbye"));
    ignore (db -> add_document(doc));
    ignore (db -> flush());
    let qp = new_QueryParser '() in 
      ignore (qp -> set_database(db));
      let pq = build_parsed_query qp "hello" in
      let tb = pq -> get_terms_begin() in
        assert (pq -> empty() as bool = false);
        assert ((tb -> get_term() as string) = "hello")
;;


let _ = 
  ignore (
    Array.map (fun el ->
                 ignore (Sys.remove (Filename.concat "test.db" el)))
      (Sys.readdir "test.db"));
  Unix.rmdir "test.db"
;;

print_endline "All tests passed."
