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

#include "errdef.h"
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <libintl.h>

#define _(str) gettext(str)

//! This defines the valid values that the error level parameter can take
char *valid_levels[NUM_VALID_LEVELS]=
{
  "DEBUG",
  "INFO",
  "NOTICE",
  "WARNING",
  "ERR",
  "CRIT",
  "ALERT",
  "EMERG"
};

//! this defines the valid values that the error response parameter can take
char *valid_responses[NUM_VALID_RESPONSES]=
{
  "IGNORE",
  "ALERT",
  "FALLOVER",
  "RETRY",
  "ABORT",
  "PANIC"
};

Error_Definitions::Error_Definitions( char *file_name )
{
  installed=Init( file_name );
}

Error_Definitions::~Error_Definitions()
{
  int count;
  for (count=0;count<NumberOfErrors();count++)
    {
      if (error_names && error_names[count]) delete []error_names[count];
      if (levels && levels[count]) delete []levels[count];
      if (responses && responses[count]) delete []responses[count];
      if (messages && messages[count]) delete []messages[count];
    }
  if (error_names) delete []error_names;
  if (error_codes) delete []error_codes;
  if (levels) delete []levels;
  if (responses) delete []responses;
  if (messages) delete []messages;
  error_names=NULL;
  error_codes=NULL;
  levels=NULL;
  responses=NULL; 
  messages=NULL;
  installed=0;
}

int Error_Definitions::Init( char *file_name )
{
  FILE *inp;
  char buf[2048];
  int count, result;
  num_errors=0;
  error_names=NULL;
  error_codes=NULL;
  levels=NULL;
  responses=NULL; 
  messages=NULL;
  if (access( file_name, R_OK )) 
   {
     fprintf( stderr, _("No read access to %s\n"), file_name );
     return( 0 );
   }
  inp=fopen( file_name, "rt" );
  if (!inp)
    {
      fprintf( stderr, _("Unable to open %s\n"), file_name );
      return( 0 );
    }
  num_errors=count_lines( inp );
  if (!num_errors)
    {
      fprintf( stderr, _("Unable to determine length of file: %s\n"), file_name );
      return( 0 );
    }
  printf( _("File %s contains %d lines.\n"), file_name, num_errors );
  error_names=new char *[num_errors+1];
  error_codes=new int[num_errors+1];
  levels=new char *[num_errors+1];
  responses=new char *[num_errors+1]; 
  messages=new char *[num_errors+1];
  if (!error_names || !error_codes || !levels || !responses || !messages)
    {
      fprintf( stderr, _("Out of memory allocating objects.\n") );
      return( 0 );
    }
  memset( error_names, 0, sizeof(char *)*(num_errors+1) );
  memset( levels, 0, sizeof(char *)*(num_errors+1) );
  memset( responses, 0, sizeof(char *)*(num_errors+1) );
  memset( messages, 0, sizeof(char *)*(num_errors+1) );
  memset( error_codes, 0, sizeof(int)*(num_errors+1) );
  count=0;
  while (fgets( buf, 2048, inp ) )
    {
      if (count>=num_errors)
        {
	  fprintf( stderr, _("File seems to have grown???\n") );
          return( 0 );
	}
      result=parse_line( count, buf );
      if (result<0)
        { 
	  return( 0 );
        }
      if (result) count+=1;        
    }
  if (count<num_errors)
    {
      printf( _("Warning: Empty or invalid lines detected.\n") );
      num_errors=count;
    }
  fclose( inp );
  return( 1 );
}

int Error_Definitions::parse_line( int count, char *buf )
{
  char *tok;
  int cnt1, cnt2;
  char name[255], response[255], message[255], level[255];
  int code;

  if (!buf) return( -1 );
  if (buf[0]=='\n' || buf[0]=='\r') return( 0 );
  tok=strtok( buf, ":" );
  if (!tok)
    {
      fprintf( stderr, _("Invalid formatted string: %d\n"), count );
      return( -1 );
    }
  strcpy( name, tok );
  strup( name );
  for (cnt1=0;cnt1<count;cnt1++)
    if (!strcmp( error_names[cnt1], name ))
      {
	fprintf( stderr, _("Entries %d and %d have identical names: %s\n"), cnt1, count, name );
        return( -1 );
      }
  tok=strtok( NULL, ":" );
  if (!tok)
    {
      fprintf( stderr, _("Invalid formatted string: %d\n"), count );
      return( -1 );
    }
  code=atoi( tok );
  for (cnt1=0;cnt1<count;cnt1++)
    if (code==error_codes[cnt1])
      {
	fprintf( stderr, _("Entries %d and %d have identical error codes.\n"), cnt1, count );
        return( -1 );
      }
  tok=strtok( NULL, ":" );
  if (!tok)
    {
      fprintf( stderr, _("Invalid formatted string: %d\n"), count );
      return( -1 );
    }
  strcpy( level, tok );
  strup( level );
  cnt2=0;
  for (cnt1=0;cnt1<NUM_VALID_LEVELS;cnt1++)
    if (!strcmp( level, _(valid_levels[cnt1]) )) cnt2+=1;
  if (!cnt2)
    {
      fprintf( stderr, _("Invalid level %s specified in line %d\n"), level, count );
      return( -1 );
    }
  tok=strtok( NULL, ":" );
  if (!tok)
    {
      fprintf( stderr, _("Invalid formatted string: %d\n"), count );
      return( -1 );
    }
  strcpy( response, tok );
  strup( response );
  cnt2=0;
  for (cnt1=0;cnt1<NUM_VALID_RESPONSES;cnt1++)
    if (!strcmp( response, _(valid_responses[cnt1]) )) cnt2+=1;
  if (!cnt2)
    {
      fprintf( stderr, _("Invalid response %s specified in line %d\n"), response, count );
      return( -1 );
    }  
  tok=strtok( NULL, ":" );
  if (!tok)
    {
      fprintf( stderr, _("Invalid formatted string: %d\n"), count );
      return( -1 );
    }
  strcpy( message, tok );
  if (message[strlen(message)-1]=='\n') message[strlen(message)-1]=0;
  error_names[count]=new char[strlen(name)+1];
  levels[count]=new char[strlen(level)+1];
  responses[count]=new char[strlen(response)+1];
  messages[count]=new char[strlen(message)+1];
  if (!error_names[count] || !levels[count] || !responses[count] || !messages[count])
    {
      fprintf( stderr, _("Out of memory.\n") );
      return( -1 );
    }
  error_codes[count]=code;
  strcpy( error_names[count], name );
  strcpy( levels[count], level );
  strcpy( responses[count], response );
  strcpy( messages[count], message );  
  return( 1 );
}

void Error_Definitions::strup( char *buf )
{
  char *p;
  if (!buf) return;
  p=buf;
  while (*p) 
   { 
    *p=toupper( *p ); 
    p+=1;
   };
}

int Error_Definitions::count_lines( FILE * inp )
{
  long pos;
  int count;
  char buf[2048];

  pos=ftell( inp );
  count=0;
  while (fgets( buf, 2048, inp )) count+=1;
  fseek( inp, pos, SEEK_SET );
  return( count );
}

char *Error_Definitions::Message( int index )
{
  if (!isOk() || index<0 || index>=NumberOfErrors()) return( NULL );
  return( messages[index] );
}

char *Error_Definitions::Level( int index )
{
  if (!isOk() || index<0 || index>=NumberOfErrors()) return( NULL );
  return( levels[index] );
}

char *Error_Definitions::Response( int index )
{
  if (!isOk() || index<0 || index>=NumberOfErrors()) return( NULL );
  return( responses[index] );
}

char *Error_Definitions::Name( int index )
{
  if (!isOk() || index<0 || index>=NumberOfErrors()) return( NULL );
  return( error_names[index] );
}

int Error_Definitions::Code( int index )
{
  if (!isOk() || index<0 || index>=NumberOfErrors()) return( -1 );
  return( error_codes[index] );
}
