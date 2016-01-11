/*            __ __                                                     *\
**     __    /_// /  ___            lila API                            **
**    / /   __ / /  / _ |           (c) 2016, Christian Krause          **
**   / /__ / // /__/ __ |                                               **
**  /____//_//____/_/ | |                                               **
\*                    |/                                                */

#include <fstream>

#include "lexer.hpp"

using namespace std;

using namespace lila::lexer;

int main(int argc, char** argv) {

  /*
    file stream -> lexed stream -> ast stream
   */

  vector<unique_ptr<Token>> tokens;

  if (argc == 1) {
    tokenize(&cin, &tokens);
  } else {
    char * filename = argv[1];

    ifstream is(filename);

    if (is) {
      tokenize(&is, &tokens);
      is.close();
    } else {
      fprintf(stderr, "error opening file\n");
      return 1;
    }
  }

  for (auto it = tokens.begin() ; it != tokens.end(); ++it) {
    Token * token = (*it).get();
    cout << "[debug] [token] \"" << token->toString().c_str() << "\"" << endl;
  }

  return 0;
}
