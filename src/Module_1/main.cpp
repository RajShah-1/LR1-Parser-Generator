#include "LR1.hpp"
#include "lexFns.hpp"

int main() {
  LR1 cfg;
  cout << "Enter input:\n";
  vector<string> tokens;

  cout << "Test-input:\n";
  int token = yylex();
  while (token) {
    if (token == -1) {
      cout << "Error in line " << line_number << ", Rejecting: " << yytext
           << "\n";
    } else {
      cout << "Token: " << yy_token_type << "\n";
      tokens.push_back(yy_token_type);
    }
    token = yylex();
  }
  tokens.push_back(DOLLAR_SYMBOL);
  cout << (cfg.parseTokens(tokens) ? "Accepted" : "Rejected") << "\n";
  return 0;
}
