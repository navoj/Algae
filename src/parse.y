/*
   parse.y -- yacc parser file

   Copyright (C) 1994-2002  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
*/

%{
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

%}

%union {
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
}

%expect 6

%token   <datum> CONSTANT   /* integer or decimal */
%token   <cs>    STRING
%token   <id_name>   ID 
%token   <ival>  '?'  ';'


/* keyword tokens */
%token  _NULL 
%token  IF ELSE ELSEIF
%token  WHILE BREAK CONTINUE
%token  FUNCTION_
%token  FOR IN
%token  RETURN LOCAL SELF
%token	SYMBOL_TABLE
%token	TRY CATCH
%token	VEIL

/* operators */
%right <ival>  '='  BINOP_ASSIGN
%left  CAT
%left   SHORT_OR
%left   SHORT_AND
%left   OR
%left   AND
%left <ival>   RELOP
%left   <ival> ADDOP
%left   <ival> MULOP
%left   <ival> '!'  UNARY  /* -x and +x */
%right '^'
%nonassoc  TRANS
%left  '[' ']'
%right '.'

%type  <start>  expr  vexpr
%type  <start>  col_list  
%type  <ival>   expr_list  ne_expr_list
%type  <start>  statement statements u_statements u_statement
%type  <start>  non_expr_statement
%type  <start>  block_statement
%type  <start>  if_front
%type  <start>  try_front
%type  <start>  or_front
%type  <ival>   elseif_part
%type  <start>  while_front
%type  <start>  for_front
%type  <start>  callee
%type  <ival>  terminator  

%type  <start>  table  table_lb  table_dot
%type  <ival>   member members
%type  <twofer> id dup_id Id symtab_dot dup_sv dup_sm
%type  <twofer>  sub_vector sub_matrix  

%type  <datum>    function_def

%%


program :  /* empty */  
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
	|  program  statement
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
	|  program  error terminator
	   { 
	     raise_exception() ;
	   }
	;




statement :  /* empty */ terminator
	   { $$ = code_index ; }
	|  vexpr terminator
	   {
	     if ( $2 ) code1(OP_PRINT) ; else code1(OP_POP) ;
	   }
	|  non_expr_statement terminator
	|  block_statement
	;


statements :  statement
	   |  statements  statement
	   ;

u_statement : vexpr   /* unterminated statement */
            { code1(OP_PRINT) ; }
	 |  non_expr_statement
	 ;

/* u_statements may or may not be terminated */
u_statements : statements 
	     | statements u_statement
	     | u_statement
	     | /*empty*/
	       { $$ = code_index ; }
	     ;

terminator :  '\n'
              { $$ = 1; exec_flag = 1; line_flag = 0; }
	   |  '?'
	      { $$ = 1 ; /* printing on */
		if ( exec_flag = $1 ) /* was followed by \n */
		  line_flag = 0;
	      }
	   |  ';'
	      { $$ = 0 ; /* printing off */
		if ( exec_flag = $1 ) /* was followed by \n */
		  line_flag = 0;
	      }
	   ;

id	:  ID
	   { SYMTAB *p = check_id($1) ;

	     $$.start = code_index ;
	     code_line(); 
	     $$.memcnt = 0 ;
	     $$.op = code_index;
	     code_id_address(p) ;
	   }
	;


expr	:  CONSTANT
	   { 
	     $$ = code_index ;
	     code2(OP_PUSHC, $1) ;
	   }

	|  STRING
	   {
	     $$ = code_index ;
	     code2(OP_PUSHS2E, $1) ;
	   }

	|  id
	   { INST *p = INST_PTR( $1.op );
	     int memcnt = (int) $1.memcnt ;

	     $$ = (int) $1.start;

	     A2I(p) ;

	     if ( memcnt )
	     {
	       code1(OP_REF_MEMB) ; code1(memcnt) ;
	     }
	   }

	|  _NULL
	   { $$ = code_index ;
	     code2(OP_PUSHC, &the_null) ;
	   }
	|  SELF
	   {
	     if ( active_function )
	       {
		 assert( scope_level != 0 );
		 $$ = code_index;
		 code1( OP_SELF );
	       }
	     else
	       {
		 yyerror( "Self reference outside function declaration." );
		 YYERROR;
	       }
	   }
	;


vexpr	:  id  '=' vexpr
	   { 
	     $$ = $1.start;

	     code_line();
	     if ( $1.memcnt == 0 /* simple id */ )
		  code1(OP_ASSIGN) ;
	     else icode2(OP_ASG_MEMB,$1.memcnt) ;
	   }

	|  dup_id  BINOP_ASSIGN vexpr
	   { 
	     $$ = $1.start;
	     code_line();
	     code1($2) ; /* OP_ADD, OP_SUB, etc. */

	     if ( $1.memcnt == 0 /* simple id */ )
		  code1(OP_ASSIGN) ;
	     else icode2(OP_ASG_MEMB,$1.memcnt) ;
	   }
	;

dup_id  :  id
	   { 
	     code_line();
	     code_dup_id( $1.start, $1.op );

	     if ( $1.memcnt )
	     {
	       code1(OP_REF_MEMB) ;
	       code1($1.memcnt) ;
	     }
	   }
	;

lparen  :  '('
	    { brace_cnt++ ; }
	;

rparen  :  ')'
	    { brace_cnt-- ; }
	;

expr	:  lparen  vexpr rparen
	   { $$ = $2 ; }
	|  expr ADDOP expr
	   { code_line(); code1($2) ; }
	|  expr MULOP expr
	   { code_line(); code1($2) ; }
	|  ADDOP  expr    %prec  UNARY
	   { if ( $1 == OP_SUB ) { code_line(); code1(OP_UMINUS); }
	     $$ = $2 ;
	   }

	|  expr  RELOP  expr
	   { code_line(); code1($2); }

	|  expr  AND  expr
	   { code_line(); code1(OP_AND); }
	|  expr  OR  expr
	   { code_line(); code1(OP_OR); }
	|  '!'  expr 
	   { $$ = $2 ; code_line(); code1(OP_NOT) ; }

	|  expr  TRANS
	   { code_line(); code1(OP_TRANS) ; }

        |  expr '^' expr
           { code_line(); code1(OP_POWER) ; }

	;


/*----------build matrices----------------------------*/

lbox	:  '['  { brace_cnt++ ; }
	;

rbox	:  ']'  { brace_cnt-- ; }
	;

expr	:   lbox col_list rbox
	    { $$ = $2 ;
              code_line(); code1(OP_MATRIX) ; }
	;

col_list : /* empty */
            { code_line(); code1(OP_PUSH_NULL); code1(OP_MATRIX); }
         |  vexpr
         |  col_list ';'
            { code_line(); code1(OP_PUSH_NULL);
	      code1(OP_MATRIX); code1(OP_PILE); }
	 |  col_list ';' vexpr
	    { code_line(); code1(OP_PILE) ; }
	 ;


/*------------build vectors------------------------*/


vexpr	:  expr
	|  vexpr CAT vexpr
	   { code_line(); code1(OP_APPEND) ; }
	|  expr ':' expr 
	   { DATUM *dp = new_INT(1) ;
	     code_line(); 
	     code2(OP_PUSHC, dp) ;
	     code1(OP_VSEQ) ;
	   }
	|  expr ':' expr ':' expr
	   { code_line(); code1(OP_VSEQ) ; }
	;


/*----------- sub vector ---------------*/

sub_vector :  id lbox vexpr_or_empty rbox
	   ;


expr	:  sub_vector
	   { $$ = $1.start;
	     A2I( INST_PTR( $1.op ) ) ;
	     icode2(OP_P1D, $1.memcnt) ;
	   }
	;

expr    :  expr lbox vexpr_or_empty rbox
	   { code_line(); icode2(OP_P1D,0) ; }
	;


vexpr	: sub_vector '=' vexpr
	  {
	      $$ = $1.start;
	      code_line();
	      icode2( OP_ASSIGN_SV, $1.memcnt ); 
	  }
	| dup_sv BINOP_ASSIGN vexpr
	  {
	      $$ = $1.start;
	      code_line();
	      code1( $2 );	/* OP_ADD, OP_SUB, etc. */
	      icode2( OP_ASSIGN_SV, $1.memcnt );
	  }
	;

dup_sv	: sub_vector
	  {
	      code_line();
	      code_dup_id( $1.start, $1.op );
	      icode2( OP_P1D, $1.memcnt );
	  }
	;

/*------------sub matrix------------------------*/

expr  :    sub_matrix
	   { $$ = $1.start;
	     A2I( INST_PTR( $1.op ) );
	     icode2(OP_P2D,$1.memcnt) ;
	   }
      ;

vexpr	: sub_matrix '=' vexpr
	  {
	      $$ = $1.start;
	      code_line();
	      icode2( OP_ASSIGN_SM, $1.memcnt );
	  }
	| dup_sm BINOP_ASSIGN vexpr
	  {
	      $$ = $1.start;
	      code_line();
	      code1( $2 );	/* OP_ADD, OP_SUB, etc. */
	      icode2( OP_ASSIGN_SM, $1.memcnt );
	  }
	;

dup_sm	: sub_matrix
	  {
	      code_line();
	      code_dup_id( $1.start, $1.op );
	      icode2( OP_P2D, $1.memcnt );
	  }
	;

sub_matrix    :  id lbox  vexpr_or_empty ';' vexpr_or_empty rbox
	      ;

expr	:  expr lbox vexpr_or_empty ';' vexpr_or_empty rbox
	   { code_line(); icode2(OP_P2D,0) ; }
	;


vexpr_or_empty : /* empty */  { code1(OP_PUSH_NULL) ; }
		|   vexpr 
		    { /* this empty action shuts up bison on a vacuous 
			 warning */
		    }
		;

/*----------- tables ---------------*/

expr	:  table
	;

expr	:  expr  '.'  members   /* table reference */
	   {
	     code_line();
	     icode2(OP_REF_MEMB, $3) ;
	   }
	;

table 	: table_lb  table_entries  '}'
	  { brace_cnt-- ; }
	;

table_lb :  '{' 
	   { $$ = code_index ; code1(OP_MK_TABLE) ; 
	     brace_cnt++ ;
	   }
	 ;

table_entries :  table_entry
	      |  table_entries ';' table_entry
	      ;

table_entry   :  /* empty */
              |  ID 
		 { /* {X} must be resolved at run time */

		   SYMTAB *p = find($1,NO_CREATE) ;

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
		     code2(OP_PUSHS, $1) ;
		     code1(OP_R_TABLE) ;
		   }
		   else
		   {
		     code2(OP_PUSHS, $1) ;
		     code1(OP_ID_TABLE) ;
		   }
		 }

		    
	      |  ID '=' vexpr
		 { 
		   code2(OP_PUSHS, $1) ;
		   code1(OP_R_TABLE) ;
		 }
	      ;

table_dot :  ID '.'
	     { SYMTAB *p = check_id($1) ;
	       $$ = code_index ;
	       code_id_address(p) ;
	     }
	  ;

member   :   ID  /*  follows table_dot */
	     { $$ = 1 ;
	       code_line();
               code2(OP_PUSHS, $1) ;
	     }
	 |   lparen  vexpr  rparen  /* e.g. x.("string") */
	     { $$ = 1 ;
	       code_line();
	       code1(OP_E2STR) ;
	     }
	 ;

members  :   member
	 |   members '.' ID
	     {
		 $$ = $1 + 1;
		 code_line();
		 code2(OP_PUSHS, $3);
	     }
	 |   members '.' lparen vexpr rparen
	     {
		 $$ = $1 + 1;
		 code_line();
		 code1(OP_E2STR);
	     }
	 ;

id	:  table_dot  members
	   {
	     $$.start = $$.op = $1;
	     $$.memcnt = $2;
	   }

	;

/*---------- try statement ----------------------*/


/*try_statement*/
block_statement  :  try_front '{' u_statements catch_part '}'
		    { code_line();
		      patch_jmp(code_index) ;
		      flow_cnt-- ;
		      pop_try ();
		    }
	;

try_front  :  TRY opt_nl 
	     { $$ = code_index ;
	       code_jmp(OP_TRY, -1) ; 
	       flow_cnt++ ;
	       push_try ();
	     }
	;

catch_part  :  /* empty */
	   {
	     code_jmp(OP_CATCH, code_index+2);
	   }
	   |   catch  u_statements
	   ;

catch	:  CATCH
	   {
	     patch_jmp(code_index+2);
	     code_jmp(OP_CATCH, -1);
	   }
	;


/*---------- if statement ----------------------*/


/*if_statement*/
block_statement  :  if_front '{' u_statements elseif_part else_part '}'
		{ int i = $4+1 ;

		  while(i--) patch_jmp(code_index) ;

		  flow_cnt-- ;
		}
	;

opt_nl   :  /* empty */
	 |  opt_nls
	 ;

opt_nls  :  '\n'
	 |  opt_nls '\n'
	 ;

if_front  :  IF  lparen vexpr rparen  opt_nl 
	     { $$ = $3 ;  code_jmp(OP_JZ, -1) ; 
	       flow_cnt++ ;
	     }
	;

/* counts the number of elseif terms */
elseif_part  :  /* empty */  { $$ = 0 ; }
	     |  elseif_part  elseif_front u_statements
		{ $$ = $1 + 1 ; }
	     ;

else_part  :  /* empty */
	   |  else  u_statements
	   ;

elseif	:  ELSEIF
	   { patch_jmp(code_index+2) ;
	     code_jmp(OP_JMP, -1) ;
	   }
	;

elseif_front :  elseif lparen vexpr rparen
		{ code_jmp(OP_JZ, -1) ; }
             ;

else	:  ELSE
	   { patch_jmp(code_index+2) ;
	     code_jmp(OP_JMP, -1) ;
	   }
	;


/*---------- the short or ----------------------*/

expr  :  or_front expr %prec SHORT_OR
         { code_line();
	   patch_jmp(code_index+2) ;
	   code_jmp(OP_JZ, -1) ;
	   code2(OP_PUSHC, new_INT(1));
	   patch_jmp(code_index+2) ;
	   code_jmp(OP_JMP, -1) ;
	   code2(OP_PUSHC, new_INT(0));
	   patch_jmp(code_index) ;
	 }
      ;

or_front  :  expr SHORT_OR
             { code_line();
	       code_jmp(OP_JNZ, -1) ; 
	     }
	  ;

/*---------- the short and ----------------------*/

expr  :  and_front expr %prec SHORT_AND
         { code_line();
	   patch_jmp(code_index+2) ;
	   code_jmp(OP_JNZ, -1) ;
	   code2(OP_PUSHC, new_INT(0));
	   patch_jmp(code_index+2) ;
	   code_jmp(OP_JMP, -1) ;
	   code2(OP_PUSHC, new_INT(1));
	   patch_jmp(code_index) ;
	 }
      ;

and_front  :  expr SHORT_AND
             { code_line();
	       code_jmp(OP_JZ, -1) ; 
	     }
	   ;

/* ---------  while  loop ---------------------*/

/*while_statement*/
block_statement    :  while_front '{' u_statements  '}'
		   { code_jmp(OP_JMP, $1) ;
		     
		     if ( $1 != $3 ) /* real test */
		     { /* patch the jump */
		       INST_PTR($3-1)->op = code_index - $3 + 1 ;
		     }

		     BC_clear(code_index, $1) ;

		     flow_cnt-- ;
		     pop_loop ();
		   }
		;
	       
while_front	:  WHILE lparen vexpr rparen opt_nl 
		   { $$ = $3 ; 
		     BC_new() ; /*start new break/cont list */

		     /* This jump is special, we look for a
			constant.  So don't use code_jmp() */
		     if ( code_index - 2 == $3 &&
			  INST_PTR($3)->op == OP_PUSHC &&
			  datum_test(INST_PTR($3+1)->ptr)
			)
			code_index -= 2 ; /* remove test */
		     else  code2(OP_JZ, 0) ;

		     flow_cnt++ ;
		     push_loop ();
		   }
		;

	
/*---------  for loop -------------------------*/

/*for_statement*/
block_statement :  for_front '{' u_statements '}'
	    { 
	      BC_clear(code_index+2, code_index) ;
	      INST_PTR($3-1)->op = code_index - $3 + 1 ;
	      code_jmp(OP_FL, $3) ;
	      code1(OP_KFL) ;

	      flow_cnt-- ;
	      pop_loop ();
	    }
	;

for_front :  FOR lparen ID  IN  vexpr rparen opt_nl 
	   { SYMTAB *p = check_id($3) ;
	   
	     $$ = $5 ;
	     code_id_address(p);

	     code1(OP_IFL) ; 
	     code_index++ ; /* slot for jmp to FL */
	     BC_new() ;

	     flow_cnt++ ;
	     push_loop ();
	   }
	;


/*-------------------------------------------------*/

non_expr_statement	:  BREAK
		   { 
		     $$ = code_index ;
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
		;

non_expr_statement  :  CONTINUE
		   { 
		     $$ = code_index ;
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
		;

/*------------ user functions -------------------------*/


function_def :  function_start '{' u_statements '}'
		{ 
		  code1(OP_HALT) ;
		  shrink_code_block() ;
		  active_function->code = code_base ;

		  active_function->my_datum->type = D_ENTITY ;
		  active_function->my_datum->data.ptr =
		      make_function(user, active_function) ;
			 
		  $$ = active_function->my_datum ;
		  active_function = pop_scope() ;

		  flow_cnt-- ;
		}
	     ;


function	:  FUNCTION_
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
		;

function_start  :  function lparen call_list rparen opt_nl 
		   {
		     active_function->targs =
			   active_function->cargs ;
		     code2( OP_FILE, curr_file );
		     line_flag = 0; code_line();
		   }
		;

call_list : /* empty */
	|  call_ids
	;

call_id  :  ID
	   { SYMTAB *p = find($1, CREATE) ;
	   
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
	;  

call_ids  :  call_id 
	  |  call_ids ';' call_id
	  ;

non_expr_statement :  LOCAL lparen local_ids rparen 
	     { if ( scope_level == 0 )
	       {
		 yyerror("local declaration at global scope") ;
		 YYERROR;
	       }
	       $$ = code_index ;
	     }
	  ;

local_ids :  local_id
	  |  local_ids ';' local_id
	  ;

local_id  :  ID
	   { 
	     if ( scope_level > 0 )
	     {
	       SYMTAB *p = find($1, CREATE) ;
	     
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
	;  


return  :  RETURN
	   { if ( scope_level == 0 )
	     {
	       yyerror("return outside function body") ;
	       YYERROR;
	     }
	   }
	;

non_expr_statement : return   
	    { $$ = code_index ; code1(OP_PUSH_NULL) ;
	      code1(OP_RET) ;
	    }
	  | return   vexpr  
	    { $$ = $2 ; code1(OP_RET) ; }
	  ;
	     

expr	:  function_def
           {
	       $$ = code_index;
	       code2( OP_PUSHC, $1 );
	   }
        ;

/*------------ function call --- user defined or builtin */

/* a function call */
expr :  callee  lparen expr_list rparen
		 { code_line(); code1(OP_CALL) ; code1($3) ; }
	      ;

callee	:  id
	   { 
	     $$ = $1.start;
	     A2I( INST_PTR( $1.op ) );

	     if ( $1.memcnt )
	     {
	       code1(OP_REF_MEMB) ;
	       code1($1.memcnt) ;
	     }
	   }
        |  lparen vexpr rparen
           { $$ = $2; }
        |  table
        |  expr '.' members
           { code_line(); icode2( OP_REF_MEMB, $3 ); }
        |  function_def
           { $$ = code_index; code2( OP_PUSHC, $1 ); }
        |  callee lparen expr_list rparen
           { code_line(); code1(OP_CALL) ; code1($3) ; }
	|  SELF
	   {
	     if ( active_function )
	       {
		 assert( scope_level != 0 );
		 $$ = code_index;
		 code1( OP_SELF );
	       }
	     else
	       {
		 yyerror( "Self reference outside function declaration." );
		 YYERROR;
	       }
	   }
	;

expr_list : /* empty */
	    { $$ = 0 ; }
          | vexpr {$$=1;}
	  | ne_expr_list 
	  ;

ne_expr_list : vexpr_or_blank ';' vexpr_or_blank {$$=2;}
	     | ne_expr_list ';' vexpr_or_blank {$$=$1+1;}
	     ;

vexpr_or_blank : /* empty */  { code2( OP_PUSHC, &the_null ); }
		|   vexpr
		    { /* this empty action shuts up bison on a vacuous 
			 warning */
		    }
		;

/* ---------- the symbol table ----------- */


expr	: SYMBOL_TABLE
	  {
	     $$ = code_index;
	     code1(OP_SYMTAB);
	  }
	;

symtab_dot : SYMBOL_TABLE '.'
	     {
		 $$.start = code_index;
		 code_line();
	     }
	   ;

Id	: symtab_dot member
	  {
	      $$.start = $1.start;
	      $$.op = code_index;
	      code1( OP_PUSHA_SYM );
	      $$.memcnt = 0;
	  }
	;

id	: Id
	| Id '.' members
	 {
	     $$.start = $1.start;
	     $$.op = $1.op;
	     $$.memcnt = $3;
	 }
	;

/* ---------- global veils ----------- */

non_expr_statement : VEIL lparen veil_ids rparen
		{
		  $$ = code_index ;
		}
		;

veil_ids : veil_id
	     | veil_ids ';' veil_id
	     ;

veil_id : ID
	{
	  SYMTAB *p = check_id ($1);
	  if (p->scope)
	    {
	      yyerror ("cannot veil a local variable");
	      YYERROR;
	    }
	  code_veil_address (p);
	}
	;

%%


/* Have we seen this ID before */
static SYMTAB *
check_id(id)
  char *id ;
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
