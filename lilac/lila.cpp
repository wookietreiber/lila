/*            __ __                                                     *\
**     __    /_// /  ___            lila API                            **
**    / /   __ / /  / _ |           (c) 2016, Christian Krause          **
**   / /__ / // /__/ __ |                                               **
**  /____//_//____/_/ | |                                               **
\*                    |/                                                */

#include <iostream>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

using namespace std;

static void repl() {
  string line;

  cout << "welcome to " << PACKAGE_STRING << endl;
  cout << "type in expressions to have them evaluated" << endl;
  cout << "type :help for more information" << endl << endl;

  while (1) {
    cout << "lila> ";

    getline(cin, line);

    if (cin.eof()) {
      // write final newline so shell prompt does not end up after our prompt
      cout << ":quit" << endl;
      break;
    }

    if (line.compare(":quit") == 0) {
      break;
    }

    if (line.compare(":help") == 0) {
      cout << ":help   print this help summary" << endl;
      cout << ":quit   exit" << endl;
    }

    // TODO jit the line
  }
}

int main(int argc, char** argv) {

  repl();

  // ---------------------------------------------------------------------------
  // end
  // ---------------------------------------------------------------------------

  return 0;
}
