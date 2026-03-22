/*
 * C compiler file aeops.h, Version 46
 * Copyright (C) Codemist Ltd., 1988-1992.
 * Copyright Advanced RISC Machines Limited, 1990-1992.
 * SPDX-Licence-Identifier: Apache-2.0
 */

/*
 * RCS $Revision$
 * Checkin $Date$
 * Revising $Author$
 */

#ifndef _aeops_LOADED
#define _aeops_LOADED 1

/* Notes (AM):
   0) This is a core file of tree and syntax operators.
      Only some operators appear in the tree -- others are just
      punctuation or for language frontend internal use.
   1) some lexical operators, like ++ and & correspond to more
      than one AE tree operation (pre/post incr. and bitand/addrof).
      The lexical routines are all assumed to return the operator
      corresponding to the binary or prefix form.
      Macros unaryop_() and postop_() are here provided for the syntax
      analyser to use for getting the other form.
   2) Assignment operators are treated similarly.  This time by
      the lexical routines which use assignop_() to get the
      assignment form.
   3) It goes without saying that the caller must only apply
      these macros to valid arguments.
   4) s_and provides a good view of this.
   5) some AE operators do not have tokens (e.g. s_fnap, s_block).
      I have tried to enclose these in parens in the following.
*/

/* Here is a list of all tokens types used in C                        */

typedef enum AE_op {
    s_nothing,

/* identifier, constants: */
    s_error,
    s_unknown,          /* e.g. template parameter value.               */
#define iserrororunknown_(op) ((op) <= s_unknown)
    s_identifier,
    s_integer,
    s_floatcon,
    s_int64con,
    s_boolean,          /* mainly represented by s_integer (type char)  */
    s_character,        /* mainly represented by s_integer (type bool)  */
    s_wcharacter,       /* mainly represented by s_integer (type wchar) */
    s_string,
#ifdef EXTENSION_UNSIGNED_STRINGS
    s_ustring,
    s_sstring,
#endif
    s_wstring,
/* s_binder heads binding records - see type Binder */
    s_binder,
    s_tagbind,  /* AM may want this soon LDS wants NOW */
    s_member,   /*  LDS wants this too... for C++       */

#define hastypefield_(op) ((s_invisible<=(op) && (op)<=s_cast))
#define hasfileline_(op) ((op)==s_integer || (hastypefield_(op)))

    s_invisible,
#define isdiad_(op)       (s_andand<=(op) && (op)<=s_assign)
/* expression operators, mainly non-letter symbols */
    s_andand,
    s_comma,
    s_oror,
    s_arrowstar,                /* C++ only */
    s_dotstar,                  /* C++ only */

/* relations: reorder to make NOT or SWAP easier? */
#define isrelational_(op) (s_equalequal<=(op) && (op)<=s_lessequal)
    s_equalequal,
    s_notequal,
#define isinequality_(op) (s_greater<=(op) && (op)<=s_lessequal)
    s_greater,
    s_greaterequal,
    s_less,
    s_lessequal,

/* NB: the next 3 blocks of opcodes must correspond. */

#define floatableop_(op) \
    (isrelational_(op)||(s_times<=(op) && (op)<=s_div)||(op) == s_cond)

    s_and,
    s_times,
    s_plus,
    s_minus,
    s_power,            /* Needed for Fortran */
    s_div,
    s_leftshift,
    s_or,
    s_idiv,
    s_rem,
    s_rightshift,
    s_xor,

#define can_have_becomes(x) (((x)>= s_and) && ((x) <= s_xor))
#define and_becomes(x)  ((AE_op)((x)+(s_andequal-s_and)))
#define assignop_(op)   ((AE_op)((op)+(s_andequal-s_and)))
#define unassignop_(op) ((AE_op)((op)-(s_andequal-s_and)))
#define isassignop_(x)  (((x)>= s_andequal) && ((x) <= s_xorequal))

    s_andequal,
    s_timesequal,
    s_plusequal,
    s_minusequal,
    s_powerequal,       /* here for consistency - do not remove */
    s_divequal,
    s_leftshiftequal,
    s_orequal,
    s_idivequal,
    s_remequal,
    s_rightshiftequal,
    s_xorequal,

#define diadneedslvalue_(op)    (s_andequal<=(op) && (op)<=s_assign)
    s_displace,         /* no repn in C,  ++ is spec case */
    s_init,             /* initialising '=' */
    s_assign,

/* unary ops.  note that the first 4 correspond (via unaryop_()) to diads */
#define ismonad_(op)    (s_addrof<=(op) && (op)<=s_postdec)
#define unaryop_(x)     ((AE_op)((x) + (s_addrof-s_and)))
    s_addrof,
    s_content,
    s_monplus,
    s_neg,
    s_bitnot,
    s_boolnot,
    s_content4,
    s_evalerror,        /* A special sort of invisible mode */
/* move the next block of 4 to just before s_addrof? */
#define monadneedslvalue_(op)   (isincdec_(op) || (op)==s_addrof)
#define isincdec_(op)           (s_plusplus<=(op) && (op)<=s_postdec)
#define incdecisinc_(op)        ((op) <= s_postinc)
#define incdecispre_(op)        ((((op)-s_plusplus) & 1) == 0)
#define postop_(preop)          ((AE_op)((preop)+(s_postinc-s_plusplus)))
    s_plusplus,
    s_postinc,
    s_minusminus,
    s_postdec,
/* end of unary ops */
    s_dot,
    s_arrow,
    s_cond,
    s_throw,            /* CPLUSPLUS */

/* pseudo expression operators */
    s_fnap,
    s_fnapstruct,
    s_fnapstructvoid,
    s_subscript,
    s_let,
#ifdef EXTENSION_VALOF
    s_valof,            /* BCPL-like valof block support */
#endif
#ifdef RANGECHECK_SUPPORTED
    s_rangecheck,
    s_checknot,
#endif
    s_qualdot,          /* C++ expr.T::f() - no virtual call */
    s_const_cast,       /* CPLUSPLUS */
    s_dynamic_cast,     /* CPLUSPLUS */
    s_reinterpret_cast, /* CPLUSPLUS */
    s_static_cast,      /* CPLUSPLUS */
    s_cast,             /* see hastypefield_() above */
    s_sizeoftype,
    s_sizeofexpr,
    s_typeoftype,
    s_typeofexpr,
    s_ptrdiff,

/* mip/cg.c command nodes (mainly keywords): */
    s_break,
    s_case,
    s_catch,            /* CPLUSPLUS */
    s_colon,
    s_continue,
    s_default,
    s_do,
    s_endcase,          /* C switch break = bcpl endcase */
    s_for,
    s_goto,
    s_if,
    s_return,
    s_semicolon,
    s_switch,
    s_while,
    s_block,
    s_thunkentry,       /* C++ virtual functions */

#ifdef EXTENSION_VALOF
    s_resultis,         /* used with valof blocks */
#endif

/* declaration nodes: */
    s_decl,
    s_fndef,
    s_typespec,

/* note the next two blocks must be adjacent for the next 3 tests. */
#define istypestarter_(x)   (s_bool<=(x) && (x)<=s_lasttype)
#define isstorageclass_(x)  (s_auto<=(x) && (x)<=s_globalfreg)
#define isdeclstarter_(x)   (s_bool<=(x) && (x)<=s_typestartsym)
#define shiftoftype_(x)     ((x)-s_bool)
#define bitoftype_(x)       (1L<<((x)-s_bool))
/* soon: stop bool, float, wchar being a proper type.  Map bool to      */
/* short char, and wchar to long char.  syn.c still needs older form?   */
    s_wchar,            /* reserved, not yet used.      */
    s_bool,
    s_char,
    s_double,
    s_enum,
    s_float,
    s_int,
    s_struct,
    s_class,            /* here, whether or not C++     */
    s_union,
#  define CLASSBITS         (bitoftype_(s_union+1)-bitoftype_(s_struct))
#  define ENUMORCLASSBITS   (CLASSBITS|bitoftype_(s_enum))
    s_void,
    s_longlong,         /* C extension */
    s_typedefname,
#define NONQUALTYPEBITS (bitoftype_(s_typedefname+1)-bitoftype_(s_bool))
/* modifiers last (high bits for m&-m trick) */
    s_long,
    s_short,
    s_signed,
    s_unsigned,
/* rework the next two lines?                                           */
#define TYPEDEFINHIBITORS   (bitoftype_(s_unsigned+1)-bitoftype_(s_bool))
#define CVBITS              (bitoftype_(s_unaligned+1)-bitoftype_(s_const))
    s_const,
    s_volatile,
    s_unaligned,
#define s_lasttype s_unaligned
#define NUM_OF_TYPES        (s_lasttype-s_bool+1)
    /* currently 18 */
#define TYPEBITS            (bitoftype_(s_lasttype+1)-bitoftype_(s_bool))
    s_opaque,   /* not in TYPEBITS since it is only kept for tagbinders */
/* storage classes and qualifiers */
#define bitofstg_(x)        (1L<<((x)-s_auto+16))
/* ***NOTE*** bitofstg_() is ***NOT*** the same as bitoftype_().
   Callers must be careful to use the right one.
 */
    s_auto,
    s_extern,
    s_static,
    s_typedef,
    s_globalreg,
    s_register,
    s_friend,
    s_inline,
    s_virtual,
    s_weak,
/* N.B. s_register is equivalent to 0x100000.  See h.defs for other bits */
#define PRINCSTGBITS        (bitofstg_(s_register)-bitofstg_(s_auto))
#define STGBITS             (bitofstg_(s_weak+1)-bitofstg_(s_auto))
#define NUM_OF_STGBITS      (s_weak-s_auto+1)
    /* currently 10 */
    s_globalfreg,
#define bitoffnaux_(x)      ((FnAuxFlags)1<<((x)-s_pure))
#define isfnaux_(x)         (s_pure<=(x) && (x)<s_typestartsym)
    s_pure,
    s_structreg,
    s_swi,
    s_swi_i,
    s_irq,
    s_commutative,

    s_typestartsym,     /* used to help error reporting */

/* impedementia (not appearing in parse trees) */
    s_mutable,          /* CPLUSPLUS - currently ignored */
    s_explicit,         /* CPLUSPLUS - currently ignored */
    s_namespace,        /* CPLUSPLUS - currently ignored */
    s_using,            /* CPLUSPLUS - currently ignored */
    s_else,
    s_toplevel,
    s_lbrace,
    s_lbracket,
    s_lpar,
    s_rbrace,
    s_rbracket,
    s_rpar,
    s_try,
    s_typeof,           /* 2 special cases above */
    s_sizeof,           /* 2 special cases above */
    s_typeid,           /* CPLUSPLUS */
    s_ellipsis,
    s_eol,
    s_eof,
    s_hash,             /* inline assembler */
    s_quote,            /* inline assembler */

/* C++ keywords not in ANSI C.                                          */
/* AM: memo, arrange non-aetree ops to be treated by langfe\*.[ch]      */
/* via s_frontend to avoid mip getting lots of ops per language.        */
    s_asm,
    s_delete,
    s_new,
    s_operator,
    s_template,
    s_typename,         /* names the following id a type, post-ARM feature */
    s_export,
    s_static_assert,    /* C11/C++ static assertion declaration */
    s_this,

#  define isaccessspec_(op) (s_private <= (op) && (op) <= s_public )
/* bitofaccess_() bits are contiguous with CLASSBITS... */
#  define bitofaccess_(x)  (1L<<((x)-s_private+shiftoftype_(s_union+1)))
#  define ACCESSBITS  (bitofaccess_(s_public+1)-bitofaccess_(s_private))
    s_private,
    s_protected,
    s_public,

/* non-keyword C++ operators... */
    s_coloncolon,
    s_ovld,
    s_convfn,           /* C++ front end only (temp?)   */
    s_pseudoid,         /* C++ things like operator +   */
    s_cppinit,          /* C++ syntax node for int a(5) */
    s_memfndef,         /* local to C++ f.e.            */
    s_ctor,             /* C++ front end only.          */
    s_dtor,             /* C++ front end only.          */

#ifdef PASCAL           /* PASCAL front-end symbols (not tree nodes) */
    s_array,
    s_begin,
    s_downto,
    s_else,
    s_end,
    s_file,
    s_function,
    s_in,
    s_label,
    s_nil,
    s_of,
    s_packed,
    s_procedure,
    s_program,
    s_record,
    s_repeat,
    s_set,
    s_then,
    s_to,
    s_type,
    s_until,
    s_var,
    s_with,
#endif                  /* PASCAL */

#ifdef BCPL
    s_global,
    s_manifest,
    s_abs,
    s_get,
    s_else,
    s_eqv,
    s_query,
    s_vecap,
    s_andlet,
    s_be,
    s_by,
    s_false,
    s_finish,
    s_into,
    s_repeat,
    s_repeatwhile,
    s_repeatuntil,
    s_test,
    s_true,
    s_table,
    s_unless,
    s_vec,
    s_valdef,
#endif                  /* BCPL */

#ifdef FORTRAN
    s_boolean,          /* needed for Fortran */
    s_complex,          /* needed for Fortran */
#endif

#ifndef BCPL
    s_true,
    s_false,
#endif

    s_SPARE1,
    s_SPARE2,

    s_NUMSYMS
} AE_op;

/* Any unqualified symbol is less than s_NUMSYMS <= s_SYMMASK */
#define s_SYMMASK       255
/* ORR this bit in to qualify a symbol... a hack for C++...   */
#define s_qualified     512

/* synonyms (used in types for clarity) -- soon a separate ADT          */
/* (along with s_typespec!, but see use of t_coloncolon etc in syn.c).  */

#define t_fnap      s_fnap
#define t_subscript s_subscript
#define t_content   s_content
#define t_ref       s_addrof
#define t_coloncolon  s_coloncolon
#define t_ovld      s_ovld
#define t_unknown   s_unknown       /* template type parameter          */

#ifdef EXTENSION_UNSIGNED_STRINGS
#  define case_s_any_string  case s_string: case s_wstring: \
                             case s_ustring: case s_sstring:
#else
#  define case_s_any_string  case s_string: case s_wstring:
#endif
#define isstring_(op) (s_string<=(op) && (op)<=s_wstring)

#endif

/* we use 'short long int' internally to represent 'long long int'.     */
#define ts_longlong   (bitoftype_(s_int)|bitoftype_(s_short)|bitoftype_(s_long))
#define ts_long       (bitoftype_(s_int)|bitoftype_(s_long))
#define ts_short      (bitoftype_(s_int)|bitoftype_(s_short))
#define ts_int        (bitoftype_(s_int))
#define ts_float      (bitoftype_(s_short)|bitoftype_(s_double))
#define ts_double     (bitoftype_(s_double))
#define ts_longdouble (bitoftype_(s_long)|bitoftype_(s_double))

#define int_isshort_(m) \
    (((m) & (bitoftype_(s_long) | bitoftype_(s_short))) ==\
            bitoftype_(s_short))
#define is_float_(m) \
    (((m) & (bitoftype_(s_short) | bitoftype_(s_double))) ==\
            (bitoftype_(s_short)|bitoftype_(s_double)))
#define is_double_(m) \
    (((m) & (bitoftype_(s_long)|bitoftype_(s_short) | bitoftype_(s_double))) ==\
            bitoftype_(s_double))
#define is_longdouble_(m) \
    (((m) & (bitoftype_(s_long) | bitoftype_(s_double))) ==\
            (bitoftype_(s_long)|bitoftype_(s_double)))
#define is_anydouble_(m) \
    (((m) & (bitoftype_(s_short) | bitoftype_(s_double))) ==\
            bitoftype_(s_double))
#define int_islonglong_(m) \
    (((m) & (bitoftype_(s_long) | bitoftype_(s_short))) ==\
     (bitoftype_(s_long)|bitoftype_(s_short)))
#define int_decodelength_(m) \
    (((m) & bitoftype_(s_short)) ? \
         (((m) & bitoftype_(s_long)) ? sizeof_longlong : sizeof_short) : \
         (((m) & bitoftype_(s_long)) ? sizeof_long : sizeof_int))
#define int_decodealign_(m) \
    (((m) & bitoftype_(s_short)) ? \
         (((m) & bitoftype_(s_long)) ? alignof_longlong : alignof_short) : \
         (((m) & bitoftype_(s_long)) ? alignof_long : alignof_int))

/* end of file aeops.h */
