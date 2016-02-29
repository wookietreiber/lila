/*            __ __                                                     *\
**     __    /_// /  ___            lila compiler API                   **
**    / /   __ / /  / _ |           (c) 2016, Christian Krause          **
**   / /__ / // /__/ __ |                                               **
**  /____//_//____/_/ | |                                               **
\*                    |/                                                */

#ifndef LILA_UTIL_H
#define LILA_UTIL_H

#include <sstream>
#include <vector>

using namespace std;

namespace lila {
  namespace util {

    string mkString(const vector<string> &strings, const string &sep);

  }
}

#endif
