/*
 * Copyright (C) 2001 SecureWorx South Africa (Pty) Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#ifdef __LOCAL__
#include <locale.h>
#include <libintl.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
//#include <unistd.h>
#include <string.h>
#include "errdef.h"
#include "errcpp.h"
#include "errjava.h"
#include "errc.h"

#ifdef __LOCAL__
#define _(str) gettext(str)
#else
#define _(str) str
#endif

/*! \brief This program converts an error file definition into the needed C++ and Java classes to be used by other programs

    The parameters to be passed to this program is:
       --c++ - Generate C++ class.
       --java - Generate Java class.
       --cout - Output base for C++ class.
       --jout - Output base for Java class.
       --in - The errorcode definition file.
       --c    - Generate C functions.
       --cnout - Output base for C files.

    The program will process the input file and generate the required output files. The definition of the
    input file is: Documents/Errors/defintion.rtf

 */

//--------------------------------------------------------------------------------------------------------------------
//                             Definitions
//--------------------------------------------------------------------------------------------------------------------

//! This means no language types should be output
#define ERRCONV_NO_OUT    0
//! This means there should be a C++ output
#define ERRCONV_C_OUT     1
//! This means there should be a Java output
#define ERRCONV_JAVA_OUT  2
//! This means there should be a C output
#define ERRCONV_CN_OUT    4

//--------------------------------------------------------------------------------------------------------------------
//                             Non - class functions
//--------------------------------------------------------------------------------------------------------------------
/*! \brief Show help on command line and returns an error code.
    \pre Must either request help or provide invalid flag to end up here.
    \post Printed the help screen to stderr.
    \param reason The string to be printed as reason why we are here. If it is NULL assumes help requested.
    \param program_name The name of the program file.
    \returns If reason is set returns 1 otherwise returns 0.

    This function does not fit in with any class that may be needed in this program.
    It depends on the command line parameters as defined for this program.
 */
int show_help_and_exit( char *reason, char *program_name )
{
  if (reason)
    fprintf( stderr, "\n%s\n\n", reason );
  fprintf( stderr, _("Usage: %s --in infile [--c++] [--java] [--cout base-name] [--jout base-name] [--help]\n"), program_name );
  fprintf( stderr, _("      --in    - The error definition input file.\n") );
  fprintf( stderr, _("      --c++   - Generate C++ files. Requires that --cout be specified.\n") );
  fprintf( stderr, _("      --java  - Generate Java files. Requires that --jout be specified.\n") );
  fprintf( stderr, _("      --cout  - The base name used for generating the C++ file names.\n") );
  fprintf( stderr, _("      --jout  - The base name used for generating the Java file names.\n") );
  fprintf( stderr, _("      --c     - Generate C files. Requires that --cnout be specified.\n") );
  fprintf( stderr, _("      --cnout - The base name used for generating the C file names.\n") );
  fprintf( stderr, _("      --help  - This screen.\n\n") );
  if (reason) return( 1 );
  else return( 0 );
}

/*! \brief Process commandline arguments and returned needed configuration information
    \pre Must be called before the program can do it's job.
    \post Parses argc and argv, set's the parameters.
    \param flag This variable contains the flag type settings.
    \param cout The base name of the C++ output file as specified on the command line.
    \param jout The base name of the Java output file as specified on the command line.
    \param cnout The base name of the C output file as specified on the command line.
    \param infile The name of the input file with the error definitions.
    \param argc The argument counter for the command line.
    \param argv The array containing the command line.
    \returns Non-zero on error.
    
    This function does not fit in with any class that may be needed in this program.
    It depends on the ERRCONV_* conditional defines and the command line parameters as 
    defined for this program.
 */
int process_arguments( int *flag, char cout[255], char jout[255], char cnout[255], char infile[255], int argc, char **argv )
{
  int arg_counter, option, option_index; 
  struct option my_options[]=
   {
    { "c++", 0, NULL, 1 },
    { "java", 0, NULL, 2 },
    { "cout", 1, NULL, 3 },
    { "jout", 1, NULL, 4 },
    { "in", 1, NULL, 5 },
    { "c", 0, NULL, 7 },
    { "cnout", 1, NULL, 8 },
    { "help", 0, NULL, 6 },
    { NULL, 0, NULL, 0 }
   };
  
  if (argc==1) return( show_help_and_exit( _( "No parameters defined." ), argv[0] ));
  arg_counter=0;
  *flag=ERRCONV_NO_OUT;
  infile[0]=0;
  cout[0]=0;
  jout[0]=0;
  cnout[0]=0;
  opterr=0;
  while (1) 
    {
     option=getopt_long( argc, argv, "", my_options, &option_index );
     if (option==-1)
      {
	if (*flag==ERRCONV_NO_OUT)
          {
	    return( show_help_and_exit( _("No output type defined."), argv[0] ) );
	  }
        if (infile[0]==0)
          {
            return( show_help_and_exit( _("No input file defined."), argv[0] ) );
	  }
        if (((*flag)&ERRCONV_C_OUT) && cout[0]==0)
          {
            return( show_help_and_exit( _("C++ base name not specified."), argv[0] ) );
	  }
        if (((*flag)&ERRCONV_JAVA_OUT) && jout[0]==0)
	  {
	    return( show_help_and_exit( _("Java base name not specified."), argv[0] ) );
	  }
	if (((*flag)&ERRCONV_CN_OUT) && cnout[0]==0)
	  {
	    return( show_help_and_exit( _("C base name not specified."), argv[0] ) );
	  }
        return( 0 );
      }
     switch (option)
      {
      case 1: // c++
        (*flag)|=ERRCONV_C_OUT;
        break;
      case 2: // java
        (*flag)|=ERRCONV_JAVA_OUT;
        break;
      case 3: // cout
        strcpy( cout, optarg );
        break;
      case 4: // jout
        strcpy( jout, optarg );
        break;
      case 5: // in
        strcpy( infile, optarg );
        break;
      case 6: // help
	return( show_help_and_exit( NULL, argv[0] ) );
      case 7: // c
	(*flag)|=ERRCONV_CN_OUT;
	break;
      case 8: // cnout
	strcpy( cnout, optarg );
	break;
      default:
	return( show_help_and_exit( _("Invalid command line option."), argv[0] ) );
      };

    }
}

//--------------------------------------------------------------------------------------------------------------------
//                     MAIN
//--------------------------------------------------------------------------------------------------------------------
/*! \brief The main function for this program.
    \pre None
    \post The files have been generated and either an error returned or success returned.
    \param argc The argument count to this program.
    \param argv The array of command line arguments to this program.
    \returns 0 on success and 1 on failure.
    
    The main function may not be part of any class.
    The program will first parse the command line and then create and call the relevant classes to create the
    C++ and Java errorcode definitions from the input file.
 */
int main( int argc, char **argv )
{
  int result;
  int flag;
  char cout[255], jout[255], infile[255], cnout[255];
  Error_Definitions *err_def;
  CPP_Errors *cpp;
  Java_Errors *java;
  C_Errors *c;

  cpp=NULL;
  java=NULL;
  c=NULL;
#ifdef __LOCAL__
  setlocale( LC_ALL, "" );
  bindtextdomain( PACKAGE, NULL );
  textdomain( PACKAGE );
#endif
  printf( _("%s %s compiled on %s, %s\n"), PACKAGE, VERSION, __DATE__, __TIME__ );
  result=process_arguments( &flag, cout, jout, cnout, infile, argc, argv );
  if (result)
    { // Assumes error code has been printed.
      return( 1 );
    }
  
  err_def=new Error_Definitions( infile );
  if (!err_def) 
    { // No mem
      fprintf( stderr, _("Out of memory.\n") );
      return( 1 );
    }

  if (!err_def->isOk())
    { // Assumes error code has been printed.
      delete err_def;
      return( 1 );
    }

  if (flag&ERRCONV_C_OUT)
    {
      cpp=new CPP_Errors( cout, err_def );
      if (!cpp)
	{
	  delete err_def;
	  fprintf( stderr, _("Out of memory.\n") );
          return( 1 );
	}
      if (!cpp->isOk())
        {
	  fprintf( stderr, _("Unable to initialize CPP Parser.\n") );
	  delete err_def;
          delete cpp;
          cpp=NULL;
          return( 1 );
	}
      if (cpp->execute())
        {
	  delete err_def;
	  delete cpp;
	  return( 1 );
	}
    }
  if (flag&ERRCONV_JAVA_OUT)
    {
      java=new Java_Errors( jout, err_def );
      if (!java)
	{
	  delete err_def;
	  if (cpp) delete cpp;
	  fprintf( stderr, _("Out of memory.\n") );
          return( 1 );
	}
      if (!java->isOk())
        {
	  delete err_def;
	  if (cpp) delete cpp;
	  delete java;
	  fprintf( stderr, _("Unable to initialize Java Parser.\n") );
          delete cpp;
          cpp=NULL;
          return( 1 );
	}
      if (java->execute())
        {
	  delete err_def;
	  if (cpp) delete cpp;
	  delete java;
	  return( 1 );
	}
    }
  if (flag&ERRCONV_CN_OUT)
    {
      c=new C_Errors( cnout, err_def );
      if (!c)
	{
	  delete err_def;
	  if (cpp) delete cpp;
	  if (java) delete java;
	  fprintf( stderr, _("Out of memory.\n") );
          return( 1 );
	}
      if (!c->isOk())
        {
	  delete err_def;
	  if (cpp) delete cpp;
	  if (java) delete java;
	  fprintf( stderr, _("Unable to initialize CPP Parser.\n") );
          delete c;
          c=NULL;
          return( 1 );
	}
      if (c->execute())
        {
	  delete err_def;
	  if (cpp) delete cpp;
	  if (java) delete java;
	  delete c;
	  return( 1 );
	}
    }
  if (cpp) delete cpp;
  cpp=NULL;
  if (java) delete java;
  java=NULL;
  if (c) delete c;
  c=NULL;
  delete err_def;
  err_def=NULL;
  return( 0 );
}
