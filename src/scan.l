/*
   scan.l -- Algae lexical scanner.

   Copyright (C) 1994-2002  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
*/

/* $Id: scan.l,v 1.12 2003/08/07 02:34:09 ksh Exp $ */

%{

#define  PSR_DATA_HERE

#include "algae.h"
#include <string.h>
#include <ctype.h>
#include "entity.h"
#include "psr.h"
#include "code.h"
#include "parse.h"
#include "mem.h"
#include "pmem.h"
#include "exception.h"
#include "message.h"
#include "print.h"
#include "printf.h"
#include "error.h"

int PROTO(yylex, (YYSTYPE *lexval)) ;
#define YY_DECL int yylex( lexval ) YYSTYPE *lexval;

static int PROTO( line_input, ( char *buff, int buffer_size ) );

#if HAVE_LIBREADLINE
extern int use_readline;

char *PROTO( readline, ( char *prompt ) );
char *PROTO( readline_fgets, ( char *s, int n, char *prompt ) );
#endif /* HAVE_LIBREADLINE */

int issue_prompt;	/* Give prompt if set. */

static char *PROTO(rm_escape, (char *)) ;
static void PROTO(add_to_string, (char *,int)) ;
static char *PROTO(get_string, (void)) ;


static int next_offset ;
static int max_size ; /* max size string curr_line will hold */
static int continuing ; /* helps with prompt */
static int eof_flag ;   /* only used when non-interactive,
	to help when last line lacks a newline */




#define  ADJ_OFFSET()    curr_offset = next_offset ;\
			 next_offset += yyleng 

int yywrap() { return 1; }

/* this needs to be expanded for non-interactive input */
#undef  YY_INPUT
#define YY_INPUT(buff, result, max_size) \
	do {\
	    result = line_input((char *)buff, max_size) ; \
	    if ( flush_flag )\
	    { flush_flag = 0 ; BEGIN(0) ; }}while(0)

static char *input_string;	/* input, if commands not coming from file */ 

#define FGETS(s, n, stream) \
  ((stream) ? fgets (s, n, stream) : sgets (s, n, &input_string))

/*
 * Like fgets(3), except it reads from the string pointed to by `c' and
 * then modifies that string to point to the remainder.
 */
static char *
sgets (s, n, c)
     char *s;
     int n;
     char **c;
{
  char *e, *p;
  int i;

  p = *c;
  if (!*p) return NULL;

  e = strchr (p, '\n');
  i = e ? e-p+1 : strlen (p);
  n--;
  if (i > n) i = n;
  memcpy (s, p, i);
  s[i] = '\0';
  *c = p+i;
  return s;
}

static int
line_input (buff, buffer_size)
     char *buff;
     int buffer_size;
{
  int len;
  char *prompt, *more_input;

  SIGINT_RAISE_ON ();

again:
  if (issue_prompt && yyin)
    {
      if (continuing)
	{
	  prompt = get_prompt (1);
	  continuing = 0;
	}
      else
	prompt = get_prompt (brace_cnt || flow_cnt);

#if HAVE_LIBREADLINE
      if (use_readline)
	more_input = readline_fgets (buff, buffer_size, prompt);
      else
#endif
	{
#ifdef VAX_VMS
	  xputc ('\n', stderr);
#endif
	  xfputs (prompt, stderr);
	  more_input = fgets (buff, buffer_size, yyin);
	}
      FREE (prompt);
    }
  else
    more_input = FGETS (buff, buffer_size, yyin);

  if (more_input)
    {
      curr_line_no++;
      len = strlen (buff);

      if (len > max_size)
	{
	  if (max_size)
	    FREE (curr_line);
	  curr_line = MALLOC (len + 1);
	  max_size = len;
	}

      (void) strcpy (curr_line, buff);
      next_offset = 0;
    }
  else
    {
      if (errno == EINTR)
	{
	  errno = 0;
	  goto again;
	}
      else if (yyin && ferror (yyin))
	{
	  p_error ("%s", strerror (errno));
	  exit (1);
	}
      else			/* EOF */
	len = 0;
    }

  SIGINT_RAISE_OFF ();
  return len;
}


#if HAVE_LIBREADLINE
extern void PROTO (add_history, (char * string));

char *
readline_fgets( s, n, prompt )
  char *s;
  int n;
  char *prompt;
{
    char *line_read = readline( prompt );
    if ( line_read ) {
	if ( *line_read ) add_history( line_read );
	strncpy( s, line_read, n-2 );
	s[n-2] = '\0';
	n = strlen( s );
	s[n] = '\n';
	s[n+1] = '\0';
	free( line_read );
	line_read = s;
    }

    return line_read;
}
#endif /* HAVE_LIBREADLINE */      
	

%}

DIGIT		[0-9]
INTEGER 	{DIGIT}+
PURE_DECIMAL	({DIGIT}+"."|"."{DIGIT}){DIGIT}*
EXPONENT	[eE][-+]?{DIGIT}+
DECIMAL		({INTEGER}{EXPONENT}|{PURE_DECIMAL}{EXPONENT}?)


ID	[_A-Za-z\$][_A-Za-z0-9\$]*

WS	[ \t\v\f\r]
TERMINATOR	[;?]

LITERAL		[()=\[\]:!{}.^]

ADDOP	[-+]
MULOP	[*/%@]
RELOP   [<>]=?|==|!=
CAT     ,
COMMENT	#[^\n]*
CONTINUE	\\{WS}*{COMMENT}?\n
STRBODY		([^\\\"\n]|\\.)*

%x  FLUSH
%x  STRINGING

%%

	/* this always gets done */
	if ( flush_flag )
	{ continuing = 0 ; BEGIN(FLUSH) ; }

<FLUSH>.*\n		{ /* eat the line */
			  BEGIN(0) ;
			}


			  
        /*----------------------------------------*/

	/* keywords are hardwired into the scanner */

NULL		{ ADJ_OFFSET() ; return _NULL ; }
if		{ ADJ_OFFSET() ; return IF ; }
else		{ ADJ_OFFSET() ; return ELSE ; }
elseif		{ ADJ_OFFSET() ; return ELSEIF ; }
while		{ ADJ_OFFSET() ; return WHILE ; }
break		{ ADJ_OFFSET() ; return BREAK ; }
continue	{ ADJ_OFFSET() ; return CONTINUE ; }
function	{ ADJ_OFFSET() ; return FUNCTION_ ; }
for		{ ADJ_OFFSET() ; return FOR ; }
in		{ ADJ_OFFSET() ; return IN ; }
return		{ ADJ_OFFSET() ; return RETURN ; }
local		{ ADJ_OFFSET() ; return LOCAL ; }
self		{ ADJ_OFFSET() ; return SELF  ; }
\$\$		{ ADJ_OFFSET() ; return SYMBOL_TABLE ; }
try		{ ADJ_OFFSET() ; return TRY  ; }
catch		{ ADJ_OFFSET() ; return CATCH  ; }
veil		{ ADJ_OFFSET() ; return VEIL  ; }

{CONTINUE}	{ continuing = 1 ; }

{COMMENT}?\n?	{ ADJ_OFFSET() ;
		  if ( brace_cnt == 0 ) return '\n' ;
		}

{TERMINATOR}{WS}*{CONTINUE}	{ ADJ_OFFSET() ;
			continuing = 1 ;
			lexval->ival = 0 ;
			return *yytext ;
		    }

{TERMINATOR}{WS}*{COMMENT}?\n	{ ADJ_OFFSET() ;
			lexval->ival = 1 ;
			return *yytext ;
		      }

{TERMINATOR}	{ ADJ_OFFSET() ; 
		  lexval->ival = 0 ;
		  return *yytext ;
		}

{LITERAL}	{ ADJ_OFFSET() ; return *yytext ; }

{WS}+			{ ADJ_OFFSET() ; }

{ID}		{ ADJ_OFFSET() ;
		  lexval->id_name = strcpy(MALLOC(yyleng+1),yytext) ;
		  return ID ;
		}

{INTEGER}	{ ADJ_OFFSET() ;
		  errno = 0;
		  lexval->datum = new_INT(atoi(yytext)) ;
		  if ( errno ) {
		      p_error( "Value out of range." );
		      raise_exception();
		  }
		  return CONSTANT ;
		}

{DECIMAL}       { ADJ_OFFSET() ;
		  errno = 0;
		  lexval->datum = new_REAL(atof(yytext)) ;
		  if ( errno ) {
		      p_error( "Value out of range." );
		      raise_exception();
		  }
		  return CONSTANT ;
		}

\"		{ ADJ_OFFSET() ;
		  BEGIN(STRINGING) ;
		}

<STRINGING>{STRBODY}{CONTINUE}	{ 
		  char *s = yytext + yyleng - 2 ;
		  
		  continuing = 1 ;
		  assert(s>=yytext) ;
		  while ( *s != '\\')
		  {
		    assert(s>yytext) ;
		    s-- ;
		  }
		  *s = 0 ;
		  add_to_string(yytext,s-yytext) ;
		}

<STRINGING>{STRBODY}[\"\n]   { char * s ;
			      int last ;

		  ADJ_OFFSET() ;
		  last = yytext[yyleng-1] ;
		  yytext[yyleng-1] = 0 ;
		  add_to_string(yytext,yyleng-1) ;
		  s = rm_escape(get_string()) ;

		  if ( last == '\n' )
		  {
		    p_error("Unterminated character scalar:\n\"%s", s) ;
		    FREE(s) ;
		    raise_exception() ;
		  }
		  /* found end of string */
		  lexval->cs = strcpy(pmem( strlen(s) + 1 ),s) ;
		  FREE(s) ;
		  BEGIN(0) ;
		  return STRING ;
		}

<STRINGING>{STRBODY}  { /* EOF */
		  char *s ;

		  ADJ_OFFSET() ;
		  add_to_string(yytext,yyleng) ;
		  p_error("Unterminated character scalar at EOF:\n\"%s",
			s = rm_escape(get_string())) ;
		  FREE(s) ;
		  raise_exception() ;
		}


{ADDOP}		{ ADJ_OFFSET() ;
		  lexval->ival = *yytext == '+' ? OP_ADD : OP_SUB ;
		  return ADDOP ;
		}

{MULOP}		{ ADJ_OFFSET() ;
		  lexval->ival = *yytext == '*' ? OP_MUL : 
		                 *yytext == '/' ? OP_DIV :
				 *yytext == '%' ? OP_MOD :
				                  OP_PROD;
		  return MULOP ;
		}

{RELOP}		{ ADJ_OFFSET() ;
		  
		  switch( yytext[0] )
		  {
		    case '=' :
			lexval->ival = OP_EQ ;
			break ;

		    case '<' :
			lexval->ival = yytext[1] ? OP_LTE : OP_LT ;
			break ;

		    case '>' :
			lexval->ival = yytext[1] ? OP_GTE : OP_GT ;
			break ;

		    case '!' :
			lexval->ival = OP_NE ;
			break ;

		    default :
			wipeout("scanner relop") ;
		  }
		  return RELOP ;
		}


[-+*/@%]=	{ ADJ_OFFSET() ; 
			
		  switch (yytext[0])
                  {
                    case '-':
                      lexval->ival = OP_SUB;
                      break;
                    case '+':
                      lexval->ival = OP_ADD;
                      break;
                    case '*':
                      lexval->ival = OP_MUL;
                      break;
                    case '/':
                      lexval->ival = OP_DIV;
                      break;
                    case '@':
                      lexval->ival = OP_PROD;
                      break;
                    case '%':
                      lexval->ival = OP_MOD;
                      break;
                    default:
                      wipeout ("scanner binop_assign");
                  }

		  return BINOP_ASSIGN ;
		}


"||"		{ ADJ_OFFSET() ;  return SHORT_OR ; }
"&&"		{ ADJ_OFFSET() ;  return SHORT_AND ; }

"|"		{ ADJ_OFFSET() ;  return OR ; }
"&"		{ ADJ_OFFSET() ;  return AND ; }
{CAT}		{ ADJ_OFFSET() ;  return CAT ; }
\'		{ ADJ_OFFSET() ;  return TRANS ; }


.		{ /* anything not covered */
		  ADJ_OFFSET() ;
		  if ( yytext[0] )
		    yyerror("unexpected input character -- ignored") ;
		}


<<EOF>> { if ( !interactive && eof_flag == 0 )
	  { eof_flag = 1 ; return '\n' /* phoney */ ; }
	  else  return EOF ;
	}

%%


typedef struct ps_state
  {
    struct ps_state *link;
    YY_BUFFER_STATE buffer_state;
    FILE *yyin;
    char *input_string;
    char *curr_line;
    int curr_line_no;
    char *curr_file;
    int max_size;
    int eof_flag;

    int flush_flag;
    int interaction;
    int prompting;
    int brace_cnt;
    int flow_cnt;
    CODE_BLOCK code_block;
    struct pm saved_pmem;
    void *jmp_top;
    void *bc_top;
  }
PS_STATE;

extern int interactive;

/*
 * The `stdin_is_interactive' flag may be set in `initialize'.  If it's set,
 * then we use interactive mode for stdin whether it looks like a terminal
 * or not.
 */
extern int stdin_is_interactive;

static PS_STATE *ps_state_list;

/*
 * Use `push_parser_scanner_state' to save the parser's state (the file
 * from which it's reading, along with globals like `interactive', etc.)
 * and change to a new one.
 */

void 
push_parser_scanner_state (fp, file_name)
     FILE *fp;
     char *file_name;
{
  /*
   * If `fp' is NULL, then `file_name' is a pointer to a command string.
   * Otherwise, `fp' points to the input file, and `file_name' is its name.
   * The `file_name' string is not FREE'd.
   */

  PS_STATE *p = MALLOC (sizeof (PS_STATE));

  p->link = ps_state_list;
  ps_state_list = p;

  p->buffer_state = YY_CURRENT_BUFFER;
  p->yyin = yyin;
  yyin = fp;
  p->input_string = input_string;
  input_string = fp ? NULL : file_name;
  yy_switch_to_buffer (yy_create_buffer (yyin, YY_BUF_SIZE));

  p->interaction = interactive;
  p->prompting = issue_prompt;

  issue_prompt = fp && isatty (fileno (fp));
  interactive = issue_prompt || stdin_is_interactive && fp == stdin;

  continuing = 0;

  p->curr_line = curr_line;
  curr_line = (char *) 0;
  p->max_size = max_size;
  max_size = 0;
  p->curr_line_no = curr_line_no;
  curr_line_no = 0;
  p->curr_file = curr_file;
  curr_file = file_name;

  p->eof_flag = eof_flag;
  eof_flag = 0;
  p->flush_flag = flush_flag;
  flush_flag = 0;
  p->brace_cnt = brace_cnt;
  brace_cnt = 0;
  p->flow_cnt = flow_cnt;
  flow_cnt = 0;

  p->code_block = the_code_block;
  new_code_block ();

  p->saved_pmem = active_pmem;
  (void) memset (&active_pmem, 0, sizeof (active_pmem));

  p->jmp_top = jmp_top_push ();
  p->bc_top = bc_top_push ();
}

void 
pop_parser_scanner_state ()
{
  PS_STATE *p;

  assert (ps_state_list);

  p = ps_state_list;
  ps_state_list = p->link;

  yyin = p->yyin;
  input_string = p->input_string;
  yy_delete_buffer (YY_CURRENT_BUFFER);
  if (p->buffer_state)
    yy_switch_to_buffer (p->buffer_state);

  interactive = p->interaction;
  issue_prompt = p->prompting;

  continuing = 0;

  if (curr_line)
    FREE (curr_line);
  curr_line = p->curr_line;
  max_size = p->max_size;
  curr_line_no = p->curr_line_no;
  curr_file = p->curr_file;

  flush_flag = p->flush_flag;
  eof_flag = p->eof_flag;
  brace_cnt = p->brace_cnt;
  flow_cnt = p->flow_cnt;

  FREE (code_base);
  the_code_block = p->code_block;

  free_all_pmem ();
  active_pmem = p->saved_pmem;

  jmp_top_pop (p->jmp_top);
  bc_top_pop (p->bc_top);

  FREE (p);
}


/*----------  process escape characters in strings ----*/


#define isoctal(x)  ((x)>='0'&&(x)<='7')

#define  hex_value(x)   ((x)<='F'?(x)-'A'+10:(x)-'a'+10)


/* process one , two or three octal digits
   moving a pointer forward by reference */
static int octal( start_p )
  char **start_p ;
{ register char *p = *start_p ;
  register unsigned x ;

  x = *p++ - '0' ;
  if ( isoctal(*p) )
  {
    x = (x<<3) + *p++ - '0' ;
    if ( isoctal(*p) )   x = (x<<3) + *p++ - '0' ;
  }
  *start_p = p ;
  return  x & 0xff ;
}

/* process one or two hex digits
   moving a pointer forward by reference */

static int  hex( start_p )
  char **start_p ;
{ register unsigned char *p = (unsigned char*) *start_p ;
  register unsigned x ;

  if ( isdigit(*p) ) x = *p - '0' ;
  else  x = hex_value(*p) ;
  p++ ;

  if ( isdigit(*p) ) x = (x<<4) + *p++ - '0' ;
  else
  if ( isxdigit(*p) )
  { x = (x<<4) + hex_value(*p) ; p++ ; }

  *start_p = (char *) p ;
  return x ;
}

#define  NUM_ESCAPE_CHARS    8

static struct { char in , out ; } 
escape_test[NUM_ESCAPE_CHARS+1] = {
    'n' , '\n',		/* newline	   */
    't' , '\t',		/* tab		   */
    'f' , '\f',		/* formfeed	   */
    'b' , '\b',		/* backspace	   */
    'r' , '\r',		/* carriage return */
    'a' , '\07',	/* bell		   */
    'v' , '\013',	/* vertical tab	   */
    'e' , '\033',	/* escape	   */
    0 , 0
};

/* all other escaped chars stand for themselves */


/* process the escape characters in a string, in place . */

static char *rm_escape(s)
  char *s ;
{ char *p, *q ;
  char *t ;
  int i ;

  q = p = s ;

  while ( *p )
      if ( *p == '\\' )
      { 
        escape_test[NUM_ESCAPE_CHARS].in = * ++p ; /* sentinel */
        i = 0 ;
        while ( escape_test[i].in != *p )  i++ ;

        if ( i != NUM_ESCAPE_CHARS )  /* in table */
        { 
          p++ ; *q++ = escape_test[i].out ;
        }
        else
        if ( isoctal(*p) ) 
        { /* keep p in register */
          t = p ;  *q++ = octal(&t) ; p = t ;
        }
        else
        if ( *p == 'x' && isxdigit(*(unsigned char*)(p+1)) )
        {
          t = p+1 ; *q++ = hex(&t) ; p = t ;
        }
	else
	if ( *p == 0 )  *q++ = '\\' ; 
        else  /* not an escape sequence */
           *q++ = *p++ ;
      }
      else  *q++ = *p++ ;

  *q = 0 ;
  return s ;
}


/* stuff to put multiple line strings together */

static char *the_string ; /* built it here */
static int len ;  /* strlen(the_string) */

static char *get_string()
{
  char *s = the_string ;
  the_string = (char *) 0 ;
  len = 0 ;
  return s ;
}

static void 
add_to_string( s,  slen) 
  char *s ;
  int slen ;
{
    char *new = MALLOC(len+slen+1) ;
    if ( the_string )
    { (void) memcpy(new, the_string, len) ;
      FREE(the_string) ;
    }
    (void) strcpy(new+len,s) ;
    len += slen ;
    the_string = new ;
}
