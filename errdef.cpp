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
#include <ctype.h>
#include <stdexcept>
#include <string.h>
#include <string>
#include <unistd.h>
#ifdef __LOCAL__
#include <libintl.h>
#include <locale.h>

#define _(str) gettext(str)
#else
#define _(str) str
#endif

//! This defines the valid values that the error level parameter can take
std::string valid_levels[NUM_VALID_LEVELS] = {
    "DEBUG", "INFO", "NOTICE", "WARNING", "ERR", "CRIT", "ALERT", "EMERG"};

//! this defines the valid values that the error response parameter can take
std::string valid_responses[NUM_VALID_RESPONSES] = {
    "IGNORE", "ALERT", "FALLOVER", "RETRY", "ABORT", "PANIC"};

Error_Definitions::Error_Definitions(const std::string &file_name) {
  installed = Init(file_name);
}

Error_Definitions::~Error_Definitions() { installed = 0; }

int Error_Definitions::Init(const std::string &file_name) {
  FILE *inp;
  char buf[2048];
  int count, result;
  num_errors = 0;
  if (access(file_name.c_str(), R_OK)) {
    fprintf(stderr, _("No read access to %s\n"), file_name.c_str());
    return (0);
  }
  inp = fopen(file_name.c_str(), "rt");
  if (!inp) {
    fprintf(stderr, _("Unable to open %s\n"), file_name.c_str());
    return (0);
  }
  num_errors = count_lines(inp);
  if (!num_errors) {
    fprintf(stderr, _("Unable to determine length of file: %s\n"),
            file_name.c_str());
    return (0);
  }
  printf(_("File %s contains %d lines.\n"), file_name.c_str(), num_errors);
  error_names.resize(num_errors);
  error_codes.resize(num_errors);
  levels.resize(num_errors);
  responses.resize(num_errors);
  messages.resize(num_errors);
  count = 0;
  while (fgets(buf, 2048, inp)) {
    if (count >= num_errors) {
      fprintf(stderr, _("File seems to have grown???\n"));
      fclose(inp);
      return (0);
    }
    result = parse_line(count, buf);
    if (result < 0) {
      fclose(inp);
      return (0);
    }
    if (result)
      count += 1;
  }
  if (count < num_errors) {
    printf(_("Warning: Empty or invalid lines detected.\n"));
    num_errors = count;
  }
  fclose(inp);
  return (1);
}

int Error_Definitions::parse_line(int count, char *buf) {
  char *tok;
  int cnt1, cnt2;
  std::string name, response, message, level;
  int code;

  if (!buf)
    return (-1);
  if (buf[0] == '\n' || buf[0] == '\r')
    return (0);
  tok = strtok(buf, ":");
  if (!tok) {
    fprintf(stderr, _("Invalid formatted string: %d\n"), count);
    return (-1);
  }
  name = tok;
  strup(name);
  for (cnt1 = 0; cnt1 < count; cnt1++) {
    if (error_names[cnt1] == name) {
      fprintf(stderr, _("Entries %d and %d have identical names: %s\n"), cnt1,
              count, name.c_str());
      return (-1);
    }
  }
  tok = strtok(NULL, ":");
  if (!tok) {
    fprintf(stderr, _("Invalid formatted string: %d\n"), count);
    return (-1);
  }
  code = atoi(tok);
  for (cnt1 = 0; cnt1 < count; cnt1++) {
    if (code == error_codes[cnt1]) {
      fprintf(stderr, _("Entries %d and %d have identical error codes.\n"),
              cnt1, count);
      return (-1);
    }
  }
  tok = strtok(NULL, ":");
  if (!tok) {
    fprintf(stderr, _("Invalid formatted string: %d\n"), count);
    return (-1);
  }
  level = tok;
  strup(level);
  cnt2 = 0;
  for (cnt1 = 0; cnt1 < NUM_VALID_LEVELS; cnt1++) {
    if (level == _(valid_levels[cnt1]))
      cnt2 += 1;
  }
  if (!cnt2) {
    fprintf(stderr, _("Invalid level %s specified in line %d\n"), level.c_str(),
            count);
    return (-1);
  }
  tok = strtok(NULL, ":");
  if (!tok) {
    fprintf(stderr, _("Invalid formatted string: %d\n"), count);
    return (-1);
  }
  response = tok;
  strup(response);
  cnt2 = 0;
  for (cnt1 = 0; cnt1 < NUM_VALID_RESPONSES; cnt1++) {
    if (response == _(valid_responses[cnt1]))
      cnt2 += 1;
  }
  if (!cnt2) {
    fprintf(stderr, _("Invalid response %s specified in line %d\n"),
            response.c_str(), count);
    return (-1);
  }
  tok = strtok(NULL, ":");
  if (!tok) {
    fprintf(stderr, _("Invalid formatted string: %d\n"), count);
    return (-1);
  }
  message = tok;
  if (message[message.length() - 1] == '\n')
    message[message.length() - 1] = 0;
  error_names[count] = name;
  levels[count] = level;
  responses[count] = response;
  messages[count] = message;
  error_codes[count] = code;
  return (1);
}

void Error_Definitions::strup(std::string &buf) {
  std::transform(buf.begin(), buf.end(), buf.begin(), ::toupper);
}

int Error_Definitions::count_lines(FILE *inp) {
  long pos;
  int count;
  char buf[2048];

  pos = ftell(inp);
  count = 0;
  while (fgets(buf, 2048, inp))
    count += 1;
  fseek(inp, pos, SEEK_SET);
  return (count);
}

const std::string &Error_Definitions::Message(int index) const {
  if (!isOk() || index < 0 || index >= NumberOfErrors())
    throw std::invalid_argument("Invalid index");
  return (messages[index]);
}

const std::string &Error_Definitions::Level(int index) const {
  if (!isOk() || index < 0 || index >= NumberOfErrors())
    throw std::invalid_argument("Invalid index");
  return (levels[index]);
}

const std::string &Error_Definitions::Response(int index) const {
  if (!isOk() || index < 0 || index >= NumberOfErrors())
    throw std::invalid_argument("Invalid index");
  return (responses[index]);
}

const std::string &Error_Definitions::Name(int index) const {
  if (!isOk() || index < 0 || index >= NumberOfErrors())
    throw std::invalid_argument("Invalid index");
  return (error_names[index]);
}

int Error_Definitions::Code(int index) const {
  if (!isOk() || index < 0 || index >= NumberOfErrors())
    throw std::invalid_argument("Invalid index");
  return (error_codes[index]);
}
