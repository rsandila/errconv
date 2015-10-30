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

#include "errc.h"
#include <string.h>
#ifdef __LOCAL__
#include <locale.h>
#include <libintl.h>

#define _(str) gettext(str)
#else
#define _(str) str
#endif
C_Errors::C_Errors( char *out_base, Error_Definitions *err )
{
  installed=Init( out_base, err );
}

C_Errors::~C_Errors()
{
  if (out_h) fclose( out_h );
  out_h=NULL; 
  if (out_c) fclose( out_c );
  out_c=NULL;
  installed=0;
}

int C_Errors::execute()
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

int C_Errors::Init( char *out_base, Error_Definitions *err )
{
  errdef=NULL;
  out_h=NULL; 
  out_c=NULL;
  if (!err || !err->isOk()) return( 0 );
  if (strlen( out_base )>255) 
    {
      fprintf( stderr, _("The base name is too long.\n") );
      return( 0 );
    }
  strcpy( base, out_base );
  errdef=err;
  return( 1 );  
}

int C_Errors::create_files()
{
  char cname[255], hname[255], classname[255];
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
  strcat( classname, _("_Errors") );
  strcpy( cname, base );
  strcat( cname, _(".c") );
  strcpy( hname, base );
  strcat( hname, _(".h") );
  out_c=fopen( cname, "wt" );
  if (!out_c) 
    {
     fprintf( stderr, _("Unable to create: %s\n"), cname );
     return( 1 );
    }
  out_h=fopen( hname, "wt" );
  if (!out_h) 
    { 
     fprintf( stderr, _("Unable to create: %s\n"), hname );
     fclose( out_c ); 
     out_c=NULL; 
     return( 1 ); 
    };
  fprintf( out_h, _("#ifndef %s_H__\n#define %s_H__\n"), classname, classname );
  fprintf( out_h, _("\n\n/* This file has been automatically generated -- DO NOT EDIT */\n\n") );
  fprintf( out_h, _("  int %sErrorCode_To_Name( int code, char *name, int len );\n"), ERROR_PREFIX );
  fprintf( out_h, _("  int %sErrorCode_To_Level( int code, int *level );\n"), ERROR_PREFIX );
  fprintf( out_h, _("  int %sErrorCode_To_Response( int code, int *response );\n"), ERROR_PREFIX );
  fprintf( out_h, _("  int %sErrorCode_To_Message( int code, char *message, int len );\n\n\n"), ERROR_PREFIX );
  fprintf( out_h, _("struct error_defs\n{\n int code;\n char *name;\n int level;\n int response;\n char *message;\n};\n\n") );
  for (count=0;count<NUM_VALID_LEVELS;count++)
    fprintf( out_h, _("#define %s_LEVEL_%-20s       0x0%08X\n"), ERROR_PREFIX, _(valid_levels[count]), count );
  fprintf( out_h, "\n" );
  for (count=0;count<NUM_VALID_RESPONSES;count++)
    fprintf( out_h, _("#define %s_RESPONSE_%-20s    0x0%08X\n"), ERROR_PREFIX, _(valid_responses[count]), count );
  fprintf( out_h, _("\n\n#define %s_NUM_ERROR          %d\n\n"), ERROR_PREFIX, errdef->NumberOfErrors() );
  fprintf( out_c, _("#include \"%s\"\n"), hname );
  fprintf( out_c, _("#include <string.h>\n\n") );
  fprintf( out_c, _("\n\n/* This file has been automatically generated -- DO NOT EDIT */\n\n") );
  return( 0 );
}

int C_Errors::parse_errors()
{
  int cnt1, cnt2, result;
  int code, level, response;
  char *name, *message, *tmp;

  fprintf( out_c, _("struct error_defs %sS[%d]=\n{\n"), ERROR_PREFIX, errdef->NumberOfErrors() );
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
        if (!strcmp( tmp, _(valid_levels[cnt2]) )) level=cnt2;
      if (level==-1)
        {
	}
      tmp=errdef->Response( cnt1 );
      if (!tmp)
        {
	}
      for (cnt2=0;cnt2<NUM_VALID_RESPONSES;cnt2++)
        if (!strcmp( tmp, _(valid_responses[cnt2]) )) response=cnt2;
      if (response==-1)
        {
	}
      message=errdef->Message( cnt1 );
      if (!message)
        {
	}
      fprintf( out_h, _("#define %s_%-20s        0x%08X\n"), ERROR_PREFIX, name, code );
      fprintf( out_c, _("   { %d, \"%s\", %d, %d, %s }"), code, name, level, response, message );
      if ((cnt1+1)<errdef->NumberOfErrors()) fprintf( out_c, ",\n" );
      else fprintf( out_c, "\n" );
    }
  fprintf( out_c, "};\n\n\n" );
  return( 0 );
}

int C_Errors::close_files()
{ 
  fprintf( out_h, _("\n\n#endif\n") );
  fprintf( out_c, _("int %sErrorCode_To_Name( int code, char *name, int len )\n"), ERROR_PREFIX );
  fprintf( out_c, _("{\n int cnt;\n\n for (cnt=0;cnt<%s_NUM_ERROR;cnt++)\n    {\n    if (code==%sS[cnt].code)\n      {\n      if (strlen(%sS[cnt].name)+1>len) return( 1 );\n      strcpy( name, %sS[cnt].name );\n      return( 0 );\n      }\n    }\n  return(1);\n}\n\n"), ERROR_PREFIX, ERROR_PREFIX, ERROR_PREFIX, ERROR_PREFIX );
  fprintf( out_c, _("int %sErrorCode_To_Level( int code, int *level )\n"), ERROR_PREFIX );
  fprintf( out_c, _("{\n int cnt;\n\n for (cnt=0;cnt<%s_NUM_ERROR;cnt++)\n    {\n    if (code==%sS[cnt].code)\n      {\n       *level=%sS[cnt].level;\n      return( 0 );\n      }\n    }\n  return(1);\n}\n\n"), ERROR_PREFIX, ERROR_PREFIX, ERROR_PREFIX );
  fprintf( out_c, _("int %sErrorCode_To_Response( int code, int *response )\n"), ERROR_PREFIX );
  fprintf( out_c, _("{\n int cnt;\n\n for (cnt=0;cnt<%s_NUM_ERROR;cnt++)\n    {\n    if (code==%sS[cnt].code)\n      {\n       *response=%sS[cnt].response;\n      return( 0 );\n      }\n    }\n  return(1);\n}\n\n"), ERROR_PREFIX, ERROR_PREFIX, ERROR_PREFIX );  
  fclose( out_h );
  fprintf( out_c, _("int %sErrorCode_To_Message( int code, char *message, int len )\n"), ERROR_PREFIX );
  fprintf( out_c, _("{\n int cnt;\n\n for (cnt=0;cnt<%s_NUM_ERROR;cnt++)\n    {\n    if (code==%sS[cnt].code)\n      {\n      if (strlen(%sS[cnt].message)+1>len) return( 1 );\n      strcpy( message, %sS[cnt].message );\n      return( 0 );\n      }\n    }\n  return(1);\n}\n\n"), ERROR_PREFIX, ERROR_PREFIX, ERROR_PREFIX, ERROR_PREFIX );  
  out_h=NULL;
  fclose( out_c );
  out_c=NULL;
  return( 0 );
}
