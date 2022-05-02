#include "parser.h"

int main() {
  memset(ActionGoto, 0, sizeof(ActionGoto));
  readGrammarFile();
  getFirstSet();
  create('Z', "S");
  show();
  scan(token);
}