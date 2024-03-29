/*
 *  mutt_vc_query - vCard query utility for mutt
 *  Copyright (C) 2003  Andrew Hsu
 * 
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA
 *
 *  $Id: main.c,v 1.6 2003/05/19 07:44:50 ahsu Rel $
 */

#include <limits.h>
#include "result.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_STRING "mutt_vc_query"
#endif

#define DEFAULT_HOME_ROLO_DIR ".rolo"
#define DEFAULT_FILENAME "contacts.vcf"

static void set_defaults ();
static void process_command_line_args (int argc, char *const *argv);
static void display_usage (const char *prog_name);
static void display_version ();
static void display_license ();

char *query_string = NULL;
char datafile[PATH_MAX];
char *misc_field = NULL;
int sort_by = 0;

/***************************************************************************
    Sets the default settings.
 */

static void
set_defaults ()
{
  char *home = NULL;

  home = getenv ("HOME");
  strcpy (datafile, home);
  strncat (datafile, "/", 1);
  strncat (datafile, DEFAULT_HOME_ROLO_DIR, strlen (DEFAULT_HOME_ROLO_DIR));
  strncat (datafile, "/", 1);
  strncat (datafile, DEFAULT_FILENAME, strlen (DEFAULT_FILENAME));

  sort_by = SORT_RESULTS_BY_NAME;
}

/***************************************************************************
    Parses the command-line arguments.
 */

static void
process_command_line_args (int argc, char *const *argv)
{
  int ch = -1;
  char *progname = NULL;

  progname = argv[0];

  while (-1 != (ch = getopt (argc, argv, "f:met:vVh")))
    {
      switch (ch)
        {
        case 'f':
          strcpy (datafile, optarg);
          break;
        case 'm':
          sort_by = SORT_RESULTS_BY_MISC;
          break;
        case 'e':
          sort_by = SORT_RESULTS_BY_EMAIL;
          break;
        case 't':
          misc_field = strdup (optarg);
          break;
        case 'v':
          display_version ();
          exit (0);
          break;
        case 'V':
          display_license ();
          exit (0);
          break;
        case 'h':
        case '?':
        default:
          display_usage (progname);
          exit (0);
        }
    }

  argc -= optind;
  argv += optind;

  if (1 != argc)
    {
      fprintf (stderr, "Invalid number of arguments.\n");
      display_usage (progname);
      exit (1);
    }

  query_string = argv[0];
}

/***************************************************************************
    Outputs a one-line version statement.
 */

static void
display_version ()
{
  printf ("%s\n", PACKAGE_STRING);
}

/***************************************************************************
    Outputs the software license.
 */

static void
display_license ()
{
  printf ("mutt_vc_query - vCard query utility for mutt\n");
  printf ("Copyright (C) 2003  Andrew Hsu\n");
  printf ("\n");
  printf ("This program is free software;");
  printf (" you can redistribute it and/or modify\n");
  printf ("it under the terms of the");
  printf (" GNU General Public License as published by\n");
  printf ("the Free Software Foundation;");
  printf (" either version 2 of the License, or\n");
  printf ("(at your option) any later version.\n");
  printf ("\n");
  printf ("This program is distributed");
  printf (" in the hope that it will be useful,\n");
  printf ("but WITHOUT ANY WARRANTY;");
  printf (" without even the implied warranty of\n");
  printf ("MERCHANTABILITY or FITNESS FOR A PARTICULAR");
  printf (" PURPOSE.  See the\n");
  printf ("GNU General Public License for more details.\n");
  printf ("\n");
  printf ("You should have received a copy of");
  printf (" the GNU General Public License\n");
  printf ("along with this program;");
  printf (" if not, write to the Free Software\n");
  printf ("Foundation, Inc., 59 Temple Place, Suite 330,");
  printf (" Boston, MA  02111-1307  USA\n");
}

/***************************************************************************
    Ouputs how to use the program.
 */

static void
display_usage (const char *prog_name)
{
  printf ("usage: %s [-e|-m] [-t <type>] [-f <file>] query_string\n",
          prog_name);
  printf ("       %s -v\n", prog_name);
  printf ("       %s -V\n", prog_name);
  printf ("       %s -h\n", prog_name);
  printf ("options:\n");
  printf ("  -e            sort results by email\n");
  printf ("  -m            sort results by misc\n");
  printf ("  -f <file>     specify a data file to use\n");
  printf ("  -t <type>     the type to use for the misc field\n");
  printf ("  -v            display version\n");
  printf ("  -V            display copyright and license\n");
  printf ("  -h            this help message\n");
}

/***************************************************************************
    The main function.
 */

int
main (int argc, char *argv[])
{
  query_result *results = NULL;
  FILE *fp = NULL;
  int searched = 0;
  int rc = 0;
  int ret_val = 0;

  set_defaults ();
  process_command_line_args (argc, argv);
  results = create_query_result ();

  fp = fopen (datafile, "r");
  if (NULL == fp)
    {
      fprintf (stderr, "Unable to open data file: %s\n", datafile);
      exit (1);
    }

  printf ("Searching database ... ");
  get_results (fp, query_string, misc_field, &searched, results, &rc);

  if (0 == rc)
    {
      /* the query produced no results */
      printf ("no matches found.\n");
      ret_val = 1;
    }
  else
    {
      /* success in finding matches for the query */
      printf ("%i entries ... ", searched);
      printf ("%i matching.\n", rc);
      sort_results (results, rc, sort_by);
      print_results (results);
      ret_val = 0;
    }

  return ret_val;
}
