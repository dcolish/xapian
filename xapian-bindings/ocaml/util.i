%insert (mlitail) %{

val version_string : c_obj -> c_obj
val major_version : c_obj -> c_obj
val minor_version : c_obj -> c_obj
val revision : c_obj -> c_obj

val closure_from_lst : c_obj -> c_obj
val build_parsed_query : c_obj -> string -> c_obj
val writable_db : string -> int -> c_obj
val add_terms : c_obj -> string list -> c_obj list

%}

%insert (mltail) %{

let version_string = _version_string
let major_version = _major_version
let minor_version = _minor_version
let revision = _revision

(* Helpers *)

let closure_from_lst lst = 
    match lst with
        Swig.C_list [Swig.C_string a; Swig.C_obj f] -> Swig.C_obj f
      | _ -> raise (Failure "ZOMG")


let build_parsed_query qp query = 
  let c_query = Swig.C_string query in
    closure_from_lst ((invoke qp) "parse_query" (c_query))


let writable_db path flags = 
  let c_path = Swig.C_string path in 
    let c_flags = Swig.C_int flags in
    closure_from_lst (new_WritableDatabase (Swig.C_list[c_path; c_flags]))


let add_terms doc terms =
  List.map (fun el -> 
              let c_el = Swig.C_string el in
                (invoke doc) "add_term" (c_el)) terms

%}
