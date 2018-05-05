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
#ifdef __LOCAL__
#include <locale.h>
#include <libintl.h>

#define _(str) gettext(str)
#else
#define _(str) str
#endif
CPP_Errors::CPP_Errors( const std::string & out_base, const Error_Definitions & err  ) : errdef(err), base(out_base)
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
      fprintf( stderr, _("Class did not initialize properly.\n") );
      return( 1 );
    }
  result=create_files();
  if (result) return( result );
  result=parse_errors();
  if (result) { close_files(); return( result ); }
  return( close_files() );
}

int CPP_Errors::Init( const std::string & out_base, const Error_Definitions & err  )
{
  out_h=NULL;
  out_cpp=NULL;
  return( 1 );
}

int CPP_Errors::create_files()
{
  char cppname[255], hname[255];
  char *tmp;
  int count;
  tmp=strrchr( (char *)base.c_str(), '/' );
  if (!tmp)
    {
     strcpy( classname, base.c_str() );
    }
  else
    {
      strcpy( classname, tmp+1 );
    }
  strcat( classname, _("_Errors") );
  strcpy( cppname, base.c_str() );
  strcat( cppname, _(".cpp") );
  strcpy( hname, base.c_str() );
  strcat( hname, _(".h") );
  out_cpp=fopen( cppname, "wt" );
  if (!out_cpp)
    {
     fprintf( stderr, _("Unable to create: %s\n"), cppname );
     return( 1 );
    }
  out_h=fopen( hname, "wt" );
  if (!out_h)
    {
     fprintf( stderr, _("Unable to create: %s\n"), hname );
     fclose( out_cpp );
     out_cpp=NULL;
     return( 1 );
    };
/*
"const int" is used for the constants below rather that "const unsigned"
as mightt expect given the  for compatibility with Java which does not
support unsigned. Using signed everywhere simplifies life.
*/
  fprintf( out_h, _("#ifndef %s_H__\n#define %s_H__\n"), classname, classname );
  fprintf( out_h, _("\n\n/* This file has been automatically generated -- DO NOT EDIT */\n\n") );
  fprintf( out_h, _("class %s\n{\npublic:\n  %s();\n  virtual ~%s();\n"), classname, classname, classname );
  fprintf( out_h, _("  static unsigned int ErrorCode_To_Name( int code, char *name, int len );\n") );
  fprintf( out_h, _("  static unsigned int ErrorCode_To_Level( int code, int *level );\n") );
  fprintf( out_h, _("  static unsigned int ErrorCode_To_Response( int code, int *response );\n") );
  fprintf( out_h, _("  static unsigned int ErrorCode_To_Message( int code, char *message, int len );\n") );
  fprintf( out_h, _("protected:\nprivate:\n};\n\n\n") );
  fprintf( out_h, _("struct error_defs\n{\n int code;\n char *name;\n int level;\n int response;\n char *message;\n};\n\n") );
  for (count=0;count<NUM_VALID_LEVELS;count++)
    fprintf( out_h, _("const int %s_LEVEL_%-20s        = 0x0%08X;\n"), ERROR_PREFIX, _(valid_levels[count].c_str()), count );
  fprintf( out_h, "\n" );
  for (count=0;count<NUM_VALID_RESPONSES;count++)
    fprintf( out_h, _("const int %s_RESPONSE_%-20s     = 0x0%08X;\n"), ERROR_PREFIX, _(valid_responses[count].c_str()), count );
  fprintf( out_h, _("\n\nconst int %s_NUM_ERROR           = %d;\n\n"), ERROR_PREFIX, errdef.NumberOfErrors() );
  fprintf( out_cpp, "#include \"%s\"\n", hname );
  fprintf( out_cpp, "#include <string.h>\n\n" );
  fprintf( out_cpp, _("\n\n/* This file has been automatically generated -- DO NOT EDIT */\n\n") );
  return( 0 );
}

int CPP_Errors::parse_errors()
{
  int cnt1, cnt2, result;
  int code, level, response;
  std::string name, message, tmp;

  fprintf( out_cpp, _("struct error_defs %sS[%d]=\n{\n"), ERROR_PREFIX, errdef.NumberOfErrors() );
  for (cnt1=0;cnt1<errdef.NumberOfErrors();cnt1++)
  {
    code=errdef.Code( cnt1 );
    if (code==-1)
    {
	  }
    name=errdef.Name( cnt1 );
    tmp=errdef.Level( cnt1 );
    response=-1;
    level=-1;
    for (cnt2=0;cnt2<NUM_VALID_LEVELS;cnt2++)
    if (tmp == _(valid_levels[cnt2].c_str()) ) level=cnt2;
    if (level==-1)
    {
	  }
    tmp=errdef.Response( cnt1 );
    for (cnt2=0;cnt2<NUM_VALID_RESPONSES;cnt2++)
        if (tmp == _(valid_responses[cnt2].c_str()) ) response=cnt2;
    if (response==-1)
    {
	  }
    message=errdef.Message( cnt1 );
    fprintf( out_h, _("const int %s_%-20s         = 0x%08X;\n"), ERROR_PREFIX, name.c_str(), code );
    fprintf( out_cpp, _("   { %s_%-20s, \"%s\", %d, %d, %s }"), ERROR_PREFIX, name.c_str(), name.c_str(), level, response, message.c_str() );
    if ((cnt1+1)<errdef.NumberOfErrors()) fprintf( out_cpp, ",\n" );
    else fprintf( out_cpp, "\n" );
  }
  fprintf( out_cpp, "};\n\n\n" );
  return( 0 );
}

int CPP_Errors::close_files()
{
  fprintf( out_h, _("\n\n#endif\n") );
  fprintf( out_cpp, _("%s::%s()\n{\n}\n\n"), classname, classname );
  fprintf( out_cpp, _("%s::~%s()\n{\n}\n\n"), classname, classname );
  fprintf( out_cpp, _("unsigned int %s::ErrorCode_To_Name( int code, char *name, int len )\n"), classname );
  fprintf( out_cpp, _("{\n int cnt;\n\n for (cnt=0;cnt<%s_NUM_ERROR;cnt++)\n    {\n    if (code==%sS[cnt].code)\n      {\n      if (strlen(%sS[cnt].name)+1>(unsigned)len) return 1;\n      strcpy( name, %sS[cnt].name );\n      return 0;\n      }\n    }\n  return 1;\n}\n\n"), ERROR_PREFIX, ERROR_PREFIX, ERROR_PREFIX, ERROR_PREFIX );
  fprintf( out_cpp, _("unsigned int %s::ErrorCode_To_Level( int code, int *level )\n"), classname );
  fprintf( out_cpp, _("{\n int cnt;\n\n for (cnt=0;cnt<%s_NUM_ERROR;cnt++)\n    {\n    if (code==%sS[cnt].code)\n      {\n       *level=%sS[cnt].level;\n      return 0;\n      }\n    }\n  return 1;\n}\n\n"), ERROR_PREFIX, ERROR_PREFIX, ERROR_PREFIX );
  fprintf( out_cpp, _("unsigned int %s::ErrorCode_To_Response( int code, int *response )\n"), classname );
  fprintf( out_cpp, _("{\n int cnt;\n\n for (cnt=0;cnt<%s_NUM_ERROR;cnt++)\n    {\n    if (code==%sS[cnt].code)\n      {\n       *response=%sS[cnt].response;\n      return 0;\n      }\n    }\n  return 1;\n}\n\n"), ERROR_PREFIX, ERROR_PREFIX, ERROR_PREFIX );
  fclose( out_h );
  fprintf( out_cpp, _("unsigned int %s::ErrorCode_To_Message( int code, char *message, int len )\n"), classname );
  fprintf( out_cpp, _("{\n int cnt;\n\n for (cnt=0;cnt<%s_NUM_ERROR;cnt++)\n    {\n    if (code==%sS[cnt].code)\n      {\n      if (strlen(%sS[cnt].message)+1>(unsigned)len) return 1;\n      strcpy( message, %sS[cnt].message );\n      return 0;\n      }\n    }\n  return 1;\n}\n\n"), ERROR_PREFIX, ERROR_PREFIX, ERROR_PREFIX, ERROR_PREFIX );
  out_h=NULL;
  fclose( out_cpp );
  out_cpp=NULL;
  return( 0 );
}
