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

#include "errcpp.h"
#include <string.h>

CPP_Errors::CPP_Errors( char *out_base, Error_Definitions *err )
{
  installed=Init( out_base, err );
}

CPP_Errors::~CPP_Errors()
{
  if (out_h) fclose( out_h );
  out_h=NULL; 
  if (out_cpp) fclose( out_cpp );
  out_cpp=NULL;
  installed=0;
}

int CPP_Errors::execute()
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

int CPP_Errors::Init( char *out_base, Error_Definitions *err )
{
  errdef=NULL;
  out_h=NULL; 
  out_cpp=NULL;
  if (!err || !err->isOk()) return( 0 );
  if (strlen( out_base )>255) 
    {
      fprintf( stderr, "The base name is too long.\n" );
      return( 0 );
    }
  strcpy( base, out_base );
  errdef=err;
  return( 1 );  
}

int CPP_Errors::create_files()
{
  char cppname[255], hname[255];
  char *tmp;
  int count;
  tmp=strrchr( base, '/' );
  if (!tmp) 
    {
     strcpy( classname, base );
    }
  else 
    {
      strcpy( classname, tmp+1 );
    }
  strcat( classname, "_Errors" );
  strcpy( cppname, base );
  strcat( cppname, ".cpp" );
  strcpy( hname, base );
  strcat( hname, ".h" );
  out_cpp=fopen( cppname, "wt" );
  if (!out_cpp) 
    {
     fprintf( stderr, "Unable to create: %s\n", cppname );
     return( 1 );
    }
  out_h=fopen( hname, "wt" );
  if (!out_h) 
    { 
     fprintf( stderr, "Unable to create: %s\n", hname );
     fclose( out_cpp ); 
     out_cpp=NULL; 
     return( 1 ); 
    };
/*
"const int" is used for the constants below rather that "const unsigned"
as mightt expect given the  for compatibility with Java which does not
support unsigned. Using signed everywhere simplifies life.
*/
  fprintf( out_h, "#ifndef %s_H__\n#define %s_H__\n", classname, classname );
  fprintf( out_h, "\n\n/* This file has been automatically generated -- DO NOT EDIT */\n\n" );
  fprintf( out_h, "class %s\n{\npublic:\n  %s();\n  virtual ~%s();\n", classname, classname, classname );
  fprintf( out_h, "  static unsigned int ErrorCode_To_Name( int code, char *name, int len );\n" );
  fprintf( out_h, "  static unsigned int ErrorCode_To_Level( int code, int *level );\n" );
  fprintf( out_h, "  static unsigned int ErrorCode_To_Response( int code, int *response );\n" );
  fprintf( out_h, "  static unsigned int ErrorCode_To_Message( int code, char *message, int len );\n" );
  fprintf( out_h, "protected:\nprivate:\n};\n\n\n" );
  fprintf( out_h, "struct error_defs\n{\n int code;\n char *name;\n int level;\n int response;\n char *message;\n};\n\n" );
  for (count=0;count<NUM_VALID_LEVELS;count++)
    fprintf( out_h, "const int %s_LEVEL_%-20s        = 0x0%08X;\n", ERROR_PREFIX, valid_levels[count], count );
  fprintf( out_h, "\n" );
  for (count=0;count<NUM_VALID_RESPONSES;count++)
    fprintf( out_h, "const int %s_RESPONSE_%-20s     = 0x0%08X;\n", ERROR_PREFIX, valid_responses[count], count );
  fprintf( out_h, "\n\nconst int %s_NUM_ERROR           = %d;\n\n", ERROR_PREFIX, errdef->NumberOfErrors() );
  fprintf( out_cpp, "#include \"%s\"\n", hname );
  fprintf( out_cpp, "#include <string.h>\n\n" );
  fprintf( out_cpp, "\n\n/* This file has been automatically generated -- DO NOT EDIT */\n\n" );
  return( 0 );
}

int CPP_Errors::parse_errors()
{
  int cnt1, cnt2, result;
  int code, level, response;
  char *name, *message, *tmp;

  fprintf( out_cpp, "struct error_defs %sS[%d]=\n{\n", ERROR_PREFIX, errdef->NumberOfErrors() );
  for (cnt1=0;cnt1<errdef->NumberOfErrors();cnt1++)
    {
      code=errdef->Code( cnt1 );
      if (code==-1)
        {
	}
      name=errdef->Name( cnt1 );
      if (!name)
        {
	}
      tmp=errdef->Level( cnt1 );
      if (!tmp)
        {
	}
      response=-1;
      level=-1;
      for (cnt2=0;cnt2<NUM_VALID_LEVELS;cnt2++)
        if (!strcmp( tmp, valid_levels[cnt2] )) level=cnt2;
      if (level==-1)
        {
	}
      tmp=errdef->Response( cnt1 );
      if (!tmp)
        {
	}
      for (cnt2=0;cnt2<NUM_VALID_RESPONSES;cnt2++)
        if (!strcmp( tmp, valid_responses[cnt2] )) response=cnt2;
      if (response==-1)
        {
	}
      message=errdef->Message( cnt1 );
      if (!message)
        {
	}
      fprintf( out_h, "const int %s_%-20s         = 0x%08X;\n", ERROR_PREFIX, name, code );
      fprintf( out_cpp, "   { %s_%-20s, \"%s\", %d, %d, %s }", ERROR_PREFIX, name, name, level, response, message );
      if ((cnt1+1)<errdef->NumberOfErrors()) fprintf( out_cpp, ",\n" );
      else fprintf( out_cpp, "\n" );
    }
  fprintf( out_cpp, "};\n\n\n" );
  return( 0 );
}

int CPP_Errors::close_files()
{ 
  fprintf( out_h, "\n\n#endif\n" );
  fprintf( out_cpp, "%s::%s()\n{\n}\n\n", classname, classname );
  fprintf( out_cpp, "%s::~%s()\n{\n}\n\n", classname, classname );
  fprintf( out_cpp, "unsigned int %s::ErrorCode_To_Name( int code, char *name, int len )\n", classname );
  fprintf( out_cpp, "{\n int cnt;\n\n for (cnt=0;cnt<%s_NUM_ERROR;cnt++)\n    {\n    if (code==%sS[cnt].code)\n      {\n      if (strlen(%sS[cnt].name)+1>(unsigned)len) return true;\n      strcpy( name, %sS[cnt].name );\n      return false;\n      }\n    }\n  return true;\n}\n\n", ERROR_PREFIX, ERROR_PREFIX, ERROR_PREFIX, ERROR_PREFIX );
  fprintf( out_cpp, "unsigned int %s::ErrorCode_To_Level( int code, int *level )\n", classname );
  fprintf( out_cpp, "{\n int cnt;\n\n for (cnt=0;cnt<%s_NUM_ERROR;cnt++)\n    {\n    if (code==%sS[cnt].code)\n      {\n       *level=%sS[cnt].level;\n      return false;\n      }\n    }\n  return true;\n}\n\n", ERROR_PREFIX, ERROR_PREFIX, ERROR_PREFIX );
  fprintf( out_cpp, "unsigned int %s::ErrorCode_To_Response( int code, int *response )\n", classname );
  fprintf( out_cpp, "{\n int cnt;\n\n for (cnt=0;cnt<%s_NUM_ERROR;cnt++)\n    {\n    if (code==%sS[cnt].code)\n      {\n       *response=%sS[cnt].response;\n      return false;\n      }\n    }\n  return true;\n}\n\n", ERROR_PREFIX, ERROR_PREFIX, ERROR_PREFIX );  
  fclose( out_h );
  fprintf( out_cpp, "unsigned int %s::ErrorCode_To_Message( int code, char *message, int len )\n", classname );
  fprintf( out_cpp, "{\n int cnt;\n\n for (cnt=0;cnt<%s_NUM_ERROR;cnt++)\n    {\n    if (code==%sS[cnt].code)\n      {\n      if (strlen(%sS[cnt].message)+1>(unsigned)len) return true;\n      strcpy( message, %sS[cnt].message );\n      return false;\n      }\n    }\n  return true;\n}\n\n", ERROR_PREFIX, ERROR_PREFIX, ERROR_PREFIX, ERROR_PREFIX );  
  out_h=NULL;
  fclose( out_cpp );
  out_cpp=NULL;
  return( 0 );
}
