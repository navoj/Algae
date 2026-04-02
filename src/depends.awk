
# find include dependencies in C source
#
# mawk -f depends.awk [-v config=filename] [-v exclude=re]  C_source_files
#         -- prints a dependency list suitable for make
#         -- ignores   #include <   >
#         -- optional config file name included unconditionally
#	  -- exclude file names matching regular expression


BEGIN {  stack_index = 0; dir = "" # stack[] holds the input files

  for(i = 1 ; i < ARGC ; i++)
  { 
    file = ARGV[i]
    if ( file !~ /\.c$/ )  continue  # skip it
    outfile = substr(file, 1, length(file)-2) ".o"

    # INCLUDED[] stores the set of included files
    # -- start with the empty set
    for( j in INCLUDED ) delete INCLUDED[j]
    if (config) INCLUDED[config] = "";

    while ( 1 )
    {
	k = getline line < file

        if ( k <= 0 )
	{ close(file)
	  if ( stack_index == 0 )  break # empty stack
	  else  
	  { file = stack[ stack_index ]
	    dir = dir_stack[ stack_index-- ]
	    continue
	  }
        }

	if ( line ~ /^#include[ \t]+".*"/ )
	{
	  split(line, X, "\"")  # filename is in X[2]

	  match(X[2],/(.*[/])?(.*)/,fn)
	  filedir = fn[1]
	  filename = dir X[2]
	  if ( filename in INCLUDED ) # we've already included it
		continue
	  if ( exclude && filename ~ exclude) continue

	  #push current file 
	  stack[ ++stack_index ] = file
	  dir_stack[ stack_index ] = dir
	  INCLUDED[ file = filename ] = ""
	  dir = dir filedir
        }
    }  # end of while
    
   # test if INCLUDED is empty
   flag = 0 # on once the front is printed 
   for( j in INCLUDED )
      if ( ! flag )  
      { printf "%s : %s" , outfile, j ; flag = 1 }
      else  printf " %s" , j

   if ( flag )  print ""

  }# end of loop over files in ARGV[i]

}
