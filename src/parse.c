/* A Bison parser, made from parse.y
   by GNU bison 1.35.  */

#define YYBISON 1  /* Identify Bison output.  */

# define	CONSTANT	257
# define	STRING	258
# define	ID	259
# define	_NULL	260
# define	IF	261
# define	ELSE	262
# define	ELSEIF	263
# define	WHILE	264
# define	BREAK	265
# define	CONTINUE	266
# define	FUNCTION_	267
# define	FOR	268
# define	IN	269
# define	RETURN	270
# define	LOCAL	271
# define	SELF	272
# define	SYMBOL_TABLE	273
# define	TRY	274
# define	CATCH	275
# define	VEIL	276
# define	BINOP_ASSIGN	277
# define	CAT	278
# define	SHORT_OR	279
# define	SHORT_AND	280
# define	OR	281
# define	AND	282
# define	RELOP	283
# define	ADDOP	284
# define	MULOP	285
# define	UNARY	286
# define	TRANS	287

#line 10 "parse.y"

#include "algae.h"
#include "entity.h"
#include "psr.h"
#include "pmem.h"
#include "code.h"
#include "datum.h"
#include "exception.h"
#include "for_loop.h"
#include "message.h"
#include "function.h"
#include "error.h"

static char rcsid[] =
  "$Id: parse.y,v 1.8 2003/08/07 02:34:09 ksh Exp $";

#define YYPURE   1

/*
 * Bison wants to extend the stack, if necessary.  But it uses alloca,
 * which is not very portable.  Since I don't know of any way to
 * overflow the stack with Algae, I'll just cause bison to raise an
 * exception if an overflow ever does occur.
 */

#define yyoverflow(msg,ss1,ssp,vs1,vsp,sz) \
	do { fail( msg ); raise_exception(); } while (0)

#define A2I(p)  do { if      ( (p)->op == OP_PUSHA  ) (p)->op = OP_PUSHI; \
		     else if ( (p)->op == OP_LPUSHA ) (p)->op = OP_LPUSHI; \
		     else \
		       { \
			   assert( (p)->op == OP_PUSHA_SYM ); \
			   (p)->op = OP_PUSHI_SYM; \
		       } } while (0)

static int exec_flag;	/* True if terminator was followed by newline. */
static int line_flag;	/* True if OP_LINE has been coded for current line. */
static UF_BLOCK *active_function ;

/*-------- Can't allow bare break or continue in try. ----------*/

static unsigned int control_register;

#define push_try() \
  do { control_register <<= 1; control_register |= 01; } while (0)
#define pop_try() (control_register >>= 1)
 
#define push_loop() \
  do { control_register <<= 1; } while (0)
#define pop_loop() (control_register >>= 1)

#define in_try() (control_register & 01)

/*--------------------------------------------------------------*/

extern DATUM the_null ;

static SYMTAB* PROTO(check_id, (char *)) ;
int PROTO (yyparse, (void));
void PROTO (yyrestart, ( FILE *input_file ));


#line 74 "parse.y"
#ifndef YYSTYPE
typedef union {
    int ival ;
    OFFSET start ;
    DATUM *datum ;
    char *id_name ;		/* space obtained from malloc() */
    char *cs ;			/* constant string obtained from pmem */
    UF_BLOCK *ufp ;
    struct {
	unsigned int start;	  /* where expression starts */
	unsigned int op     : 24; /* where "control" op (like OP_PUSHA) is */
	unsigned int memcnt :  8; /* number of members */
    } twofer ;
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
int yylex(YYSTYPE *);
#ifndef YYDEBUG
# define YYDEBUG 0
#endif



#define	YYFINAL		240
#define	YYFLAG		-32768
#define	YYNTBASE	48

/* YYTRANSLATE(YYLEX) -- Bison token number corresponding to YYLEX. */
#define YYTRANSLATE(x) ((unsigned)(x) <= 287 ? yytranslate[x] : 109)

/* YYTRANSLATE[YYLEX] -- Bison token number corresponding to YYLEX. */
static const char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      42,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    35,     2,     2,     2,     2,     2,     2,
      43,    44,     2,     2,     2,     2,    41,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    45,     7,
       2,    25,     2,     6,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    39,     2,    40,    37,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    47,     2,    46,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     3,     4,     5,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    36,    38
};

#if YYDEBUG
static const short yyprhs[] =
{
       0,     0,     1,     4,     8,    10,    13,    16,    18,    20,
      23,    25,    27,    29,    32,    34,    35,    37,    39,    41,
      43,    45,    47,    49,    51,    53,    57,    61,    63,    65,
      67,    71,    75,    79,    82,    86,    90,    94,    97,   100,
     104,   106,   108,   112,   113,   115,   118,   122,   124,   128,
     132,   138,   143,   145,   150,   154,   158,   160,   162,   166,
     170,   172,   179,   186,   187,   189,   191,   195,   199,   201,
     203,   207,   208,   210,   214,   217,   219,   223,   225,   229,
     235,   238,   244,   247,   248,   251,   253,   260,   261,   263,
     265,   268,   274,   275,   279,   280,   283,   285,   290,   292,
     295,   298,   301,   304,   309,   315,   320,   328,   330,   332,
     337,   339,   345,   346,   348,   350,   352,   356,   361,   363,
     367,   369,   371,   373,   376,   378,   383,   385,   389,   391,
     395,   397,   402,   404,   405,   407,   409,   413,   417,   418,
     420,   422,   425,   428,   430,   434,   439,   441,   445
};
static const short yyrhs[] =
{
      -1,    48,    49,     0,    48,     1,    53,     0,    53,     0,
      56,    53,     0,    91,    53,     0,    75,     0,    49,     0,
      50,    49,     0,    56,     0,    91,     0,    50,     0,    50,
      51,     0,    51,     0,     0,    42,     0,     6,     0,     7,
       0,     5,     0,     3,     0,     4,     0,    54,     0,     8,
       0,    20,     0,    54,    25,    56,     0,    57,    26,    56,
       0,    54,     0,    43,     0,    44,     0,    58,    56,    59,
       0,    55,    33,    55,     0,    55,    34,    55,     0,    33,
      55,     0,    55,    32,    55,     0,    55,    31,    55,     0,
      55,    30,    55,     0,    35,    55,     0,    55,    38,     0,
      55,    37,    55,     0,    39,     0,    40,     0,    60,    62,
      61,     0,     0,    56,     0,    62,     7,     0,    62,     7,
      56,     0,    55,     0,    56,    27,    56,     0,    55,    45,
      55,     0,    55,    45,    55,    45,    55,     0,    54,    60,
      67,    61,     0,    63,     0,    55,    60,    67,    61,     0,
      63,    25,    56,     0,    64,    26,    56,     0,    63,     0,
      66,     0,    66,    25,    56,     0,    65,    26,    56,     0,
      66,     0,    54,    60,    67,     7,    67,    61,     0,    55,
      60,    67,     7,    67,    61,     0,     0,    56,     0,    68,
       0,    55,    41,    74,     0,    69,    70,    46,     0,    47,
       0,    71,     0,    70,     7,    71,     0,     0,     5,     0,
       5,    25,    56,     0,     5,    41,     0,     5,     0,    58,
      56,    59,     0,    73,     0,    74,    41,     5,     0,    74,
      41,    58,    56,    59,     0,    72,    74,     0,    76,    47,
      52,    77,    46,     0,    22,    79,     0,     0,    78,    52,
       0,    23,     0,    81,    47,    52,    82,    83,    46,     0,
       0,    80,     0,    42,     0,    80,    42,     0,     9,    58,
      56,    59,    79,     0,     0,    82,    85,    52,     0,     0,
      86,    52,     0,    11,     0,    84,    58,    56,    59,     0,
      10,     0,    87,    55,     0,    55,    28,     0,    88,    55,
       0,    55,    29,     0,    89,    47,    52,    46,     0,    12,
      58,    56,    59,    79,     0,    90,    47,    52,    46,     0,
      16,    58,     5,    17,    56,    59,    79,     0,    13,     0,
      14,     0,    94,    47,    52,    46,     0,    15,     0,    93,
      58,    95,    59,    79,     0,     0,    97,     0,     5,     0,
      96,     0,    97,     7,    96,     0,    19,    58,    98,    59,
       0,    99,     0,    98,     7,    99,     0,     5,     0,    18,
       0,   100,     0,   100,    56,     0,    92,     0,   101,    58,
     102,    59,     0,    54,     0,    58,    56,    59,     0,    68,
       0,    55,    41,    74,     0,    92,     0,   101,    58,   102,
      59,     0,    20,     0,     0,    56,     0,   103,     0,   104,
       7,   104,     0,   103,     7,   104,     0,     0,    56,     0,
      21,     0,    21,    41,     0,   105,    73,     0,   106,     0,
     106,    41,    74,     0,    24,    58,   107,    59,     0,   108,
       0,   107,     7,   108,     0,     5,     0
};

#endif

#if YYDEBUG
/* YYRLINE[YYN] -- source line where rule number YYN was defined. */
static const short yyrline[] =
{
       0,   148,   165,   179,   188,   190,   194,   195,   199,   200,
     203,   205,   209,   210,   211,   212,   216,   218,   223,   230,
     242,   248,   254,   268,   272,   289,   299,   311,   324,   328,
     332,   334,   336,   338,   343,   346,   348,   350,   353,   356,
     364,   367,   370,   375,   377,   378,   381,   389,   390,   392,
     398,   405,   409,   416,   421,   427,   436,   446,   453,   459,
     468,   476,   479,   484,   485,   493,   496,   503,   507,   513,
     514,   517,   518,   547,   554,   561,   566,   573,   574,   580,
     588,   600,   608,   616,   620,   623,   635,   644,   645,   648,
     649,   652,   659,   660,   664,   665,   668,   674,   678,   687,
     699,   707,   719,   728,   743,   765,   777,   795,   813,   834,
     852,   868,   877,   878,   881,   897,   898,   901,   911,   912,
     915,   938,   947,   951,   956,   966,   970,   981,   983,   984,
     986,   988,   990,  1006,  1008,  1009,  1012,  1013,  1016,  1017,
    1026,  1033,  1040,  1049,  1050,  1060,  1066,  1067,  1070
};
#endif


#if (YYDEBUG) || defined YYERROR_VERBOSE

/* YYTNAME[TOKEN_NUM] -- String name of the token TOKEN_NUM. */
static const char *const yytname[] =
{
  "$", "error", "$undefined.", "CONSTANT", "STRING", "ID", "'?'", "';'", 
  "_NULL", "IF", "ELSE", "ELSEIF", "WHILE", "BREAK", "CONTINUE", 
  "FUNCTION_", "FOR", "IN", "RETURN", "LOCAL", "SELF", "SYMBOL_TABLE", 
  "TRY", "CATCH", "VEIL", "'='", "BINOP_ASSIGN", "CAT", "SHORT_OR", 
  "SHORT_AND", "OR", "AND", "RELOP", "ADDOP", "MULOP", "'!'", "UNARY", 
  "'^'", "TRANS", "'['", "']'", "'.'", "'\\n'", "'('", "')'", "':'", 
  "'}'", "'{'", "program", "statement", "statements", "u_statement", 
  "u_statements", "terminator", "id", "expr", "vexpr", "dup_id", "lparen", 
  "rparen", "lbox", "rbox", "col_list", "sub_vector", "dup_sv", "dup_sm", 
  "sub_matrix", "vexpr_or_empty", "table", "table_lb", "table_entries", 
  "table_entry", "table_dot", "member", "members", "block_statement", 
  "try_front", "catch_part", "catch", "opt_nl", "opt_nls", "if_front", 
  "elseif_part", "else_part", "elseif", "elseif_front", "else", 
  "or_front", "and_front", "while_front", "for_front", 
  "non_expr_statement", "function_def", "function", "function_start", 
  "call_list", "call_id", "call_ids", "local_ids", "local_id", "return", 
  "callee", "expr_list", "ne_expr_list", "vexpr_or_blank", "symtab_dot", 
  "Id", "veil_ids", "veil_id", 0
};
#endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives. */
static const short yyr1[] =
{
       0,    48,    48,    48,    49,    49,    49,    49,    50,    50,
      51,    51,    52,    52,    52,    52,    53,    53,    53,    54,
      55,    55,    55,    55,    55,    56,    56,    57,    58,    59,
      55,    55,    55,    55,    55,    55,    55,    55,    55,    55,
      60,    61,    55,    62,    62,    62,    62,    56,    56,    56,
      56,    63,    55,    55,    56,    56,    64,    55,    56,    56,
      65,    66,    55,    67,    67,    55,    55,    68,    69,    70,
      70,    71,    71,    71,    72,    73,    73,    74,    74,    74,
      54,    75,    76,    77,    77,    78,    75,    79,    79,    80,
      80,    81,    82,    82,    83,    83,    84,    85,    86,    55,
      87,    55,    88,    75,    89,    75,    90,    91,    91,    92,
      93,    94,    95,    95,    96,    97,    97,    91,    98,    98,
      99,   100,    91,    91,    55,    55,   101,   101,   101,   101,
     101,   101,   101,   102,   102,   102,   103,   103,   104,   104,
      55,   105,   106,    54,    54,    91,   107,   107,   108
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN. */
static const short yyr2[] =
{
       0,     0,     2,     3,     1,     2,     2,     1,     1,     2,
       1,     1,     1,     2,     1,     0,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     3,     1,     1,     1,
       3,     3,     3,     2,     3,     3,     3,     2,     2,     3,
       1,     1,     3,     0,     1,     2,     3,     1,     3,     3,
       5,     4,     1,     4,     3,     3,     1,     1,     3,     3,
       1,     6,     6,     0,     1,     1,     3,     3,     1,     1,
       3,     0,     1,     3,     2,     1,     3,     1,     3,     5,
       2,     5,     2,     0,     2,     1,     6,     0,     1,     1,
       2,     5,     0,     3,     0,     2,     1,     4,     1,     2,
       2,     2,     2,     4,     5,     4,     7,     1,     1,     4,
       1,     5,     0,     1,     1,     1,     3,     4,     1,     3,
       1,     1,     1,     2,     1,     4,     1,     3,     1,     3,
       1,     4,     1,     0,     1,     1,     3,     3,     0,     1,
       1,     2,     2,     1,     3,     4,     1,     3,     1
};

/* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
   doesn't specify something else to do.  Zero means the default is an
   error. */
static const short yydefact[] =
{
       1,     0,     0,    20,    21,    19,    17,    18,    23,     0,
       0,   107,   108,   110,     0,   121,     0,    24,   140,    87,
       0,     0,     0,    40,    16,    28,    68,     2,     4,    22,
      47,     0,     0,     0,    43,    52,     0,     0,    57,    65,
      71,     0,     7,     0,     0,     0,     0,     0,     0,     0,
     124,     0,     0,   122,     0,     0,   143,     3,    74,     0,
       0,     0,     0,   141,    89,    82,    88,     0,    22,    33,
      52,    57,    37,     0,    63,   100,   102,     0,     0,     0,
       0,     0,     0,    38,     0,     0,    63,     0,     5,     0,
       0,    44,     0,     0,     0,     0,     0,    72,     0,    69,
      75,     0,    77,    80,    15,    15,    99,   101,    15,    15,
       6,   112,    15,   123,   133,   142,     0,     0,     0,     0,
     120,     0,   118,    90,   148,     0,   146,    25,    64,     0,
      36,    35,    34,    31,    32,    39,    66,    49,     0,    48,
      26,    29,    30,    45,    41,    42,    54,    55,    59,    58,
       0,    71,    67,     0,     0,     8,    12,    14,    83,    10,
      11,    92,     0,     0,   114,     0,   115,   113,     0,   134,
       0,   135,     0,   144,    87,    87,     0,     0,   117,     0,
     145,    63,    51,     0,    63,    53,    46,    73,    70,    76,
      78,     0,     9,    13,    85,     0,    15,    94,   103,   105,
      87,     0,   109,   125,   138,   138,    91,   104,     0,   119,
     147,     0,    50,     0,     0,    81,    84,    98,    96,     0,
       0,    15,    15,   111,   116,   139,   137,   136,    87,    61,
      62,    79,    86,     0,    93,    95,   106,     0,    97,     0,
       0
};

static const short yydefgoto[] =
{
       1,   155,   156,   157,   158,    28,    29,    30,   159,    32,
      33,   142,    34,   145,    92,    35,    36,    37,    38,   129,
      39,    40,    98,    99,    41,   102,   103,    42,    43,   195,
     196,    65,    66,    44,   197,   219,   220,   221,   222,    45,
      46,    47,    48,   160,    50,    51,    52,   165,   166,   167,
     121,   122,    53,    54,   170,   171,   172,    55,    56,   125,
     126
};

static const short yypact[] =
{
  -32768,   271,    20,-32768,-32768,   -24,-32768,-32768,-32768,   -23,
     -23,-32768,-32768,-32768,   -23,-32768,   -23,   -20,     0,    34,
     -23,   354,   354,-32768,-32768,-32768,-32768,-32768,-32768,   148,
     374,    92,    10,   354,   354,     5,    55,    86,    65,    74,
      41,    14,-32768,    84,   111,   354,   354,   119,   120,    20,
      81,   -23,   129,   354,   -23,    14,   137,-32768,-32768,   354,
     354,   117,   128,-32768,-32768,-32768,   142,   174,   138,   171,
  -32768,-32768,   171,   354,   354,-32768,-32768,   354,   354,   354,
     354,   354,   354,-32768,    14,   354,   354,   354,-32768,   354,
      79,   155,     9,   354,   354,   354,   354,   163,     8,-32768,
  -32768,   354,-32768,   151,   321,   321,   347,   205,   321,   321,
  -32768,   184,   321,   155,    35,-32768,    14,    79,    79,   176,
  -32768,     1,-32768,-32768,-32768,    22,-32768,   155,   155,    11,
     228,   264,    77,   107,   171,   171,    28,   392,    21,-32768,
     155,-32768,   153,   354,-32768,-32768,   155,   155,   155,   155,
     354,    41,-32768,    79,    17,-32768,   321,-32768,   177,    92,
      20,-32768,   156,   160,-32768,   167,-32768,   192,   168,    -6,
     167,   208,   209,   151,    34,    34,   354,   128,-32768,   174,
  -32768,   354,-32768,   354,   354,-32768,   155,   155,-32768,-32768,
  -32768,   354,-32768,-32768,-32768,   178,   321,    73,-32768,-32768,
      34,   184,-32768,   180,   354,   354,-32768,-32768,    79,-32768,
  -32768,   200,   410,   200,    79,-32768,-32768,-32768,-32768,   201,
     -23,   321,   321,-32768,-32768,   155,-32768,-32768,    34,-32768,
  -32768,-32768,-32768,   354,-32768,-32768,-32768,    79,-32768,   248,
  -32768
};

static const short yypgoto[] =
{
  -32768,     3,-32768,    93,    60,     4,    58,    75,    -1,-32768,
      -7,    80,    -5,  -124,-32768,   140,-32768,-32768,   149,   -76,
  -32768,-32768,-32768,   100,-32768,   197,    -9,-32768,-32768,-32768,
  -32768,  -163,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
  -32768,-32768,-32768,   252,-32768,-32768,-32768,-32768,    53,-32768,
  -32768,    78,-32768,-32768,-32768,-32768,   -54,-32768,-32768,-32768,
      85
};


#define	YYLAST		451


static const short yytable[] =
{
      31,  -139,    59,    60,    27,   182,    57,    61,   177,    62,
     138,   206,   207,    67,   185,   151,   143,    58,   181,   100,
      25,    87,   190,  -132,    74,    86,     6,     7,   184,   179,
      93,   -56,    90,    91,   101,    88,    89,   223,     3,     4,
       5,    63,  -138,     8,   111,   141,    97,   114,   101,   144,
      13,   144,   113,   110,   152,    17,    18,    25,   117,   118,
      25,   144,    24,    74,    86,   236,   141,    86,    21,   154,
      22,  -129,   127,   128,    23,   136,    64,   101,    25,    68,
      68,    94,    26,   217,   218,   128,   139,   229,   140,   230,
      96,   -60,   146,   147,   148,   149,    69,    72,     6,     7,
     153,    86,    86,    68,    68,   211,    87,   173,   213,   101,
      80,    81,    95,   169,    82,    83,    23,  -128,    84,    87,
     106,   107,   119,   141,  -130,    86,    86,    86,    86,    86,
      86,   104,    86,   120,    24,    68,    68,    68,    68,    68,
      68,    81,   186,    68,    82,    83,    23,   191,    84,   187,
     226,   227,   130,   131,   132,   133,   134,   135,   105,   192,
     137,    70,    70,    88,   110,   161,   108,   109,   162,   163,
      71,    71,   168,    73,   -27,   208,   112,    23,   116,   124,
     128,  -126,    87,   128,   123,    70,    70,    23,   150,   164,
     214,  -126,   154,   176,    71,    71,  -127,   174,   175,   201,
     194,   178,   198,   225,   225,   180,   199,    86,    82,    83,
      23,   141,    84,   233,   202,   204,   205,    70,    70,    70,
      70,    70,    70,  -131,   215,    70,    71,    71,    71,    71,
      71,    71,   237,   189,    71,    77,    78,    79,    80,    81,
     144,    68,    82,    83,    23,   200,    84,   232,   240,   193,
     203,   188,   115,    49,   224,   209,   216,     0,   212,    78,
      79,    80,    81,     0,   210,    82,    83,    23,     0,    84,
       0,   239,     2,     0,     3,     4,     5,     6,     7,     8,
       9,   234,   235,    10,    11,    12,    13,    14,   228,    15,
      16,    17,    18,    19,   231,    20,    79,    80,    81,     0,
       0,    82,    83,    23,    21,    84,    22,     0,     0,     0,
      23,     0,     0,    24,    25,     0,     0,   238,    26,     0,
       0,     0,     0,    70,     3,     4,     5,     6,     7,     8,
       9,     0,    71,    10,    11,    12,    13,    14,     0,    15,
      16,    17,    18,    19,     0,    20,     0,     0,     0,     0,
       0,     0,     0,     0,    21,     0,    22,     3,     4,     5,
      23,     0,     8,    24,    25,     0,     0,     0,    26,    13,
       0,     0,     0,     0,    17,    18,    76,    77,    78,    79,
      80,    81,     0,     0,    82,    83,    23,    21,    84,    22,
       0,     0,     0,    23,     0,     0,     0,    25,     0,     0,
       0,    26,    75,    76,    77,    78,    79,    80,    81,     0,
       0,    82,    83,    23,     0,    84,     0,     0,     0,    85,
      75,    76,    77,    78,    79,    80,    81,     0,     0,    82,
      83,    23,     0,    84,     0,     0,     0,   183,    75,    76,
      77,    78,    79,    80,    81,     0,     0,    82,    83,    23,
       0,    84
};

static const short yycheck[] =
{
       1,     7,     9,    10,     1,   129,     2,    14,     7,    16,
      86,   174,   175,    20,   138,     7,     7,    41,     7,     5,
      43,    27,     5,    43,    29,    30,     6,     7,     7,     7,
      25,    26,    33,    34,    41,    31,    26,   200,     3,     4,
       5,    41,     7,     8,    51,    44,     5,    54,    55,    40,
      15,    40,    53,    49,    46,    20,    21,    43,    59,    60,
      43,    40,    42,    68,    69,   228,    44,    72,    33,    41,
      35,    43,    73,    74,    39,    84,    42,    84,    43,    21,
      22,    26,    47,    10,    11,    86,    87,   211,    89,   213,
      25,    26,    93,    94,    95,    96,    21,    22,     6,     7,
     101,   106,   107,    45,    46,   181,    27,   116,   184,   116,
      33,    34,    26,   114,    37,    38,    39,    43,    41,    27,
      45,    46,     5,    44,    43,   130,   131,   132,   133,   134,
     135,    47,   137,     5,    42,    77,    78,    79,    80,    81,
      82,    34,   143,    85,    37,    38,    39,   154,    41,   150,
     204,   205,    77,    78,    79,    80,    81,    82,    47,   156,
      85,    21,    22,   159,   160,   105,    47,    47,   108,   109,
      21,    22,   112,    25,    26,   176,    47,    39,    41,     5,
     181,    43,    27,   184,    42,    45,    46,    39,    25,     5,
     191,    43,    41,    17,    45,    46,    43,   117,   118,     7,
      23,   121,    46,   204,   205,   125,    46,   212,    37,    38,
      39,    44,    41,   220,    46,     7,     7,    77,    78,    79,
      80,    81,    82,    43,    46,    85,    77,    78,    79,    80,
      81,    82,   233,   153,    85,    30,    31,    32,    33,    34,
      40,   183,    37,    38,    39,   165,    41,    46,     0,   156,
     170,   151,    55,     1,   201,   177,   196,    -1,   183,    31,
      32,    33,    34,    -1,   179,    37,    38,    39,    -1,    41,
      -1,     0,     1,    -1,     3,     4,     5,     6,     7,     8,
       9,   221,   222,    12,    13,    14,    15,    16,   208,    18,
      19,    20,    21,    22,   214,    24,    32,    33,    34,    -1,
      -1,    37,    38,    39,    33,    41,    35,    -1,    -1,    -1,
      39,    -1,    -1,    42,    43,    -1,    -1,   237,    47,    -1,
      -1,    -1,    -1,   183,     3,     4,     5,     6,     7,     8,
       9,    -1,   183,    12,    13,    14,    15,    16,    -1,    18,
      19,    20,    21,    22,    -1,    24,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    33,    -1,    35,     3,     4,     5,
      39,    -1,     8,    42,    43,    -1,    -1,    -1,    47,    15,
      -1,    -1,    -1,    -1,    20,    21,    29,    30,    31,    32,
      33,    34,    -1,    -1,    37,    38,    39,    33,    41,    35,
      -1,    -1,    -1,    39,    -1,    -1,    -1,    43,    -1,    -1,
      -1,    47,    28,    29,    30,    31,    32,    33,    34,    -1,
      -1,    37,    38,    39,    -1,    41,    -1,    -1,    -1,    45,
      28,    29,    30,    31,    32,    33,    34,    -1,    -1,    37,
      38,    39,    -1,    41,    -1,    -1,    -1,    45,    28,    29,
      30,    31,    32,    33,    34,    -1,    -1,    37,    38,    39,
      -1,    41
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison/bison.simple"

/* Skeleton output parser for bison,

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software
   Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser when
   the %semantic_parser declaration is not specified in the grammar.
   It was written by Richard Stallman by simplifying the hairy parser
   used when %semantic_parser is specified.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

#if ! defined (yyoverflow) || defined (YYERROR_VERBOSE)

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || defined (YYERROR_VERBOSE) */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYLTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
# if YYLSP_NEEDED
  YYLTYPE yyls;
# endif
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAX (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# if YYLSP_NEEDED
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE) + sizeof (YYLTYPE))	\
      + 2 * YYSTACK_GAP_MAX)
# else
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAX)
# endif

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAX;	\
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif


#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");			\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).

   When YYLLOC_DEFAULT is run, CURRENT is set the location of the
   first token.  By default, to implement support for ranges, extend
   its range to the last symbol.  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)       	\
   Current.last_line   = Rhs[N].last_line;	\
   Current.last_column = Rhs[N].last_column;
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#if YYPURE
# if YYLSP_NEEDED
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, &yylloc, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval, &yylloc)
#  endif
# else /* !YYLSP_NEEDED */
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval)
#  endif
# endif /* !YYLSP_NEEDED */
#else /* !YYPURE */
# define YYLEX			yylex ()
#endif /* !YYPURE */


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)
/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
#endif /* !YYDEBUG */

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif

#ifdef YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif
#endif

#line 315 "/usr/share/bison/bison.simple"


/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
#  define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL
# else
#  define YYPARSE_PARAM_ARG YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
# endif
#else /* !YYPARSE_PARAM */
# define YYPARSE_PARAM_ARG
# define YYPARSE_PARAM_DECL
#endif /* !YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
# ifdef YYPARSE_PARAM
int yyparse (void *);
# else
int yyparse (void);
# endif
#endif

/* YY_DECL_VARIABLES -- depending whether we use a pure parser,
   variables are global, or local to YYPARSE.  */

#define YY_DECL_NON_LSP_VARIABLES			\
/* The lookahead symbol.  */				\
int yychar;						\
							\
/* The semantic value of the lookahead symbol. */	\
YYSTYPE yylval;						\
							\
/* Number of parse errors so far.  */			\
int yynerrs;

#if YYLSP_NEEDED
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES			\
						\
/* Location data for the lookahead symbol.  */	\
YYLTYPE yylloc;
#else
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES
#endif


/* If nonreentrant, generate the variables here. */

#if !YYPURE
YY_DECL_VARIABLES
#endif  /* !YYPURE */

int
yyparse (YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  /* If reentrant, generate the variables here. */
#if YYPURE
  YY_DECL_VARIABLES
#endif  /* !YYPURE */

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yychar1 = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack. */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;

#if YYLSP_NEEDED
  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
#endif

#if YYLSP_NEEDED
# define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
# define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  YYSIZE_T yystacksize = YYINITDEPTH;


  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
#if YYLSP_NEEDED
  YYLTYPE yyloc;
#endif

  /* When reducing, the number of symbols on the RHS of the reduced
     rule. */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;
#if YYLSP_NEEDED
  yylsp = yyls;
#endif
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  */
# if YYLSP_NEEDED
	YYLTYPE *yyls1 = yyls;
	/* This used to be a conditional around just the two extra args,
	   but that might be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
# else
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);
# endif
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);
# if YYLSP_NEEDED
	YYSTACK_RELOCATE (yyls);
# endif
# undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
#if YYLSP_NEEDED
      yylsp = yyls + yysize - 1;
#endif

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yychar1 = YYTRANSLATE (yychar);

#if YYDEBUG
     /* We have to keep this `#if YYDEBUG', since we use variables
	which are defined only if `YYDEBUG' is set.  */
      if (yydebug)
	{
	  YYFPRINTF (stderr, "Next token is %d (%s",
		     yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise
	     meaning of a token, for further debugging info.  */
# ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
# endif
	  YYFPRINTF (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %d (%s), ",
	      yychar, yytname[yychar1]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to the semantic value of
     the lookahead token.  This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

#if YYLSP_NEEDED
  /* Similarly for the default location.  Let the user run additional
     commands if for instance locations are ranges.  */
  yyloc = yylsp[1-yylen];
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
#endif

#if YYDEBUG
  /* We have to keep this `#if YYDEBUG', since we use variables which
     are defined only if `YYDEBUG' is set.  */
  if (yydebug)
    {
      int yyi;

      YYFPRINTF (stderr, "Reducing via rule %d (line %d), ",
		 yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (yyi = yyprhs[yyn]; yyrhs[yyi] > 0; yyi++)
	YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
      YYFPRINTF (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif

  switch (yyn) {

case 1:
#line 149 "parse.y"
{ reset_pmem() ;
	     reset_jmp(0) ;
	     if ( active_function )
	     {
	       if ( active_function->code ) FREE(active_function->code) ;
	       FREE(active_function) ;
	       active_function = (UF_BLOCK *) 0 ;
	     }
	     cleanup_scope_stack() ;
	     exec_flag = 0 ;
	     line_flag = 0;
	     brace_cnt = 0 ;
	     flow_cnt = 0 ;
	     code_index = 0 ;
	     flush_flag = 1 ;
	   }
    break;
case 2:
#line 166 "parse.y"
{ if ( interactive && exec_flag )
	     { code1(OP_HALT) ;
	       if (da_flag ) da(code_base, stderr) ;

               execute(code_base, NULL, NULL);

	       reset_pmem() ;
	       code_index = 0 ;
#if DEBUG
	       reset_jmp(1) ;
#endif
	     }
	   }
    break;
case 3:
#line 180 "parse.y"
{ 
	     raise_exception() ;
	   }
    break;
case 4:
#line 189 "parse.y"
{ yyval.start = code_index ; }
    break;
case 5:
#line 191 "parse.y"
{
	     if ( yyvsp[0].ival ) code1(OP_PRINT) ; else code1(OP_POP) ;
	   }
    break;
case 10:
#line 204 "parse.y"
{ code1(OP_PRINT) ; }
    break;
case 15:
#line 213 "parse.y"
{ yyval.start = code_index ; }
    break;
case 16:
#line 217 "parse.y"
{ yyval.ival = 1; exec_flag = 1; line_flag = 0; }
    break;
case 17:
#line 219 "parse.y"
{ yyval.ival = 1 ; /* printing on */
		if ( exec_flag = yyvsp[0].ival ) /* was followed by \n */
		  line_flag = 0;
	      }
    break;
case 18:
#line 224 "parse.y"
{ yyval.ival = 0 ; /* printing off */
		if ( exec_flag = yyvsp[0].ival ) /* was followed by \n */
		  line_flag = 0;
	      }
    break;
case 19:
#line 231 "parse.y"
{ SYMTAB *p = check_id(yyvsp[0].id_name) ;

	     yyval.twofer.start = code_index ;
	     code_line(); 
	     yyval.twofer.memcnt = 0 ;
	     yyval.twofer.op = code_index;
	     code_id_address(p) ;
	   }
    break;
case 20:
#line 243 "parse.y"
{ 
	     yyval.start = code_index ;
	     code2(OP_PUSHC, yyvsp[0].datum) ;
	   }
    break;
case 21:
#line 249 "parse.y"
{
	     yyval.start = code_index ;
	     code2(OP_PUSHS2E, yyvsp[0].cs) ;
	   }
    break;
case 22:
#line 255 "parse.y"
{ INST *p = INST_PTR( yyvsp[0].twofer.op );
	     int memcnt = (int) yyvsp[0].twofer.memcnt ;

	     yyval.start = (int) yyvsp[0].twofer.start;

	     A2I(p) ;

	     if ( memcnt )
	     {
	       code1(OP_REF_MEMB) ; code1(memcnt) ;
	     }
	   }
    break;
case 23:
#line 269 "parse.y"
{ yyval.start = code_index ;
	     code2(OP_PUSHC, &the_null) ;
	   }
    break;
case 24:
#line 273 "parse.y"
{
	     if ( active_function )
	       {
		 assert( scope_level != 0 );
		 yyval.start = code_index;
		 code1( OP_SELF );
	       }
	     else
	       {
		 yyerror( "Self reference outside function declaration." );
		 YYERROR;
	       }
	   }
    break;
case 25:
#line 290 "parse.y"
{ 
	     yyval.start = yyvsp[-2].twofer.start;

	     code_line();
	     if ( yyvsp[-2].twofer.memcnt == 0 /* simple id */ )
		  code1(OP_ASSIGN) ;
	     else icode2(OP_ASG_MEMB,yyvsp[-2].twofer.memcnt) ;
	   }
    break;
case 26:
#line 300 "parse.y"
{ 
	     yyval.start = yyvsp[-2].twofer.start;
	     code_line();
	     code1(yyvsp[-1].ival) ; /* OP_ADD, OP_SUB, etc. */

	     if ( yyvsp[-2].twofer.memcnt == 0 /* simple id */ )
		  code1(OP_ASSIGN) ;
	     else icode2(OP_ASG_MEMB,yyvsp[-2].twofer.memcnt) ;
	   }
    break;
case 27:
#line 312 "parse.y"
{ 
	     code_line();
	     code_dup_id( yyvsp[0].twofer.start, yyvsp[0].twofer.op );

	     if ( yyvsp[0].twofer.memcnt )
	     {
	       code1(OP_REF_MEMB) ;
	       code1(yyvsp[0].twofer.memcnt) ;
	     }
	   }
    break;
case 28:
#line 325 "parse.y"
{ brace_cnt++ ; }
    break;
case 29:
#line 329 "parse.y"
{ brace_cnt-- ; }
    break;
case 30:
#line 333 "parse.y"
{ yyval.start = yyvsp[-1].start ; }
    break;
case 31:
#line 335 "parse.y"
{ code_line(); code1(yyvsp[-1].ival) ; }
    break;
case 32:
#line 337 "parse.y"
{ code_line(); code1(yyvsp[-1].ival) ; }
    break;
case 33:
#line 339 "parse.y"
{ if ( yyvsp[-1].ival == OP_SUB ) { code_line(); code1(OP_UMINUS); }
	     yyval.start = yyvsp[0].start ;
	   }
    break;
case 34:
#line 344 "parse.y"
{ code_line(); code1(yyvsp[-1].ival); }
    break;
case 35:
#line 347 "parse.y"
{ code_line(); code1(OP_AND); }
    break;
case 36:
#line 349 "parse.y"
{ code_line(); code1(OP_OR); }
    break;
case 37:
#line 351 "parse.y"
{ yyval.start = yyvsp[0].start ; code_line(); code1(OP_NOT) ; }
    break;
case 38:
#line 354 "parse.y"
{ code_line(); code1(OP_TRANS) ; }
    break;
case 39:
#line 357 "parse.y"
{ code_line(); code1(OP_POWER) ; }
    break;
case 40:
#line 364 "parse.y"
{ brace_cnt++ ; }
    break;
case 41:
#line 367 "parse.y"
{ brace_cnt-- ; }
    break;
case 42:
#line 371 "parse.y"
{ yyval.start = yyvsp[-1].start ;
              code_line(); code1(OP_MATRIX) ; }
    break;
case 43:
#line 376 "parse.y"
{ code_line(); code1(OP_PUSH_NULL); code1(OP_MATRIX); }
    break;
case 45:
#line 379 "parse.y"
{ code_line(); code1(OP_PUSH_NULL);
	      code1(OP_MATRIX); code1(OP_PILE); }
    break;
case 46:
#line 382 "parse.y"
{ code_line(); code1(OP_PILE) ; }
    break;
case 48:
#line 391 "parse.y"
{ code_line(); code1(OP_APPEND) ; }
    break;
case 49:
#line 393 "parse.y"
{ DATUM *dp = new_INT(1) ;
	     code_line(); 
	     code2(OP_PUSHC, dp) ;
	     code1(OP_VSEQ) ;
	   }
    break;
case 50:
#line 399 "parse.y"
{ code_line(); code1(OP_VSEQ) ; }
    break;
case 52:
#line 410 "parse.y"
{ yyval.start = yyvsp[0].twofer.start;
	     A2I( INST_PTR( yyvsp[0].twofer.op ) ) ;
	     icode2(OP_P1D, yyvsp[0].twofer.memcnt) ;
	   }
    break;
case 53:
#line 417 "parse.y"
{ code_line(); icode2(OP_P1D,0) ; }
    break;
case 54:
#line 422 "parse.y"
{
	      yyval.start = yyvsp[-2].twofer.start;
	      code_line();
	      icode2( OP_ASSIGN_SV, yyvsp[-2].twofer.memcnt ); 
	  }
    break;
case 55:
#line 428 "parse.y"
{
	      yyval.start = yyvsp[-2].twofer.start;
	      code_line();
	      code1( yyvsp[-1].ival );	/* OP_ADD, OP_SUB, etc. */
	      icode2( OP_ASSIGN_SV, yyvsp[-2].twofer.memcnt );
	  }
    break;
case 56:
#line 437 "parse.y"
{
	      code_line();
	      code_dup_id( yyvsp[0].twofer.start, yyvsp[0].twofer.op );
	      icode2( OP_P1D, yyvsp[0].twofer.memcnt );
	  }
    break;
case 57:
#line 447 "parse.y"
{ yyval.start = yyvsp[0].twofer.start;
	     A2I( INST_PTR( yyvsp[0].twofer.op ) );
	     icode2(OP_P2D,yyvsp[0].twofer.memcnt) ;
	   }
    break;
case 58:
#line 454 "parse.y"
{
	      yyval.start = yyvsp[-2].twofer.start;
	      code_line();
	      icode2( OP_ASSIGN_SM, yyvsp[-2].twofer.memcnt );
	  }
    break;
case 59:
#line 460 "parse.y"
{
	      yyval.start = yyvsp[-2].twofer.start;
	      code_line();
	      code1( yyvsp[-1].ival );	/* OP_ADD, OP_SUB, etc. */
	      icode2( OP_ASSIGN_SM, yyvsp[-2].twofer.memcnt );
	  }
    break;
case 60:
#line 469 "parse.y"
{
	      code_line();
	      code_dup_id( yyvsp[0].twofer.start, yyvsp[0].twofer.op );
	      icode2( OP_P2D, yyvsp[0].twofer.memcnt );
	  }
    break;
case 62:
#line 480 "parse.y"
{ code_line(); icode2(OP_P2D,0) ; }
    break;
case 63:
#line 484 "parse.y"
{ code1(OP_PUSH_NULL) ; }
    break;
case 64:
#line 486 "parse.y"
{ /* this empty action shuts up bison on a vacuous 
			 warning */
		    }
    break;
case 66:
#line 497 "parse.y"
{
	     code_line();
	     icode2(OP_REF_MEMB, yyvsp[0].ival) ;
	   }
    break;
case 67:
#line 504 "parse.y"
{ brace_cnt-- ; }
    break;
case 68:
#line 508 "parse.y"
{ yyval.start = code_index ; code1(OP_MK_TABLE) ; 
	     brace_cnt++ ;
	   }
    break;
case 72:
#line 519 "parse.y"
{ /* {X} must be resolved at run time */

		   SYMTAB *p = find(yyvsp[0].id_name,NO_CREATE) ;

		   if ( p && p->scope != NO_SCOPE) 
		   { /* put value of ID on stack */
		     
		     if ( p->scope == 0 ) 
			  code2(OP_PUSHI, p->stval.datum) ;
		     else
		     if ( p->scope == 1 ) 
			      icode2(OP_LPUSHI, p->stval.offset) ;
		     else
		     {
			detour("too much scope") ;
			raise_exception() ;
		     }
		     code2(OP_PUSHS, yyvsp[0].id_name) ;
		     code1(OP_R_TABLE) ;
		   }
		   else
		   {
		     code2(OP_PUSHS, yyvsp[0].id_name) ;
		     code1(OP_ID_TABLE) ;
		   }
		 }
    break;
case 73:
#line 548 "parse.y"
{ 
		   code2(OP_PUSHS, yyvsp[-2].id_name) ;
		   code1(OP_R_TABLE) ;
		 }
    break;
case 74:
#line 555 "parse.y"
{ SYMTAB *p = check_id(yyvsp[-1].id_name) ;
	       yyval.start = code_index ;
	       code_id_address(p) ;
	     }
    break;
case 75:
#line 562 "parse.y"
{ yyval.ival = 1 ;
	       code_line();
               code2(OP_PUSHS, yyvsp[0].id_name) ;
	     }
    break;
case 76:
#line 567 "parse.y"
{ yyval.ival = 1 ;
	       code_line();
	       code1(OP_E2STR) ;
	     }
    break;
case 78:
#line 575 "parse.y"
{
		 yyval.ival = yyvsp[-2].ival + 1;
		 code_line();
		 code2(OP_PUSHS, yyvsp[0].id_name);
	     }
    break;
case 79:
#line 581 "parse.y"
{
		 yyval.ival = yyvsp[-4].ival + 1;
		 code_line();
		 code1(OP_E2STR);
	     }
    break;
case 80:
#line 589 "parse.y"
{
	     yyval.twofer.start = yyval.twofer.op = yyvsp[-1].start;
	     yyval.twofer.memcnt = yyvsp[0].ival;
	   }
    break;
case 81:
#line 601 "parse.y"
{ code_line();
		      patch_jmp(code_index) ;
		      flow_cnt-- ;
		      pop_try ();
		    }
    break;
case 82:
#line 609 "parse.y"
{ yyval.start = code_index ;
	       code_jmp(OP_TRY, -1) ; 
	       flow_cnt++ ;
	       push_try ();
	     }
    break;
case 83:
#line 617 "parse.y"
{
	     code_jmp(OP_CATCH, code_index+2);
	   }
    break;
case 85:
#line 624 "parse.y"
{
	     patch_jmp(code_index+2);
	     code_jmp(OP_CATCH, -1);
	   }
    break;
case 86:
#line 636 "parse.y"
{ int i = yyvsp[-2].ival+1 ;

		  while(i--) patch_jmp(code_index) ;

		  flow_cnt-- ;
		}
    break;
case 91:
#line 653 "parse.y"
{ yyval.start = yyvsp[-2].start ;  code_jmp(OP_JZ, -1) ; 
	       flow_cnt++ ;
	     }
    break;
case 92:
#line 659 "parse.y"
{ yyval.ival = 0 ; }
    break;
case 93:
#line 661 "parse.y"
{ yyval.ival = yyvsp[-2].ival + 1 ; }
    break;
case 96:
#line 669 "parse.y"
{ patch_jmp(code_index+2) ;
	     code_jmp(OP_JMP, -1) ;
	   }
    break;
case 97:
#line 675 "parse.y"
{ code_jmp(OP_JZ, -1) ; }
    break;
case 98:
#line 679 "parse.y"
{ patch_jmp(code_index+2) ;
	     code_jmp(OP_JMP, -1) ;
	   }
    break;
case 99:
#line 688 "parse.y"
{ code_line();
	   patch_jmp(code_index+2) ;
	   code_jmp(OP_JZ, -1) ;
	   code2(OP_PUSHC, new_INT(1));
	   patch_jmp(code_index+2) ;
	   code_jmp(OP_JMP, -1) ;
	   code2(OP_PUSHC, new_INT(0));
	   patch_jmp(code_index) ;
	 }
    break;
case 100:
#line 700 "parse.y"
{ code_line();
	       code_jmp(OP_JNZ, -1) ; 
	     }
    break;
case 101:
#line 708 "parse.y"
{ code_line();
	   patch_jmp(code_index+2) ;
	   code_jmp(OP_JNZ, -1) ;
	   code2(OP_PUSHC, new_INT(0));
	   patch_jmp(code_index+2) ;
	   code_jmp(OP_JMP, -1) ;
	   code2(OP_PUSHC, new_INT(1));
	   patch_jmp(code_index) ;
	 }
    break;
case 102:
#line 720 "parse.y"
{ code_line();
	       code_jmp(OP_JZ, -1) ; 
	     }
    break;
case 103:
#line 729 "parse.y"
{ code_jmp(OP_JMP, yyvsp[-3].start) ;
		     
		     if ( yyvsp[-3].start != yyvsp[-1].start ) /* real test */
		     { /* patch the jump */
		       INST_PTR(yyvsp[-1].start-1)->op = code_index - yyvsp[-1].start + 1 ;
		     }

		     BC_clear(code_index, yyvsp[-3].start) ;

		     flow_cnt-- ;
		     pop_loop ();
		   }
    break;
case 104:
#line 744 "parse.y"
{ yyval.start = yyvsp[-2].start ; 
		     BC_new() ; /*start new break/cont list */

		     /* This jump is special, we look for a
			constant.  So don't use code_jmp() */
		     if ( code_index - 2 == yyvsp[-2].start &&
			  INST_PTR(yyvsp[-2].start)->op == OP_PUSHC &&
			  datum_test(INST_PTR(yyvsp[-2].start+1)->ptr)
			)
			code_index -= 2 ; /* remove test */
		     else  code2(OP_JZ, 0) ;

		     flow_cnt++ ;
		     push_loop ();
		   }
    break;
case 105:
#line 766 "parse.y"
{ 
	      BC_clear(code_index+2, code_index) ;
	      INST_PTR(yyvsp[-1].start-1)->op = code_index - yyvsp[-1].start + 1 ;
	      code_jmp(OP_FL, yyvsp[-1].start) ;
	      code1(OP_KFL) ;

	      flow_cnt-- ;
	      pop_loop ();
	    }
    break;
case 106:
#line 778 "parse.y"
{ SYMTAB *p = check_id(yyvsp[-4].id_name) ;
	   
	     yyval.start = yyvsp[-2].start ;
	     code_id_address(p);

	     code1(OP_IFL) ; 
	     code_index++ ; /* slot for jmp to FL */
	     BC_new() ;

	     flow_cnt++ ;
	     push_loop ();
	   }
    break;
case 107:
#line 796 "parse.y"
{ 
		     yyval.start = code_index ;
		     code1(OP_JMP) ;
		     if ( BC_insert('B', code_index) == 0 )
		     {
		       yyerror("break statement outside loop") ;
		       YYERROR ;
		     }
		     if (in_try ())
		       {
			 yyerror ("break statement inside try block");
			 YYERROR;
		       }
		     code1(0) ;
		   }
    break;
case 108:
#line 814 "parse.y"
{ 
		     yyval.start = code_index ;
		     code1(OP_JMP) ;
		     if ( BC_insert('C', code_index) == 0 )
		     {
		       yyerror("continue statement outside loop") ;
		       YYERROR ;
		     }
		     if (in_try ())
		       {
			 yyerror ("continue statement inside try block");
			 YYERROR;
		       }
		     code1(0) ;
		   }
    break;
case 109:
#line 835 "parse.y"
{ 
		  code1(OP_HALT) ;
		  shrink_code_block() ;
		  active_function->code = code_base ;

		  active_function->my_datum->type = D_ENTITY ;
		  active_function->my_datum->data.ptr =
		      make_function(user, active_function) ;
			 
		  yyval.datum = active_function->my_datum ;
		  active_function = pop_scope() ;

		  flow_cnt-- ;
		}
    break;
case 110:
#line 853 "parse.y"
{
		     if ( active_function ) {
			 detour( "Too much scope." );
			 raise_exception();
		     }
		     push_scope(active_function) ;
		     active_function = CALLOC(1,sizeof(UF_BLOCK)) ;

		     active_function->my_datum =
			MALLOC(sizeof(DATUM)) ;

		     flow_cnt++ ;
		   }
    break;
case 111:
#line 869 "parse.y"
{
		     active_function->targs =
			   active_function->cargs ;
		     code2( OP_FILE, curr_file );
		     line_flag = 0; code_line();
		   }
    break;
case 114:
#line 882 "parse.y"
{ SYMTAB *p = find(yyvsp[0].id_name, CREATE) ;
	   
	     if ( p->scope == scope_level )
	     {
	       yyerror("duplicated function argument") ;
	       YYERROR;
	     }

	     save_id(p) ;
	     p->scope = scope_level ;
	     p->stval.offset = 
		     active_function->cargs ++ ;
	   }
    break;
case 117:
#line 902 "parse.y"
{ if ( scope_level == 0 )
	       {
		 yyerror("local declaration at global scope") ;
		 YYERROR;
	       }
	       yyval.start = code_index ;
	     }
    break;
case 120:
#line 916 "parse.y"
{ 
	     if ( scope_level > 0 )
	     {
	       SYMTAB *p = find(yyvsp[0].id_name, CREATE) ;
	     
		 if ( p->scope == scope_level )
		 {
		   warn("duplicated local declaration (ignored)") ;
		 }
		 else
		 {
		   save_id(p) ;
		   p->scope = scope_level ;
		   p->stval.offset = 
			 active_function->targs ++ ;
		 }
	     }
	     
	   }
    break;
case 121:
#line 939 "parse.y"
{ if ( scope_level == 0 )
	     {
	       yyerror("return outside function body") ;
	       YYERROR;
	     }
	   }
    break;
case 122:
#line 948 "parse.y"
{ yyval.start = code_index ; code1(OP_PUSH_NULL) ;
	      code1(OP_RET) ;
	    }
    break;
case 123:
#line 952 "parse.y"
{ yyval.start = yyvsp[0].start ; code1(OP_RET) ; }
    break;
case 124:
#line 957 "parse.y"
{
	       yyval.start = code_index;
	       code2( OP_PUSHC, yyvsp[0].datum );
	   }
    break;
case 125:
#line 967 "parse.y"
{ code_line(); code1(OP_CALL) ; code1(yyvsp[-1].ival) ; }
    break;
case 126:
#line 971 "parse.y"
{ 
	     yyval.start = yyvsp[0].twofer.start;
	     A2I( INST_PTR( yyvsp[0].twofer.op ) );

	     if ( yyvsp[0].twofer.memcnt )
	     {
	       code1(OP_REF_MEMB) ;
	       code1(yyvsp[0].twofer.memcnt) ;
	     }
	   }
    break;
case 127:
#line 982 "parse.y"
{ yyval.start = yyvsp[-1].start; }
    break;
case 129:
#line 985 "parse.y"
{ code_line(); icode2( OP_REF_MEMB, yyvsp[0].ival ); }
    break;
case 130:
#line 987 "parse.y"
{ yyval.start = code_index; code2( OP_PUSHC, yyvsp[0].datum ); }
    break;
case 131:
#line 989 "parse.y"
{ code_line(); code1(OP_CALL) ; code1(yyvsp[-1].ival) ; }
    break;
case 132:
#line 991 "parse.y"
{
	     if ( active_function )
	       {
		 assert( scope_level != 0 );
		 yyval.start = code_index;
		 code1( OP_SELF );
	       }
	     else
	       {
		 yyerror( "Self reference outside function declaration." );
		 YYERROR;
	       }
	   }
    break;
case 133:
#line 1007 "parse.y"
{ yyval.ival = 0 ; }
    break;
case 134:
#line 1008 "parse.y"
{yyval.ival=1;}
    break;
case 136:
#line 1012 "parse.y"
{yyval.ival=2;}
    break;
case 137:
#line 1013 "parse.y"
{yyval.ival=yyvsp[-2].ival+1;}
    break;
case 138:
#line 1016 "parse.y"
{ code2( OP_PUSHC, &the_null ); }
    break;
case 139:
#line 1018 "parse.y"
{ /* this empty action shuts up bison on a vacuous 
			 warning */
		    }
    break;
case 140:
#line 1027 "parse.y"
{
	     yyval.start = code_index;
	     code1(OP_SYMTAB);
	  }
    break;
case 141:
#line 1034 "parse.y"
{
		 yyval.twofer.start = code_index;
		 code_line();
	     }
    break;
case 142:
#line 1041 "parse.y"
{
	      yyval.twofer.start = yyvsp[-1].twofer.start;
	      yyval.twofer.op = code_index;
	      code1( OP_PUSHA_SYM );
	      yyval.twofer.memcnt = 0;
	  }
    break;
case 144:
#line 1051 "parse.y"
{
	     yyval.twofer.start = yyvsp[-2].twofer.start;
	     yyval.twofer.op = yyvsp[-2].twofer.op;
	     yyval.twofer.memcnt = yyvsp[0].ival;
	 }
    break;
case 145:
#line 1061 "parse.y"
{
		  yyval.start = code_index ;
		}
    break;
case 148:
#line 1071 "parse.y"
{
	  SYMTAB *p = check_id (yyvsp[0].id_name);
	  if (p->scope)
	    {
	      yyerror ("cannot veil a local variable");
	      YYERROR;
	    }
	  code_veil_address (p);
	}
    break;
}

#line 705 "/usr/share/bison/bison.simple"


  yyvsp -= yylen;
  yyssp -= yylen;
#if YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;
#if YYLSP_NEEDED
  *++yylsp = yyloc;
#endif

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("parse error, unexpected ") + 1;
	  yysize += yystrlen (yytname[YYTRANSLATE (yychar)]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "parse error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[YYTRANSLATE (yychar)]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exhausted");
	}
      else
#endif /* defined (YYERROR_VERBOSE) */
	yyerror ("parse error");
    }
  goto yyerrlab1;


/*--------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action |
`--------------------------------------------------*/
yyerrlab1:
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;
      YYDPRINTF ((stderr, "Discarding token %d (%s).\n",
		  yychar, yytname[yychar1]));
      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;


/*-------------------------------------------------------------------.
| yyerrdefault -- current state does not do anything special for the |
| error token.                                                       |
`-------------------------------------------------------------------*/
yyerrdefault:
#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */

  /* If its default is to accept any token, ok.  Otherwise pop it.  */
  yyn = yydefact[yystate];
  if (yyn)
    goto yydefault;
#endif


/*---------------------------------------------------------------.
| yyerrpop -- pop the current state because it cannot handle the |
| error token                                                    |
`---------------------------------------------------------------*/
yyerrpop:
  if (yyssp == yyss)
    YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#if YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "Error: state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

/*--------------.
| yyerrhandle.  |
`--------------*/
yyerrhandle:
  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

/*---------------------------------------------.
| yyoverflowab -- parser overflow comes here.  |
`---------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}
#line 1082 "parse.y"



/* Have we seen this ID before */
static SYMTAB *
check_id (char *id)
{
  SYMTAB *stp = find(id,CREATE) ;

  if ( stp->scope == NO_SCOPE )
  {
    stp->scope = 0 ;
    stp->stval.datum = new_DATUM() ;
    /* new_DATUM() is init to D_NULL */
  }

  return stp ;
}
