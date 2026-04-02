/*
   two_type.h -- Macros for testing two types or classes with a big switch.

   Copyright (C) 1994-96  K. Scott Hunziker.
   Copyright (C) 1990-94  The Boeing Company.

   See the file COPYING for license, warranty, and permission details.
 */

/* $Id: two_type.h,v 1.2 1996/10/25 05:28:31 ksh Exp $ */

#ifndef TWO_TYPE_H
#define TWO_TYPE_H	1

#define  TWO_THINGS(l,r,n)  (((int)(n))*((int)(l)) +((int)(r)))


/*------------CLASS-----------------------------------*/

#define  TWO_CLASS_(l,r)   TWO_THINGS(l,r,NUM_CLASS)
#define  TWO_CLASS(l,r)	   TWO_CLASS_((l)->class,(r)->class)


#define undefined_class_undefined_class	TWO_CLASS_(undefined_class,undefined_class)
#define undefined_class_scalar		TWO_CLASS_(undefined_class,scalar)
#define undefined_class_table		TWO_CLASS_(undefined_class,table)
#define undefined_class_vector		TWO_CLASS_(undefined_class,vector)
#define undefined_class_matrix		TWO_CLASS_(undefined_class,matrix)
#define undefined_class_function	TWO_CLASS_(undefined_class,function)
#define scalar_undefined_class		TWO_CLASS_(scalar,undefined_class)
#define scalar_scalar			TWO_CLASS_(scalar,scalar)
#define scalar_table			TWO_CLASS_(scalar,table)
#define scalar_vector			TWO_CLASS_(scalar,vector)
#define scalar_matrix			TWO_CLASS_(scalar,matrix)
#define scalar_function			TWO_CLASS_(scalar,function)
#define table_undefined_class		TWO_CLASS_(table,undefined_class)
#define table_scalar			TWO_CLASS_(table,scalar)
#define table_table			TWO_CLASS_(table,table)
#define table_vector			TWO_CLASS_(table,vector)
#define table_matrix			TWO_CLASS_(table,matrix)
#define table_function			TWO_CLASS_(table,function)
#define vector_undefined_class		TWO_CLASS_(vector,undefined_class)
#define vector_scalar			TWO_CLASS_(vector,scalar)
#define vector_table			TWO_CLASS_(vector,table)
#define vector_vector			TWO_CLASS_(vector,vector)
#define vector_matrix			TWO_CLASS_(vector,matrix)
#define vector_function			TWO_CLASS_(vector,function)
#define matrix_undefined_class		TWO_CLASS_(matrix,undefined_class)
#define matrix_scalar			TWO_CLASS_(matrix,scalar)
#define matrix_table			TWO_CLASS_(matrix,table)
#define matrix_vector			TWO_CLASS_(matrix,vector)
#define matrix_matrix			TWO_CLASS_(matrix,matrix)
#define matrix_function			TWO_CLASS_(matrix,function)
#define function_undefined_class	TWO_CLASS_(function,undefined_class)
#define function_scalar			TWO_CLASS_(function,scalar)
#define function_table			TWO_CLASS_(function,table)
#define function_vector			TWO_CLASS_(function,vector)
#define function_matrix			TWO_CLASS_(function,matrix)
#define function_function		TWO_CLASS_(function,function)

/*------------------TYPE------------------------------*/

#define TWO_TYPE(l,r)        TWO_THINGS(l,r,NUM_TYPE)

#define undefined_type_undefined_type	TWO_TYPE(undefined_type,undefined_type)
#define undefined_type_integer		TWO_TYPE(undefined_type,integer)
#define undefined_type_real		TWO_TYPE(undefined_type,real)
#define undefined_type_complex		TWO_TYPE(undefined_type,complex)
#define undefined_type_character	TWO_TYPE(undefined_type,character)
#define integer_undefined_type		TWO_TYPE(integer,undefined_type)
#define integer_integer			TWO_TYPE(integer,integer)
#define integer_real			TWO_TYPE(integer,real)
#define integer_complex			TWO_TYPE(integer,complex)
#define integer_character		TWO_TYPE(integer,character)
#define real_undefined_type		TWO_TYPE(real,undefined_type)
#define real_integer			TWO_TYPE(real,integer)
#define real_real			TWO_TYPE(real,real)
#define real_complex			TWO_TYPE(real,complex)
#define real_character			TWO_TYPE(real,character)
#define complex_undefined_type		TWO_TYPE(complex,undefined_type)
#define complex_integer			TWO_TYPE(complex,integer)
#define complex_real			TWO_TYPE(complex,real)
#define complex_complex			TWO_TYPE(complex,complex)
#define complex_character		TWO_TYPE(complex,character)
#define character_undefined_type	TWO_TYPE(character,undefined_type)
#define character_integer		TWO_TYPE(character,integer)
#define character_real			TWO_TYPE(character,real)
#define character_complex		TWO_TYPE(character,complex)
#define character_character		TWO_TYPE(character,character)

/*------------------DENSITY------------------------------*/

#define TWO_DENSITY(l,r)        	TWO_THINGS(l,r,NUM_DENSITY)

#define undefined_density_undefined_density \
  TWO_DENSITY(undefined_density,undefined_density)
#define undefined_density_dense \
  TWO_DENSITY(undefined_density,dense)
#define undefined_density_sparse \
  TWO_DENSITY(undefined_density,sparse)
#define undefined_density_sparse_upper \
  TWO_DENSITY(undefined_density,sparse_upper)
#define dense_undefined_density \
  TWO_DENSITY(dense,undefined_density)
#define dense_dense \
  TWO_DENSITY(dense,dense)
#define dense_sparse \
  TWO_DENSITY(dense,sparse)
#define dense_sparse_upper \
  TWO_DENSITY(dense,sparse_upper)
#define sparse_undefined_density \
  TWO_DENSITY(sparse,undefined_density)
#define sparse_dense \
  TWO_DENSITY(sparse,dense)
#define sparse_sparse \
  TWO_DENSITY(sparse,sparse)
#define sparse_sparse_upper \
  TWO_DENSITY(sparse,sparse_upper)
#define sparse_upper_undefined_density \
  TWO_DENSITY(sparse_upper,undefined_density)
#define sparse_upper_dense \
  TWO_DENSITY(sparse_upper,dense)
#define sparse_upper_sparse \
  TWO_DENSITY(sparse_upper,sparse)
#define sparse_upper_sparse_upper \
  TWO_DENSITY(sparse_upper,sparse_upper)

/*------------------SYMMETRY------------------------------*/

#define TWO_SYMMETRY( l, r )        TWO_THINGS( l, r, NUM_SYMMETRY )

#define undefined_symmetry_undefined_symmetry \
  TWO_SYMMETRY( undefined_symmetry, undefined_symmetry )
#define undefined_symmetry_general \
  TWO_SYMMETRY( undefined_symmetry, general )
#define undefined_symmetry_symmetric \
  TWO_SYMMETRY( undefined_symmetry, symmetric )
#define undefined_symmetry_hermitian \
  TWO_SYMMETRY( undefined_symmetry, hermitian )
#define general_undefined_symmetry \
  TWO_SYMMETRY( general, undefined_symmetry )
#define general_general \
  TWO_SYMMETRY( general, general )
#define general_symmetric \
  TWO_SYMMETRY( general, symmetric )
#define general_hermitian \
  TWO_SYMMETRY( general, hermitian )
#define symmetric_undefined_symmetry \
  TWO_SYMMETRY( symmetric, undefined_symmetry )
#define symmetric_general \
  TWO_SYMMETRY( symmetric, general )
#define symmetric_symmetric \
  TWO_SYMMETRY( symmetric, symmetric )
#define symmetric_hermitian \
  TWO_SYMMETRY( symmetric, hermitian )
#define hermitian_undefined_symmetry \
  TWO_SYMMETRY( hermitian, undefined_symmetry )
#define hermitian_general \
  TWO_SYMMETRY( hermitian, general )
#define hermitian_symmetric \
  TWO_SYMMETRY( hermitian, symmetric )
#define hermitian_hermitian \
  TWO_SYMMETRY( hermitian, hermitian )

#endif /* TWO_TYPE_H */
