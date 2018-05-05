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

#include "globalerrors.h"
#include <libintl.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

#define _(str) gettext(str)

/*! \brief This is a test for the error code dynamic library. It assumes that
   error code 3 is declared \pre   Depends on "make all" running successfully.
   The /etc/ld.so.conf must also be edited to add the $(CVSBASE)/lib directory
   to the library path followed by /sbin/ldconfig. \post Prints out messages
   from the error library.

    It is not a class because it is too simple to use. It depends on the
   Makefile and the errconv utility.
 */
int main(int argc, char **argv) {
  int result;
  char name[255];
  int level, response;
  char message[255];

  setlocale(LC_ALL, "");
  bindtextdomain("errconv", NULL);
  textdomain("errconv");

  result = globalerrors_Errors::ErrorCode_To_Name(3, name, 255);
  if (result) {
    printf(_("Unable to resolve name.\n"));
    return (1);
  }
  result = globalerrors_Errors::ErrorCode_To_Level(3, &level);
  if (result) {
    printf(_("Unable to resolve level.\n"));
    return (1);
  }
  result = globalerrors_Errors::ErrorCode_To_Response(3, &response);
  if (result) {
    printf(_("Unable to resolve response.\n"));
    return (1);
  }
  result = globalerrors_Errors::ErrorCode_To_Message(3, message, 255);
  if (result) {
    printf(_("Unable to resolve message.\n"));
    return (1);
  }
  printf(_("For Errorcode 3 the following is true:\n Name=%s, Level=%d, "
           "Response=%d, Message=%s\n"),
         name, level, response, message);
  return (0);
}
