#include "globalerrors.h"
#include <stdlib.h>
#include <stdio.h>
/*! \brief This is a test for the error code dynamic library. It assumes that error code 3 is declared
    \pre   Depends on "make all" running successfully. The /etc/ld.so.conf must also be edited to add the $(CVSBASE)/lib directory to the library path followed by /sbin/ldconfig.
    \post Prints out messages from the error library.
    
    It is not a class because it is too simple to use. It depends on the Makefile and the errconv utility.
 */
int main( int argc, char **argv )
{
  int result;
  char name[255];
  int level, response;
  char message[255];
  
  result=globalerrors_Errors::ErrorCode_To_Name( 3, name, 255 );
  if (result)
    {
      printf( "Unable to resolve name.\n" );
      return( 1 );
    }
  result=globalerrors_Errors::ErrorCode_To_Level( 3, &level );
  if (result)
    {
      printf( "Unable to resolve level.\n" );
      return( 1 );
    }
  result=globalerrors_Errors::ErrorCode_To_Response( 3, &response );
  if (result)
    {
      printf( "Unable to resolve response.\n" );
      return( 1 );
    }
  result=globalerrors_Errors::ErrorCode_To_Message( 3, message, 255 );
  if (result)
    {
      printf( "Unable to resolve message.\n" );
      return( 1 );
    }
  printf( "For Errorcode 3 the following is true:\n Name=%s, Level=%d, Response=%d, Message=%s\n", name, level, response, message );
  return( 0 );
}


