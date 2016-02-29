/*            __ __                                                     *\
**     __    /_// /  ___            lila compiler API                   **
**    / /   __ / /  / _ |           (c) 2016, Christian Krause          **
**   / /__ / // /__/ __ |                                               **
**  /____//_//____/_/ | |                                               **
\*                    |/                                                */

#include "util.hpp"

string lila::util::mkString(const vector<string> &strings, const string &sep) {
  ostringstream oss;

  auto size = strings.size();

  for (unsigned i = 0; i < size; i++) {
    oss << strings[i];
    if (i < (size - 1)) oss << sep;
  }

  return oss.str();
}
