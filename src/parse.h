#ifndef BISON_Y_TAB_H
# define BISON_Y_TAB_H

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


extern YYSTYPE yylval;

#endif /* not BISON_Y_TAB_H */
