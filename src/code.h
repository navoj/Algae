/*
   code.h -- Byte-compile code.

   Copyright (C) 1994-2002  K. Scott Hunziker.
   Copyright (C) 1990-1994  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: code.h,v 1.4 2003/08/07 02:34:09 ksh Exp $ */

#ifndef   CODE_H
#define   CODE_H	1

#include "entity.h"
#include "ptypes.h"

typedef int OFFSET;

typedef struct
  {
    INST *base;
    OFFSET index;
    OFFSET danger;
    OFFSET limit;
  }
CODE_BLOCK;

extern CODE_BLOCK the_code_block;

#define  code_base  the_code_block.base
#define  code_index  the_code_block.index
#define  code_danger  the_code_block.danger
#define  code_limit  the_code_block.limit

#define INST_PTR(i)  (code_base+(i))

#define CODE_SIZE    1024
#define CODE_SAFETY    64

enum
  {
    OP_HALT,
    OP_POP,
    OP_PUSHC,
    OP_PUSHS,
    OP_PUSHS2E,
    OP_PUSHI,
    OP_PUSHA,
    OP_LPUSHI,
    OP_LPUSHA,
    OP_PUSH_NULL,
    OP_PRINT,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_PROD,
    OP_UMINUS,
    OP_NOT,
    OP_TRANS,
    OP_POWER,
    OP_ASSIGN,
    OP_PILE,			/* by column */
    OP_APPEND,
    OP_MATRIX,			/* Convert to a matrix. */
    OP_VSEQ,			/*  expr:expr:expr  */
    OP_P1D,
    OP_P2D,
    OP_ASSIGN_SV,
    OP_ASSIGN_SM,
    OP_JMP,
    OP_JZ,
    OP_JNZ,
    OP_EQ,
    OP_NE,
    OP_LT,
    OP_GT,
    OP_GTE,
    OP_LTE,
    OP_AND,
    OP_OR,
    OP_RET,
    OP_MK_TABLE,		/* make a table */
    OP_R_TABLE,			/* replace in table , used by {x=5} */
    OP_ID_TABLE,		/* used by {x} */
    OP_REF_MEMB,
    OP_ASG_MEMB,
    OP_CALL,
    OP_IFL,
    OP_KFL,
    OP_FL,			/* init, kill and for_loop */
    OP_SYMTAB,
    OP_E2STR,			/* convert entity to string */
    OP_LINE,			/* update the current line number */
    OP_FILE,			/* update the current file name */
    OP_NOP,			/* no op */
    OP_PUSHA_SYM,		/* address from symbol table */
    OP_PUSHI_SYM,		/* indirect reference in symbol table */
    OP_SELF,			/* function referring to itself */
    OP_TRY,			/* begin a TRY block */
    OP_CATCH,			/* begin a CATCH block */
    OP_VEIL,			/* veil a global */
    NUM_OP_CODES
  };


#define  code1(x)   code_base[code_index++].op = (x)
#define code_line()	( line_flag ? 0 : ( code1( OP_LINE ), \
					    code1( curr_line_no ), \
					    line_flag = 1 ) )

void PROTO (code2, (int, void *));
void PROTO (icode2, (int, int));
void PROTO (code_dup_id, (int, int));
void PROTO (code_id_address, (SYMTAB *));
void PROTO (code_veil_address, (SYMTAB *));
void PROTO (cleanup_local_scope, (void));


void PROTO (da, (INST *, FILE *));

int PROTO (BC_insert, (int, OFFSET));
void PROTO (BC_clear, (OFFSET, OFFSET));
void PROTO (BC_new, (void));
void PROTO (code_jmp, (int, OFFSET));
void PROTO (patch_jmp, (OFFSET));
void PROTO (reset_jmp, (int flag));
void PROTO (shrink_code_block, (void));

void *PROTO (jmp_top_push, (void));
void *PROTO (bc_top_push, (void));
void PROTO (jmp_top_pop, (void *));
void PROTO (bc_top_pop, (void *));

void PROTO (cleanup_scope_stack, (void));
void PROTO (save_id, (SYMTAB *p));

void PROTO (new_code_block, (void));

ENTITY *PROTO (execute, (INST *, DATUM *, FUNCTION *));

#endif /* CODE_H  */
