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

#include "errjava.h"
#include <string.h>
#include <ctype.h>

Java_Errors::Java_Errors( char *out_base, Error_Definitions *err )
{
  installed=Init( out_base, err );
}

Java_Errors::~Java_Errors() 
{
  if (out_level) fclose( out_level );
  out_level=NULL;
  if (out_response) fclose( out_response );
  out_response=NULL;
  if (out_error) fclose( out_error );
  out_error=NULL;
  installed=0;
  errdef=NULL;
}

int Java_Errors::execute()
{
  int result;
  if (!isOk()) 
    {
      fprintf( stderr, "Class did not initialize properly.\n" );
      return( 1 );
    }
  result=create_files();
  if (result) return( result );
  result=parse_errors();
  if (result) { close_files(); return( result ); }
  return( close_files() );
}

int Java_Errors::Init( char *out_base, Error_Definitions *err )
{
  out_level=NULL;
  out_response=NULL;
  out_error=NULL;
  errdef=NULL;
  if (!err || !out_base || !err->isOk()) return( 0 );
  if (strlen( out_base )+1>255) 
   {
     fprintf( stderr, "The base name is too long.\n" );
    return( 0 );
   }
  strcpy( base, out_base );
  errdef=err;
  return( 1 );
}

int Java_Errors::create_files()
{
  char levelname[255], responsename[255], errorname[255], classbase[255];
  char *tmp;
  int count;
  tmp=strrchr( base, '/' );
  if (!tmp) 
    {
     strcpy( class_error, base );
     strcpy( class_level, base );
     strcpy( class_response, base );
     strcpy( classbase, base );
    }
  else 
    {
      strcpy( class_error, tmp+1 );
      strcpy( class_level, tmp+1 );
      strcpy( class_response, tmp+1 );
      strcpy( classbase, tmp+1 );
    }
  tmp=classbase;
  while (*tmp) { *tmp=tolower( *tmp ); tmp+=1; };
  strcat( class_error, "Error" );
  strcat( class_level, "ImpLevel" );
  strcat( class_response, "Response" );
  strcpy( errorname, base );
  strcat( errorname, "Error.java" );
  strcpy( levelname, base );
  strcat( levelname, "ImpLevel.java" );
  strcpy( responsename, base );
  strcat( responsename, "Response.java" );
  out_error=fopen( errorname, "wt" );
  if (!out_error) 
    {
     fprintf( stderr, "Unable to create: %s\n", errorname );
     return( 1 );
    }
  out_level=fopen( levelname, "wt" );
  if (!out_level) 
    { 
     fprintf( stderr, "Unable to create: %s\n", levelname );
     fclose( out_error ); 
     out_error=NULL; 
     return( 1 ); 
    };
  out_response=fopen( responsename, "wt" );
  if (!out_response) 
    { 
     fprintf( stderr, "Unable to create: %s\n", responsename );
     fclose( out_error ); 
     fclose( out_level );
     out_error=NULL; 
     out_level=NULL;
     return( 1 ); 
    }; 
  fprintf( out_response, "package %s;\n\n/*\n This code has been automatically generated -- DO NOT EDIT\n*/\n\n", classbase );
  fprintf( out_response, "public class %s {\n", class_response );
  for (count=0;count<NUM_VALID_RESPONSES;count++)
    fprintf( out_response, "    public static final int %-20s = %d;\n", valid_responses[count], count );
  fprintf( out_response, "\n\n    private final void ErrorResponse() {}\n}\n" );
  
  fprintf( out_level, "package %s;\n\n/*\n This code has been automatically generated -- DO NOT EDIT\n*/\n\n", classbase );
  fprintf( out_level, "public class %s {\n", class_level );
  for (count=0;count<NUM_VALID_LEVELS;count++)
    fprintf( out_level, "    public static final int %-20s = %d;\n", valid_levels[count], count );
  fprintf( out_level, "\n\n    private final void ErrorImpLevel() {}\n}\n" );
  
  fprintf( out_error, "package %s;\n\nimport java.util.HashMap;\nimport commoncode.*;\n\n/*\n This code has been automatically generated -- DO NOT EDIT\n*/\n\n", classbase );
  fprintf( out_error, "public class %s {\n    private static HashMap errHash=null;\n", class_error );
  return( 0 );
}

int Java_Errors::parse_errors()
{
  int count;
  for (count=0;count<errdef->NumberOfErrors();count++)
    {
      fprintf( out_error, "    public static final int %-20s=%d;\n", errdef->Name( count ), errdef->Code( count ) );      
    }
  fprintf( out_error, "\n\npublic static void initHashMap() {\n try {\n    errHash=new HashMap( 150 );\n" );
  for (count=0;count<errdef->NumberOfErrors();count++)
    {
      fprintf( out_error, "    errHash.put(new Integer(%s), new ErrorObj(%s.%s, %s.%s, new String(%s)));\n",
	       errdef->Name( count ), class_level, errdef->Level( count ), 
               class_response, errdef->Response( count ), errdef->Message( count ) );
    }
  return( 0 );
}

int Java_Errors::close_files()
{
  fclose( out_response );
  out_response=NULL; 
  fclose( out_level );
  out_level=NULL;
  fprintf( out_error, "  } catch (Exception e) {\n    errHash=null;\n  }\n}\n" );
  fprintf( out_error, "\n\npublic static String getErrorString(int errcode) {\n  try {\n     return ((ErrorObj)(errHash.get(new Integer(errcode)))).description;\n  }\n catch (Exception e) {\n     return new String( \"Unknown Error Occured\" );\n  }\n}\n\n\nprivate %s() {\n }\n}\n", class_error );
  fclose( out_error );
  out_error=NULL;
  return( 0 );
}
