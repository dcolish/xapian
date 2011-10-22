open Swig
open Xapian
open Unix

(* Quick test to ensure the module is loaded *)
let () = 
  assert (Xapian.module_name =  "xapian");
  assert ((Xapian.version_string '() as string) = "1.3.0")
;;

(* Basis document test *)

let () = 
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
let () = 
  let db = writable_db "test.db" 1 in
  let doc = new_Document '() in
    ignore (add_terms doc ["hello"]);
    ignore (doc -> set_data("goodbye"));
    ignore (db -> add_document(doc));
    ignore (db -> flush());
    let enq = new_Enquire '(db) in
    let qp = new_QueryParser '() in 
      ignore (qp -> set_database(db));
      let pq = build_parsed_query qp "hello" in
        enq -> set_query(pq);
        let matches = enq -> get_mset(0, 10) in
          Printf.printf "Found %d\n" (matches -> size() as int);
          assert ((matches -> size() as int) = 1);
          let tb = pq -> get_terms_begin() in
            assert (pq -> empty() as bool = false);
            assert ((tb -> get_term() as string) = "hello")
;;


(* add a number of documents to a db then pull put one into an Rset and use
get_eset *)

let _ = 
  ignore (
    Array.map (fun el ->
                 ignore (Sys.remove (Filename.concat "test.db" el)))
      (Sys.readdir "test.db"));
  Unix.rmdir "test.db"
;;

print_endline "All tests passed."
