/*
 * cfe/feerrs.h - prototype for front-end error messages file
 * Copyright (C) Acorn Computers Ltd., 1988-1990.
 * Copyright (C) Codemist Ltd., 1987-1992.
 * Copyright (C) Advanced RISC Machines Limited, 1991-1992.
 * SPDX-Licence-Identifier: Apache-2.0
 * version 3b.
 */

/*
 * RCS $Revision$
 * Checkin $Date$
 * Revising $Author$
 */

#ifndef NLS                     /* if NLS, miperrs will have included tags */

%O  /* Ordinary error messages - mapped onto numeric codes */

/* Preprocessor messages are in miperrs.h */

#define lex_warn_force_unsigned "'%s' treated as '%sul' in 32-bit implementation"
#define lex_warn_force_longlong "'%s' treated as '%sll'"
#define lex_warn_force_ulonglong "'%s' treated as '%sull'"
#define lex_warn_multi_char "non-portable - not 1 char in '...'"
#define lex_warn_cplusplusid "C++ keyword used as identifier: $r"
#define lex_warn_cplusplusimpl \
        "Functionality of C++ keyword may not yet be fully implemented: $r"

#define syn_warn_hashif_undef "Undefined macro '%s' in #if - treated as 0"
#define syn_warn_invent_extern "inventing 'extern int %s();'"
#define syn_warn_unary_plus "Unary '+' is a feature of ANSI C"
#define syn_warn_spurious_braces "spurious {} around scalar initialiser"
#define syn_warn_dangling_else "Dangling 'else' indicates possible error"
#define syn_warn_void_return "non-value return in non-void function"
#define syn_warn_use_of_short \
        "'short' slower than 'int' on this machine (see manual)"
#define syn_warn_undeclared_parm \
        "formal parameter $r not declared - 'int' assumed"
#define syn_warn_old_style "Old-style function $r"
#define syn_warn_give_args "Deprecated declaration %s() - give arg types"
#define syn_warn_ANSI_decl "ANSI style function declaration used, '%s(...)'"
#define syn_warn_archaic_init "Ancient form of initialisation, use '='"
#define syn_warn_untyped_fn "implicit 'int' return type for $r - 'void' intended?"
#define syn_warn_no_named_member "$c has no named member"
#define syn_warn_extra_comma "Superfluous ',' in 'enum' declaration"
#define syn_warn_struct_padded "padding inserted in struct $b"
#define syn_warn_switch_funny "'switch (e)' not followed by '{'"
#define syn_warn_modify_access "access declaration with no effect"
#define syn_warn_storageclass_no_declarator \
        "storage-class without declarator is spurious"
#define syn_warn_lacks_storage_type \
        "declaration lacks type/storage-class (assuming 'int'): $r"
#define syn_warn_archaic_fnpara "archaic C-style function parameter $l"
#define syn_warn_special_ops "'=', ',' or unary '&' defined as non-member"
#define syn_warn_ineffective_asm_decl "asm(...) ignored: inline assembler not available"
#define syn_warn_insert_sym_anachronism "inserting $r in ':(...)' anachronism"
#define syn_warn_superfluous_prefix \
     "superfluous 'union','class', 'struct' or 'enum' prefix"
#define syn_warn_no_default_structor "$b: $c has no default %s"


#define xsyn_info_instantiate_class \
        "while instantiating $c"
#define xsyn_info_instantiate_fn \
        "while instantiating function $p$r"
#define xsyn_warn_ARM_cctor_suppress \
        "$b suppresses default copy constructor under ARM semantics"
#define xsyn_warn_ISO_cctor_no_suppress \
        "$b does not suppress default copy constructor under ISO semantics"
#define xsyn_warn_ARM_opeq_suppress \
        "$b suppresses default operator= under ARM semantics"
#define xsyn_warn_ISO_opeq_no_suppress \
        "$b does not suppress default operator= under ISO semantics"
#define xsyn_warn_delete_undef \
        "delete of pointer to undefined $c; no destructor will be called"
#define xsyn_warn_implicit_private_base "base class $c is implicitly private"

#define simp_warn_checkvar "syserr soon: checkvar use %ld"

#define vargen_warn_nonull "omitting trailing '\\0' for %s [%ld]"
#define vargen_warn_unnamed_bitfield \
        "Unnamed bit field initialised to 0"
#define vargen_warn_init_non_aggregate \
        "Attempt to initialise non-aggregate"

#define lex_err_ioverflow "Number %s too large for 32-bit implementation"
#define lex_err_ioverflow_64 "Number %s too large for 64-bit implementation"
#define lex_err_overlong_fp "Grossly over-long floating point number"
#define lex_err_fp_syntax1 "Digit required after exponent marker"
#define lex_err_overlong_hex "Grossly over-long hexadecimal constant"
#define lex_err_overlong_int "Grossly over-long number"
#define lex_err_need_hex_dig "Hex digit needed after 0x or 0X"
#define lex_err_need_hex_dig1 "Missing hex digit(s) after \\x"
#define lex_err_backslash_blank \
        "\\<space> and \\<tab> are invalid string escapes"
#define lex_err_unterminated_string "Newline or end of file within string"
#define lex_err_bad_hash "misplaced preprocessor character '%c'"
#define lex_err_bad_char "illegal character (0x%lx = \'%c\') in source"
#define lex_err_bad_noprint_char "illegal character (hex code 0x%x) in source"
#define lex_err_ellipsis "(...) must have exactly 3 dots"
#define lex_err_illegal_whitespace "$s may not have whitespace in it"

#define syn_err_illdtor "illegal destructor '~$#l'"
#define syn_err_bitsize "bit size %ld illegal - 1 assumed"
#define syn_err_zerobitsize "zero width named bit field - 1 assumed"
#define syn_err_arraysize "Array size %ld illegal - 1 assumed"
#define syn_err_expected "expected $s - inserted before $l"
#define syn_err_expected1 "expected $s - inserted before $l"
#define syn_err_expected1_aftercommand "expected $s after command - inserted before $l"
#define syn_err_expected2 "expected $s or $s - inserted $s before $l"
#define syn_err_expecteda "expected $s"
#define syn_err_expected1a "expected $s"
#define syn_err_expected1a_aftercommand "expected $s after command"
#define syn_err_expected2a "expected $s or $s"
#define syn_err_mix_strings "char and wide (L\"...\") strings do not concatenate"
#define syn_err_expected_expr "<expression> expected but found $l"
#define syn_err_expected_string_literal "expected string literal but found $l"
#ifdef EXTENSION_VALOF
#define syn_err_valof_block \
        "{ following a cast will be treated as VALOF block"
#endif
#define syn_err_typedef "typedef name $r used in expression context"
#define syn_err_assertion "___assert(0, $e)"
#define syn_err_static_assert_failed "static assertion failed"
#define syn_err_static_assert_failed_msg "static assertion failed: $e"
#define syn_err_expected_member "Expected <member> but found $l"
#define syn_err_hashif_eof "EOF not newline after #if ..."
#define syn_err_hashif_junk "Junk after #if <expression>"
#define syn_err_initialisers "too many initialisers in {} for aggregate"
#define syn_err_initialisers1 "{} must have 1 element to initialise scalar"
#define syn_rerr_empty_init "Initialiser list must contain at least one expression"
#define syn_err_default "'default' not in switch - ignored"
#define syn_err_default1 "duplicate 'default' case ignored"
#define syn_err_case "'case' not in switch - ignored"
#define syn_err_case1 "duplicated case constant: %ld"
#define syn_err_expected_cmd "<command> expected but found $l"
#define syn_err_expected_stmt "<statement> expected but found $l"
#define syn_err_expected_while "'while' expected after 'do' but found $l"
#define syn_err_else "Misplaced 'else' ignored"
#define syn_err_continue "'continue' not in loop - ignored"
#define syn_err_break "'break' not in loop or switch - ignored"
#define syn_err_no_label "'goto' not followed by label - ignored"
#define syn_err_no_brace "'{' of function body expected - found $l"
#define syn_err_stgclass \
        "storage class $s not permitted in context %s - ignored"
#define syn_err_stgclass1 "storage class $s incompatible with $g - ignored"
#define syn_err_typeclash "type $s inconsistent with $m"
#define syn_err_tag_brace \
        "'{' or <identifier> expected after $s but found $l"
#define syn_err_expected3 "Expecting <declarator> or <type> but found $l"
#define syn_err_unneeded_id \
        "superfluous $l in <abstract declarator> - ignored"
#define syn_err_undef_struct_member(_b,_s) \
        "undefined $c member: $r", _b, _s
#define syn_err_undef_struct_object(_b,_s) \
        "undefined $c object: $r", _b, _s
#define syn_err_selfdef_struct_member(_b,_s) \
        "attempt to include $c member: $r within itself", _b, _s
#define syn_err_selfdef_struct_object(_b,_s) \
        "attempt to include $c object: $r within itself", _b, _s
#define syn_err_void_object_member(_s) "illegal 'void' member: $r", _s
#define syn_err_void_object_object(_s) "illegal 'void' object: $r", _s
#define syn_err_duplicate_type \
        "duplicate type specification of formal parameter $r"
#define syn_err_not_a_formal "Non-formal $r in parameter-type-specifier"
#define syn_err_cant_init "$g names may not be initialised"
#define syn_err_enumdef \
        "<identifier> expected but found $l in 'enum' definition"
#define syn_err_undef_enum "Undefined enum $r"
#define syn_err_misplaced_brace "Misplaced '{' at top level - ignoring block"
#define syn_err_not_friend "not a friend class $r"
#define syn_err_undef_basetag "undefined base: $c"
#define syn_err_missing_basetag "missing base tag: $l"
#define syn_err_incomp_arg_ovld "Incompatible arguments to $b (overload missing?)"
#define syn_err_no_operator_ovld "No operator $s with this type signature"
#define syn_err_multi_overload_call "ambiguous %d-way overload for call to $b"
#define syn_err_multi_operator_ovld "ambiguous %d-way overload for operator $s"
#define syn_err_multi_convfn_ovld \
    "ambiguous %d-way choice of conversion from $c to $t"
#define syn_err_multi_ctor_ovld \
    "ambiguous %d-way choice of constructor from $t to $c"
#define syn_err_ambiguous_user_defined_conv \
    "ambiguous implicit user-defined conversion from $c to $c"
#define syn_err_illegal_asm_decl "illegal asm(...) declaration (ignored)"
#define syn_err_recursive_app_operator_arrow \
        "recursive application of operator->() to object of type $c; return type intended?"

#define sem_err_assign_ovld \
    "no suitable operator=() for $c: overload missing?"
#define sem_warn_virtual "virtual function call of $b"
#define sem_warn_addr_of_memfn "address of member function $b"
#define xsem_warn_cast_undef_struct \
    "cast from ptr/ref to $c to ptr/ref to $c; one is undefined, assuming unrelated"
#define xsem_warn_unreachable_handler "handler is unreachable"

#define syn_err_constructor_init "constructor forbids $r = {...} initialiser"
#define syn_err_no_named_member "$c has no $r member"
#define syn_err_not_found_named_member "member $r not found in $c"
#define syn_err_missing_named_mfn "Missing class member function name"
#define syn_err_classname_not_found "class-name $r not found"
#define syn_err_member_cannot_init "member cannot be initialised: $r"
#define syn_err_lacks_bclass_anachronism "$c lacks base class for ':(...)' anachronism"
#define syn_err_not_member "$l is not a member of struct/class $b"
#define syn_err_try_catch "'try-catch' unimplemented"
#define syn_err_catch_ignored "misplaced 'catch' ignored"
#define syn_err_illegal_simple_types "illegal <simple type>: $m"
#define syn_err_no_member_here "class member $b cannot be defined here"
#define syn_err_template_not_impl "'template' not implemented"
#define syn_err_friend_type "'friend <type>;' needs elaborated-type-specifier"
#define syn_err_linkage_spec "expected <linkage-spec> '}' before $l"
#define syn_err_illegal_operator "'operator $s' is illegal"
#define syn_err_wrong_args_ovld "wrong number of arguments to overload $b"
#define syn_err_cannot_follow_unary_dcolon "$l cannot follow unary '::'"
#define syn_err_cannot_follow_binary_dcolon "$l cannot follow binary '::'"
#define syn_err_expected_dtor_name "expected destructor name after ::~, found $l"
#define syn_err_missing_tlevel_decl "missing top-level declaration: $r"
#define syn_err_no_decl_at_type "no $b declaration at this type"
#define syn_err_lacks_arg_ctor "$t lacks an %ld-argument constructor"
#define syn_err_no_nullary_ctor "no nullary constructor: $b"
#define syn_err_ignored_new_array_ctor "new <array> initialiser ignored"
#define syn_err_requires_pntr_arg "requires pointer argument: $s"
#define syn_err_duplicated_mem_init "duplicate member initialisation: $r"
#define syn_err_init_not_in_ctor "member initialiser not in constructor"
#define syn_err_expected_id_in_mem_init "expecting <identifier> in <member-initialiser>"
#define syn_err_missing_catch_after_try "omitted 'catch' after 'try'"
#define syn_err_ill_this "legal only in member function: $l"
#define syn_err_template_notclassorfunction "'template' not class nor function"
#define syn_err_template_scope "invalid $c scope"
#define syn_err_unspecialized_template "attempt to use template $b"
#define syn_err_asm_not_available "inline assembler not available in strict ANSI mode"

#define sem_err_temp_type_float "floating point type not allowed"
#define sem_err_addr_template "attempt to take address of template $b"
#define sem_err_typededuce_disagree "type deduction fails: $t disagrees with $t"
#define sem_warn_typededuce_arraysize "type deduction fails: array size differs"
#define sem_err_typededuce_ambiguous "type deduction fails: function type %d-way resolvable"
#define sem_err_typededuce_pointer "type deduction fails: $t incompatible to $t"
#define sem_err_typededuce_recognize "type deduction fails: un-recognizable type $t"
#define sem_err_typededuce_type "type deduction fails: free template type arg $r"
#define sem_err_typededuce_const "type deduction fails: free template non-type arg $r"
#define sem_err_typename_not_found "type deduction failed: typename $b not found\n"
#define sem_err_template_ambiguous "Ambiguous class/function templates"
#define sem_err_non_type_arg_linkage "extern linkage expected for template argument $b"
#define sem_err_non_type_arg_value "illegal non-type template arg $e"
#define sem_err_template_arg_linkage "template type arg $t must have extern linkage"
#define sem_err_call_dependency "call to $e not dependent on template args to $c"
#define sem_err_type_arg_expected "template type arg expected"
#define sem_err_typeclash_1 "type $m is inconsistent with $m"
#define sem_err_typeclash_2 "template type expected, found $e"
#define sem_err_typeclash_3 "template type inconsistent with $t"
#define sem_err_lvalue_needed "temporary required for template argument"
#define sem_err_dotdotdot_handler_not_last "handler for '...' must come last"
#define sem_rerr_template_formal_length_class \
        "number of template formals for $c differs from a previous declaration"
#define sem_rerr_template_formal_length_function \
        "number of template formals for $b differs from a previous declaration"
#define sem_rerr_template_formal_type "template formal type $t inconsistent with $t"

#define vargen_err_long_string "string initialiser longer than %s [%ld]"
#define vargen_err_nonstatic_addr \
        "non-static address $b in pointer initialiser"
#define vargen_err_bad_ptr "$s: illegal use in pointer initialiser"
#define vargen_err_init_void "objects of type 'void' can not be initialised"
#define vargen_err_undefined_struct \
        "$c must be defined for (static) variable declaration"
#define vargen_err_open_array "Uninitialised static [] arrays illegal"
#define vargen_err_overlarge_reg "invalid global register number"
#define vargen_err_not_int "invalid type for global int register"
#define vargen_err_not_float "invalid type for global float register"
#ifdef TARGET_CALL_USES_DESCRIPTOR
#define vargen_err_badinit "illegal initialisation to $r%+ld"
#endif
#ifdef TARGET_IS_HELIOS
#define vg_err_dynamicinit "Initialised dynamic array with -ZR or -ZL"
#endif
#define vargen_rerr_nonaligned \
        "Non-aligned ADCON at data+0x%lx (value $r+0x%lx) set to NULL"
#define vargen_rerr_datadata_reloc \
       "RISC OS (or other) reentrant module has static init. to data $r"
#define vargen_rerr_compiler_confused \
        "compiler confused: static const with dynamic initialisation"
#define vargen_rerr_local_static_with_dtor \
        "unimplemented: local static with destructor: $b"

#define lex_rerr_8_or_9 "digit 8 or 9 found in octal number"
#define lex_rerr_pp_number "number illegally followed by letter"
#define lex_rerr_hex_exponent "hex number cannot have exponent"
#define lex_rerr_esc16_truncated \
        "overlarge escape '\\x%s%lx' treated as '\\x%lx'"
#define lex_rerr_esc8_truncated "overlarge escape '\\%o' treated as '\\%o'"
#define lex_rerr_illegal_esc "illegal string escape '\\%c' - treated as %c"
#define lex_rerr_not1wchar "L'...' needs exactly 1 wide character"
#define lex_rerr_empty_char "no chars in character constant ''"
#define lex_rerr_overlong_char "more than 4 chars in character constant"

#define syn_rerr_qualified_void "Return type may not be a void type other than 'void'"
#define syn_rerr_array_0 "array [0] found"
#ifdef EXTENSION_VALOF
#define syn_rerr_void_valof "void valof blocks are not permitted"
#endif
#define syn_rerr_undeclared "undeclared name, inventing 'extern int %s'"
#define syn_rerr_undeclared_fn "undeclared name, inventing 'extern \"C\" int %s(...);'"
#define syn_rerr_insert_parens \
        "parentheses (..) inserted around expression following $s"
#define syn_rerr_return_expr_void "return <expr> illegal for void function"
#define syn_rerr_return_expr_ctor "return <expr> illegal for constructor"
#define syn_rerr_return_expr_dtor "return <expr> illegal for destructor"
#define syn_rerr_qualified_typedef(_b,_m) \
        "$m typedef $b has $m re-specified", _m, _b, _m
#define syn_rerr_no_quals_allowed \
        "only non-static member functions can be const or volatile"
#define syn_rerr_missing_type "missing type specification - 'int' assumed"
#define syn_rerr_missing_type_for "missing type specification for $r - 'int' assumed"
#define syn_rerr_long_float "ANSI C does not support 'long float'"
#define syn_rerr_missing_type1 \
        "omitted <type> before formal declarator - 'int' assumed"
#define syn_rerr_missing_type2 \
        "function prototype formal $r needs type or class - 'int' assumed"
#define syn_rerr_ellipsis_first "ellipsis (...) cannot be only parameter"
#define syn_rerr_mixed_formals "prototype and old-style parameters mixed"
#define syn_rerr_open_member "illegal [] member: $r"
#define syn_rerr_ref_void "illegal type (void &) treated as (int &)"
#define syn_rerr_ill_ref "$t of reference illegal -- '&' ignored"
#define syn_rerr_fn_returntype "function returning $t illegal -- assuming pointer"
#define syn_rerr_abst_class_rtype(_c)\
        "function return abstract $c illegal", _c
#define syn_rerr_array_elttype "array of $t illegal -- assuming pointer"
#define syn_rerr_fn_ptr_member(_s) \
   "member $r may not be function -- assuming pointer", _s
#define syn_rerr_fn_ptr_object(_s) \
   "object $r may not be function -- assuming pointer", _s
#define syn_rerr_fn_ptr1 \
        "function $r may not be initialised - assuming function pointer"
#define syn_rerr_archaic_init "Ancient form of initialisation, use '='"
#define syn_rerr_bitfield "illegal bit field type $t - 'int' assumed"
#define syn_rerr_ANSIbitfield "ANSI C forbids bit field type $t"
#define syn_rerr_missing_formal "formal name missing in function definition"
#define syn_rerr_ineffective "declaration with no effect"
#define syn_rerr_duplicate_member(b) "duplicate member $b", b
#define syn_rerr_duplicate_member2(cl, sv) "duplicate member $p$r", cl, sv
#define syn_rerr_semicolon_in_arglist \
        "',' (not ';') separates formal parameters"
#define syn_rerr_no_members "$c has no members"
#define syn_rerr_not_base(mm,cc) "$b is not a base member of $c",mm,cc
#define syn_rerr_badly_placed_access \
    "access declarations only in public and protected parts"
#define syn_rerr_modify_access "base access rights cannot be altered"
#define xsyn_rerr_bad_conv "illegal conversion $p$r"
#define xsyn_rerr_zero_params "$p$r must have zero parameters"
#define xsyn_rerr_non_memfn_operator "$p$r must be a non-static member function"
#define xsyn_rerr_no_return_type_allowed "no return type allowed for $p$r"
#define xsyn_rerr_must_be_function "$p$r must be a function"
#define syn_rerr_jump_past_init "jump past initialisation for $b"
#define syn_rerr_ambiguous_qualification "$r is ambiguously qualified"
#define syn_rerr_abstract_class_member(_b,_s) \
        "abstract $c member: $r", _b, _s
#define syn_rerr_abstract_class_object(_b,_s) \
        "abstract $c object: $r", _b, _s
#define syn_rerr_opaque_class_member(_b,_s) \
        "opaque $c member: $r", _b, _s
#define syn_rerr_opaque_class_object(_b,_s) \
        "opaque $c object: $r", _b, _s
#define syn_rerr_insert_braces "inserting { } around command after $s"
#define syn_rerr_not_decl_qual(tb,s) \
        "Definition of $c not $s: qualifier ignored", tb, s
#define syn_rerr_ignored_non_fn "$g ignored for non-function $r"
#define syn_rerr_global_anon_union "global anonymous union must be static"
#define syn_rerr_defaults_ordering "defaulted parameter $r followed by non-defaulted"
#define syn_rerr_unknown_linkage "unknown linkage: extern $e"
#define syn_rerr_illegal_anon_union_mem "illegal anonymous union member $r"
#define syn_rerr_illegal_nonpub_anon_union_mem "illegal non-public anonymous union member $r"
#define syn_rerr_delete_expr_anachronism "'e' ignored in 'delete [e]' anachronism"
#define syn_rerr_self_copying_ctor "self-copying constructor for $c"
#define syn_rerr_duplicated_base "duplicate base $c ignored"
#define syn_rerr_no_arg_in_template "no arguments in template<>"
#define syn_rerr_superfluous_access_adjuster \
        "superfluous type for access adjuster, declaration ignored"
#define syn_rerr_private_bmember_ignored "private overloaded base member ignored"
#define syn_rerr_local_default "$b has auto storage"
#define syn_rerr_declaree_out_of_scope "'$#b::$#r' cannot be declared here"
#define syn_rerr_friend_class_not_definable "friend $c shall not be defined"
#define syn_rerr_expect_dtor "expecting destructor for $c, found $r"
#define syn_rerr_union_w_base "unions may not have bases"
#define syn_rerr_meminit_wrong_args \
   "too few/many arguments to initialiser for simple member $b"
#define syn_rerr_ref_not_initialised "reference $b must be initialised"
#define syn_rerr_const_not_initialised "constant $b must be initialised"
#define syn_rerr_addrof_cdtor_taken "can't take address of constructor or destructor"
#define syn_rerr_multi_convfn_bool \
    "ambiguous %d-way choice of conversion from $c in Boolean context"
#define syn_rerr_tagdef_in_formals "cannot define within formals: $c"
#define syn_rerr_neg_unsigned_enum "$c cannot have both negative and unsigned enumerators"
#define xsyn_rerr_instantiate_mixup "instantiate don't mix with template or specialization"
#define xsyn_rerr_spurious_instantiate "too many instantiate request"
#define syn_rerr_extra_template_actuals "extra template actual(s) ignored"

/* Split for the NLS */
#define syn_moan_hashif_nonconst "illegal in #if <expression>: <unknown>"
#define syn_moan_hashif_nonconst1 \
     "illegal in #if <expression>: non constant $b"
#define syn_moan_hashif_nonconst2 "illegal in #if <expression>: $s"
#define syn_moan_case_nonconst \
     "illegal in case expression (ignored): <unknown>"
#define syn_moan_case_nonconst1 \
     "illegal in case expression (ignored): non constant $b"
#define syn_moan_case_nonconst2 \
     "illegal in case expression (ignored): $s"

%Z      /* The following remain as ordinary (uncompressed) strings */

#define syn_moan_hashif "#if <expression>"
#define syn_moan_case "case expression (ignored)"
#define xsyn_constructor_string "constructor"
#define xsyn_copy_constructor_string "copy constructor"
#define xsyn_destructor_string "destructor"
#define xsyn_copy_assign_string "copy assignment"

#endif                          /* Under NLS these are not tags */

%S /* The next batch of things just get mapped onto syserr codes */

#define syserr_genpointer "genpointer&(%ld)"
#define syserr_initsubstatic "initsubstatic(bit)"
#define syserr_initstatic "initstatic(%ld,%#lx)"
#define syserr_initstatic1 "initstatic(%ld)"
#define syserr_rd_decl_init "rd_decl/init(%#lx)"
#define syserr_rd_typename "rd_typename()=0"
#define syserr_rdinit "syn_rdinit"
#define syserr_rd_declarator "rd_declarator(%ld)"
#define syserr_defaultstgclass "defaultstorageclass(%#x)"
#define syserr_rd_declrhslist "rd_declrhslist confused"
#define syserr_rd_decl2 "rd_decl2(%p,%ld)"
#define syserr_rd_strdecl "rd_strdecl"
#define syserr_lex_string "lex_string"

/* end of cfe/feerrs.h */
