/*
 * Algae reads some of its standard functions at startup.  This macro
 * gives the default name for that file.  At run time, it can be
 * overridden by the ALGAE_RC0 environment variable or skipped
 * altogether with the "-S" command-line option.
 */
#ifndef ALGAE_RC0_DEF
#define ALGAE_RC0_DEF	"/home/jtrujil1/lib/algae/4.3.6/algae.A"
#endif
