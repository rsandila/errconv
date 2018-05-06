#ifndef __ERR_DEF_H__
#define __ERR_DEF_H__

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
#include <memory>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

/*
  This means that whenever you define a std::unique_ptr<FILE> that the following
  method will be called when the variable goes out of scope instead of the
  standard delete.
*/
namespace std
{
  template<> struct default_delete<FILE> {
    void operator()(FILE* ptr) { fclose(ptr); }
  };
}

//! The number of error level parameters that is valid
const unsigned NUM_VALID_LEVELS = 8;
//! The number of error response parameters that is valid
const unsigned  NUM_VALID_RESPONSES = 6;
//! The prefix used for error names
#define ERROR_PREFIX        "ERROR"

extern std::string valid_levels[NUM_VALID_LEVELS];
extern std::string valid_responses[NUM_VALID_RESPONSES];

/*! \brief Error_Definitions reads and parses the error definition input file as provided
    \pre None
    \post Contains the parses error definitions that can be queried to generate error files for different compiler languages.

    This depends on the error definitions in Documents/Errors/definitons.rtf
 */
class Error_Definitions
{
public:
  /*! \brief Standard constructor that receives the file name of the error defintion file.
      \pre None
      \post The class is properly constructed except if isOk returns zero.
      \returns Nothing
      \param file_name The name of the file to be read and parsed.
   */
  Error_Definitions( const std::string & file_name );
  /*! \brief The default destructor.
      \pre The constructor has been called.
      \post The class is properly deconstructed with all resources returned to the system.
      \returns Nothing
   */
  virtual ~Error_Definitions();
  /*! \brief Returns whether the class constructed properly or not.
      \pre Constructor has been called.
      \post Nothing
      \returns Non-zero if the class initialized properly.
   */
  int isOk() const { return( installed ); };
  /*! \brief Returns the number of error codes parsed from the input file.
      \pre The constructor successfully completed.
      \post Nothing
      \returns The number of error codes parsed or 0 on error.
   */
  int NumberOfErrors() const { return( num_errors ); };
  /*! \brief Returns a string to the error message as specified for the index and the error definition file.
      \pre Class successfully initialized
      \post Nothing
      \returns String pointer to the error message or NULL on error.
      \param index The index into the array of strings containing the data. Must be bigger than 0 and smaller than the total number of error definitions.
   */
  const std::string & Message( int index ) const;
  /*! \brief Returns a string to the level string as specified for the index and the error definition file.
      \pre Class successfully initialized.
      \post Nothing
      \returns String pointer to the Level string or NULL on error.
      \param index The index into the array of strings containing the data. Must be bigger than 0 and smaller than the total number of error definitions.
   */
  const std::string & Level( int index ) const;
  /*! \brief Returns a string to the level string as specified for the index and the error definition file.
      \pre Class syccessfully initialized.
      \post Nothing
      \returns String pointer to the Response string or NULL on error.
      \param index The index into the array of strings containing the data. Must be bigger than 0 and smaller than the total number of error definitions.
   */
  const std::string & Response( int index ) const;
  /*! \brief Returns a string to the name string as specified for the index and
     the error definition file. \pre Class syccessfully initialized. \post
     Nothing \returns String pointer to the Name string or NULL on error. \param
     index The index into the array of strings containing the data. Must be
     bigger than 0 and smaller than the total number of error definitions.
   */
  const std::string &Name(int index) const;
  /*! \brief Returns the error code as specified for the index and the error definition file.
      \pre Class syccessfully initialized.
      \post Nothing
      \returns -1 on error or the error code as specified in the error definition file.
      \param index The index into the array containing the data. Must be bigger than 0 and smaller than the total number of error definitions.
   */
  int Code( int index ) const;

protected:
  /*! \brief Opens the input file and parse the contents of it. Fill in all structures in the class with relevant values.
      \pre Called from inside constructor.
      \post All structures filled.
      \returns Non-zero on failure.
      \param file_name The name of the file to open and parse.
   */
  int Init( const std::string & file_name );
  /*! \brief Counts the number of items in the input file.
      \pre The file has been opened in Init
      \post Nothing.
      \returns The number of lines in the input file.
      \param inp The file handle to the input file as opened by Init.
   */
  int count_lines( FILE * inp );
  /*! \brief Takes passed line appart according to specification and fills structures with needed information.
      \pre Assumes structures has been allocated.
      \post Added item and verified that it's name and code is unique.
      \returns Less than zero on an serious error, zero for an empty line and bigger than zero for a valid entry.
      \param count The index into the structures.
      \param buf The data to be parsed.
   */
  int parse_line( int count, char *buf );
  /*! \brief Makes the string passed all upper case.
      \pre Nothing
      \post The string passed is now all upper case.
      \returns Nothing
      \param buf This string is converted to upper case in place.
   */
  void strup( std::string & buf );
private:
  //! Contains non-zero if the class successfully initialized.
  int installed;
  //! The number of lines in the input file.
  int num_errors;
  //! The error names specified in the input file.
  std::vector<std::string> error_names;
  //! The error codes specified in the input file.
  std::vector<int> error_codes;
  //! The error levels as specified in the input file.
  std::vector<std::string> levels;
  //! The response levels as specified in the input file.
  std::vector<std::string> responses;
  //! The messages as specified in the input file.
  std::vector<std::string> messages;
};

#endif
