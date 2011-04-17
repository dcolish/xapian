open Swig;;
open Xapian;;

(* Helpers *)
let get_closure obj = 
    match obj with
        C_list [C_string a; C_obj f] -> C_obj f
      | _ -> raise (Failure "ZOMG")
;;

let parse_query qp query = 
  let c_query = C_string query in
    get_closure (qp -> parse_query(c_query))
;;

let writable_db path = 
  let c_path = C_string path in 
    get_closure (new_WritableDatabase '(c_path, 1))
;;

let add_terms doc terms =
  List.map (fun el -> 
              let c_el = C_string el in
                doc -> add_term(c_el)) terms
;;

(* Quick test to ensure the module is loaded *)
let _ = 
  assert (compare Xapian.module_name "xapian" == 0);
  assert (compare (_version_string '() as string) "1.2.5" == 0)
;;

(* Basis document test *)

let _ = 
  let doc = new_Document '() in
    add_terms doc ["hello"];
    assert ((doc -> termlist_count() as int) == 1);
    doc -> set_data ("something");
    assert (compare (doc -> get_data() as string) "something" == 0);
    let tb = doc -> termlist_begin() in
    let te = doc -> termlist_end() in
      assert (tb -> equals(te) as bool == false);
      assert (compare (tb -> get_term() as string)  "hello" == 0)
;;

(* simple database test *)
let _ = 
  let db = writable_db "test.db" in
  let doc = new_Document '() in
    add_terms doc ["hello"];
    doc -> set_data("goodbye");
    db -> add_document(doc);
    db -> flush();
    let qp = new_QueryParser '() in 
      qp -> set_database(db);
      let pq = parse_query qp "hello" in
      let tb = pq -> get_terms_begin() in
        assert (pq -> empty() as bool == false);
        assert (compare (tb -> get_term() as string) "hello" == 0)
;;


print_endline "All tests passed."
