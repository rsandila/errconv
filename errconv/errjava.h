#ifndef __ERRJAVA_H__
#define __ERRJAVA_H__

#include "errdef.h"

/*! \brief This class will take the data as provided by the Error_Defintions class and write it to Java classes
    \pre The Error_Definitions class must be properly intialized. The Error_Definitions class passed to it must be valid for the whole life of this class.
    \post Three new files will be created [out_base]ImpLevel.java, [out_base]Response.java and [out_base]Error.java with classes with the same names as the files without the .java that will handle error processing and reporting. 
    
    This class depends on the class Error_Defintions.
 */
class Java_Errors
{
 public:
  /*! \brief The constructor for the class.
      \pre The err parameter has been properly initialized.
      \post All parameters have been checked to be valid.
      \returns Nothing
   */
  Java_Errors( char *out_base, Error_Definitions *err );
  /*! \brief The default destructor for the class.
      \pre Assumes the constructor has been called.
      \post All resources have been free'd except the Error_Definitions class passed to it.
      \returns Nothing.
   */
  virtual ~Java_Errors(); 
  /*! \brief Returns whether the class initialized correctly or not.
      \pre The constructor has been called.
      \post Nothing.
      \returns Non-zero if the class initialized correctly.
   */
  int isOk() { return( installed ); };
  /*! \brief Processes the error codes and generate the relevant files.
      \pre The constructor initialized correctly.
      \post The files have been created.
      \returns Non-Zero on failure.
   */
  int execute();
 protected:
  /*! \brief Called by the constructor to set all variables and test all parameters
      \pre The err parameter has been properly initialized
      \post The class is ready to fullfill it's function.
      \returns Zero on failure.
      \param out_base The base name used for the file and class name generation.
      \param err The class containing the parsed error information.
   */
  int Init( char *out_base, Error_Definitions *err );
  /*! \brief Creates the two files and initialized it with the needed header information.
      \pre All parameters have been verified and the target files can be overwritten.
      \post The target files are overwritten and initialized.
      \return Non-zero on failure.
   */
  int create_files();
  /*! \brief Steps through every error and fills in the two files with the relevant data.
      \pre Assumes create_files have been successfully called.
      \post The two files contain most of the relevant information.
      \returns Non-zero on failure.
   */
  int parse_errors();
  /*! \brief Closes the files writing any final information that is needed.
      \pre Assumes that parse_errors and create_files were successfully called.
      \post The two files are complete and usable.
      \returns Non-zero on failure.
   */
  int close_files();
 private:
  //! Contains non-zero on successfull construction of the class.
  int installed;
  //! The object that contains the parse error defintions
  Error_Definitions *errdef;
  //! The file handles of the output files.
  FILE *out_level, *out_response, *out_error;
  //! The base name used for constructing the file names and the class name.
  char base[255];
  //! The names of the three classes
  char class_level[255], class_response[255], class_error[255];
};

#endif
