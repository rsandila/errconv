#ifndef __ERRCPP_H__
#define __ERRCPP_H__

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

#include "errdef.h"
/*! \brief This class will take the data as provided by the Error_Defintions
   class and write it to a C++ class \pre The Error_Definitions class must be
   properly intialized. The Error_Definitions class passed to it must be valid
   for the whole life of this class. \post Two new files will be created
   [out_base].cpp and [out_base].h with a class with the name of
   [out_base]_Errors that will handle error processing and reporting.

    This class depends on the class Error_Defintions.
 */
class CPP_Errors {
public:
  /*! \brief The constructor for the class.
      \pre The err parameter has been properly initialized.
      \post All parameters have been checked to be valid.
      \returns Nothing
   */
  CPP_Errors(const std::string &out_base, const Error_Definitions &err);
  /*! \brief The default destructor for the class.
      \pre Assumes the constructor has been called.
      \post All resources have been free'd except the Error_Definitions class
     passed to it. \returns Nothing.
   */
  virtual ~CPP_Errors();
  /*! \brief Returns whether the class initialized correctly or not.
      \pre The constructor has been called.
      \post Nothing.
      \returns Non-zero if the class initialized correctly.
   */
  int isOk() { return (installed); };
  /*! \brief Processes the error codes and generate the relevant files.
      \pre The constructor initialized correctly.
      \post The files have been created.
      \returns Non-Zero on failure.
   */
  int execute();

protected:
  /*! \brief Creates the two files and initialized it with the needed header
     information. \pre All parameters have been verified and the target files
     can be overwritten. \post The target files are overwritten and initialized.
      \return Non-zero on failure.
   */
  int create_files();
  /*! \brief Steps through every error and fills in the two files with the
     relevant data. \pre Assumes create_files have been successfully called.
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
  const Error_Definitions &errdef;
  //! The file handles of the output files.
  std::unique_ptr<FILE> out_h, out_cpp;
  //! The base name used for constructing the file names and the class name.
  std::string base;
  //! The name used for the created class
  std::string classname;
};

#endif
