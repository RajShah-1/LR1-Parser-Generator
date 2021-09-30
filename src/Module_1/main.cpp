#include "LR1.hpp"
#include "lexFns.hpp"

int main() {
  LR1 cfg;
  cout << "Enter a directory to store created GOTO/REDUCE actions and LR1 "
          "Grammar: ";
  string dir;
  cin >> dir;
  cfg.setDirPath(dir);
  cfg.createLR1File();
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
  bool isValid = cfg.parseTokens(tokens);
  cout << "Verdict: " << (isValid ? "Accepted" : "Rejected") << "\n";
  return 0;
}
