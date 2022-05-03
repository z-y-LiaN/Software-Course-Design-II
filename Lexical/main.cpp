#include "lexical.h"
int main() {
  init();
  createNFA();
  // printNFA();
  NFA_TO_DFA();
  // printDFA();
  // spilitSourceCode();
  scanSourceCode();
  return 0;
}