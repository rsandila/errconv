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
#include <libintl.h>
#include <locale.h>

#define _(str) gettext(str)
#else
#define _(str) str
#endif
CPP_Errors::CPP_Errors(const std::string &out_base,
                       const Error_Definitions &err)
    : errdef(err), base(out_base) {
  installed = 1;
}

CPP_Errors::~CPP_Errors() {
  installed = 0;
}

int CPP_Errors::execute() {
  if (!isOk()) {
    fprintf(stderr, _("Class did not initialize properly.\n"));
    return (1);
  }
  int result = create_files();
  if (result)
    return (result);
  result = parse_errors();
  if (result) {
    close_files();
    return (result);
  }
  return (close_files());
}

int CPP_Errors::create_files() {
  std::string cppname, hname;
  char *tmp;
  tmp = strrchr((char *)base.c_str(), '/');
  if (!tmp) {
    classname = base;
  } else {
    classname = tmp + 1;
  }
  classname += std::string(_("_Errors"));
  cppname = base + std::string(_(".cpp"));
  hname = base + std::string(_(".h"));
  out_cpp.reset(fopen(cppname.c_str(), "wt"));
  if (!out_cpp.get()) {
    fprintf(stderr, _("Unable to create: %s\n"), cppname.c_str());
    return (1);
  }
  out_h.reset(fopen(hname.c_str(), "wt"));
  if (!out_h.get()) {
    fprintf(stderr, _("Unable to create: %s\n"), hname.c_str());
    return (1);
  };
  /*
  "const int" is used for the constants below rather that "const unsigned"
  as might be expect given the compatibility with Java which does not
  support unsigned. Using signed everywhere simplifies life.
  */
  fprintf(out_h.get(), _("#pragma once"));
  fprintf(out_h.get(), _("\n\n/* This file has been automatically generated -- DO "
                   "NOT EDIT */\n\n"));
  fprintf(out_h.get(), _("class %s\n{\npublic:\n  %s();\n  virtual ~%s();\n"),
          classname.c_str(), classname.c_str(), classname.c_str());
  fprintf(out_h.get(), _("  static unsigned int ErrorCode_To_Name( int code, char "
                   "*name, int len );\n"));
  fprintf(
      out_h.get(),
      _("  static unsigned int ErrorCode_To_Level( int code, int *level );\n"));
  fprintf(out_h.get(), _("  static unsigned int ErrorCode_To_Response( int code, int "
                   "*response );\n"));
  fprintf(out_h.get(), _("  static unsigned int ErrorCode_To_Message( int code, char "
                   "*message, int len );\n"));
  fprintf(out_h.get(), _("protected:\nprivate:\n};\n\n\n"));
  fprintf(out_h.get(), _("struct error_defs\n{\n int code;\n char *name;\n int "
                   "level;\n int response;\n char *message;\n};\n\n"));
  for (int count = 0; count < NUM_VALID_LEVELS; count++)
    fprintf(out_h.get(), _("const int %s_LEVEL_%-20s        = 0x0%08X;\n"),
            ERROR_PREFIX, _(valid_levels[count].c_str()), count);
  fprintf(out_h.get(), "\n");
  for (int count = 0; count < NUM_VALID_RESPONSES; count++)
    fprintf(out_h.get(), _("const int %s_RESPONSE_%-20s     = 0x0%08X;\n"),
            ERROR_PREFIX, _(valid_responses[count].c_str()), count);
  fprintf(out_h.get(), _("\n\nconst int %s_NUM_ERROR           = %d;\n\n"),
          ERROR_PREFIX, errdef.NumberOfErrors());
  fprintf(out_cpp.get(), "#include \"%s\"\n", hname.c_str());
  fprintf(out_cpp.get(), "#include <string.h>\n\n");
  fprintf(out_cpp.get(), _("\n\n/* This file has been automatically generated -- DO "
                     "NOT EDIT */\n\n"));
  return (0);
}

int CPP_Errors::parse_errors() {
  fprintf(out_cpp.get(), _("struct error_defs %sS[%d]=\n{\n"), ERROR_PREFIX,
          errdef.NumberOfErrors());
  for (int cnt1 = 0; cnt1 < errdef.NumberOfErrors(); cnt1++) {
    int code = errdef.Code(cnt1);
    if (code == -1) {
    }
    std::string name = errdef.Name(cnt1);
    std::string tmp = errdef.Level(cnt1);
    int response = -1;
    int level = -1;
    for (int cnt2 = 0; cnt2 < NUM_VALID_LEVELS; cnt2++)
      if (tmp == _(valid_levels[cnt2].c_str()))
        level = cnt2;
    if (level == -1) {
    }
    tmp = errdef.Response(cnt1);
    for (int cnt2 = 0; cnt2 < NUM_VALID_RESPONSES; cnt2++)
      if (tmp == _(valid_responses[cnt2].c_str()))
        response = cnt2;
    if (response == -1) {
    }
    std::string message = errdef.Message(cnt1);
    fprintf(out_h.get(), _("const int %s_%-20s         = 0x%08X;\n"), ERROR_PREFIX,
            name.c_str(), code);
    fprintf(out_cpp.get(), _("   { %s_%-20s, \"%s\", %d, %d, %s }"), ERROR_PREFIX,
            name.c_str(), name.c_str(), level, response, message.c_str());
    if ((cnt1 + 1) < errdef.NumberOfErrors())
      fprintf(out_cpp.get(), ",\n");
    else
      fprintf(out_cpp.get(), "\n");
  }
  fprintf(out_cpp.get(), "};\n\n\n");
  return (0);
}

int CPP_Errors::close_files() {
  fprintf(out_cpp.get(), _("%s::%s()\n{\n}\n\n"), classname.c_str(), classname.c_str());
  fprintf(out_cpp.get(), _("%s::~%s()\n{\n}\n\n"), classname.c_str(), classname.c_str());
  fprintf(out_cpp.get(),
          _("unsigned int %s::ErrorCode_To_Name( int code, char *name, int len "
            ")\n"),
          classname.c_str());
  fprintf(out_cpp.get(),
          _("{\n int cnt;\n\n for (cnt=0;cnt<%s_NUM_ERROR;cnt++)\n    {\n    "
            "if (code==%sS[cnt].code)\n      {\n      if "
            "(strlen(%sS[cnt].name)+1>(unsigned)len) return 1;\n      strcpy( "
            "name, %sS[cnt].name );\n      return 0;\n      }\n    }\n  return "
            "1;\n}\n\n"),
          ERROR_PREFIX, ERROR_PREFIX, ERROR_PREFIX, ERROR_PREFIX);
  fprintf(out_cpp.get(),
          _("unsigned int %s::ErrorCode_To_Level( int code, int *level )\n"),
          classname.c_str());
  fprintf(
      out_cpp.get(),
      _("{\n int cnt;\n\n for (cnt=0;cnt<%s_NUM_ERROR;cnt++)\n    {\n    if "
        "(code==%sS[cnt].code)\n      {\n       *level=%sS[cnt].level;\n      "
        "return 0;\n      }\n    }\n  return 1;\n}\n\n"),
      ERROR_PREFIX, ERROR_PREFIX, ERROR_PREFIX);
  fprintf(
      out_cpp.get(),
      _("unsigned int %s::ErrorCode_To_Response( int code, int *response )\n"),
      classname.c_str());
  fprintf(
      out_cpp.get(),
      _("{\n int cnt;\n\n for (cnt=0;cnt<%s_NUM_ERROR;cnt++)\n    {\n    if "
        "(code==%sS[cnt].code)\n      {\n       *response=%sS[cnt].response;\n "
        "     return 0;\n      }\n    }\n  return 1;\n}\n\n"),
      ERROR_PREFIX, ERROR_PREFIX, ERROR_PREFIX);
  fprintf(out_cpp.get(),
          _("unsigned int %s::ErrorCode_To_Message( int code, char *message, "
            "int len )\n"),
          classname.c_str());
  fprintf(out_cpp.get(),
          _("{\n int cnt;\n\n for (cnt=0;cnt<%s_NUM_ERROR;cnt++)\n    {\n    "
            "if (code==%sS[cnt].code)\n      {\n      if "
            "(strlen(%sS[cnt].message)+1>(unsigned)len) return 1;\n      "
            "strcpy( message, %sS[cnt].message );\n      return 0;\n      }\n  "
            "  }\n  return 1;\n}\n\n"),
          ERROR_PREFIX, ERROR_PREFIX, ERROR_PREFIX, ERROR_PREFIX);
  return (0);
}
