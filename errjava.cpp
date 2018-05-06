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
#include <libintl.h>
#include <locale.h>
#endif
#include "errjava.h"
#include <ctype.h>
#include <string.h>

#ifdef __LOCAL__
#define _(str) gettext(str)
#else
#define _(str) str
#endif

Java_Errors::Java_Errors(const std::string &out_base,
                         const Error_Definitions &err)
    : errdef(err), base(out_base) {
  installed = 1;
}

Java_Errors::~Java_Errors() {
  installed = 0;
}

int Java_Errors::execute() {
  int result;
  if (!isOk()) {
    fprintf(stderr, _("Class did not initialize properly.\n"));
    return (1);
  }
  result = create_files();
  if (result)
    return (result);
  result = parse_errors();
  if (result) {
    close_files();
    return (result);
  }
  return (close_files());
}

int Java_Errors::create_files() {
  std::string levelname, responsename, errorname, classbase;
  char *tmp;
  tmp = strrchr((char *)base.c_str(), '/');
  if (!tmp) {
    class_error = base;
    class_level = base;
    class_response = base;
    classbase = base;
  } else {
    class_error = (tmp + 1);
    class_level = (tmp + 1);
    class_response = (tmp + 1);
    classbase = (tmp + 1);
  }
  std::transform(classbase.begin(), classbase.end(), classbase.begin(), ::tolower);
  class_error += std::string(_("Error"));
  class_level += std::string(_("ImpLevel"));
  class_response += std::string(_("Response"));
  errorname = base + std::string(_("Error.java"));
  levelname = base + std::string(_("ImpLevel.java"));
  responsename = base + std::string(_("Response.java"));

  out_error.reset(fopen(errorname.c_str(), "wt"));
  if (!out_error.get()) {
    fprintf(stderr, _("Unable to create: %s\n"), errorname.c_str());
    return (1);
  }
  out_level.reset(fopen(levelname.c_str(), "wt"));
  if (!out_level.get()) {
    fprintf(stderr, _("Unable to create: %s\n"), levelname.c_str());
    return (1);
  };
  out_response.reset(fopen(responsename.c_str(), "wt"));
  if (!out_response.get()) {
    fprintf(stderr, _("Unable to create: %s\n"), responsename.c_str());
    return (1);
  };
  fprintf(out_response.get(),
          _("package %s;\n\n/*\n This code has been automatically generated -- "
            "DO NOT EDIT\n*/\n\n"),
          classbase.c_str());
  fprintf(out_response.get(), _("public class %s {\n"), class_response.c_str());
  for (int count = 0; count < NUM_VALID_RESPONSES; count++)
    fprintf(out_response.get(), _("    public static final int %-20s = %d;\n"),
            _(valid_responses[count].c_str()), count);
  fprintf(out_response.get(),
          _("\n\n    private final void ErrorResponse() {}\n}\n"));

  fprintf(out_level.get(),
          _("package %s;\n\n/*\n This code has been automatically generated -- "
            "DO NOT EDIT\n*/\n\n"),
          classbase.c_str());
  fprintf(out_level.get(), _("public class %s {\n"), class_level.c_str());
  for (int count = 0; count < NUM_VALID_LEVELS; count++)
    fprintf(out_level.get(), _("    public static final int %-20s = %d;\n"),
            _(valid_levels[count].c_str()), count);
  fprintf(out_level.get(), _("\n\n    private final void ErrorImpLevel() {}\n}\n"));

  fprintf(out_error.get(),
          _("package %s;\n\nimport java.util.HashMap;\nimport "
            "commoncode.*;\n\n/*\n This code has been automatically generated "
            "-- DO NOT EDIT\n*/\n\n"),
          classbase.c_str());
  fprintf(out_error.get(),
          _("public class %s {\n    private static HashMap errHash=null;\n"),
          class_error.c_str());
  return (0);
}

int Java_Errors::parse_errors() {
  for (int count = 0; count < errdef.NumberOfErrors(); count++) {
    fprintf(out_error.get(), _("    public static final int %-20s=%d;\n"),
            errdef.Name(count).c_str(), errdef.Code(count));
  }
  fprintf(out_error.get(), _("\n\npublic static void initHashMap() {\n try {\n    "
                       "errHash=new HashMap( 150 );\n"));
  for (int count = 0; count < errdef.NumberOfErrors(); count++) {
    fprintf(out_error.get(),
            _("    errHash.put(new Integer(%s), new ErrorObj(%s.%s, %s.%s, new "
              "String(%s)));\n"),
            errdef.Name(count).c_str(), class_level.c_str(),
            errdef.Level(count).c_str(), class_response.c_str(),
            errdef.Response(count).c_str(), errdef.Message(count).c_str());
  }
  return (0);
}

int Java_Errors::close_files() {
  fprintf(out_error.get(),
          _("  } catch (Exception e) {\n    errHash=null;\n  }\n}\n"));
  fprintf(
      out_error.get(),
      _("\n\npublic static String getErrorString(int errcode) {\n  try {\n     "
        "return ((ErrorObj)(errHash.get(new Integer(errcode)))).description;\n "
        " }\n catch (Exception e) {\n     return new String( \"Unknown Error "
        "Occured\" );\n  }\n}\n\n\nprivate %s() {\n }\n}\n"),
      class_error.c_str());
  return (0);
}
