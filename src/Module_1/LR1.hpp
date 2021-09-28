#ifndef __LL1_HPP__
#define __LL1_HPP__

#include <cassert>
#include <fstream>
#include <iostream>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../Common/constants.hpp"
#include "../Common/structs.hpp"
#include "./SetOfItems.hpp"

using namespace std;

class LR1 {
  int totNumSyms;
  int totNumPr;
  int totNumStates;
  Symbol* startSymbol;
  Symbol* epsSymbol;
  Symbol* dollarSymbol;
  vector<Symbol*> terminals;
  vector<Symbol*> nonTerminals;

  // map symbol-strings to corresponding symbol-pointer
  unordered_map<string, Symbol*> symToPtr;

  // map symbol-strings to corresponding symbol-id
  unordered_map<Symbol*, int> symToId;

  // map (symbol-ptr) to (production rules with that symbol on lhs)
  unordered_map<Symbol*, unordered_set<ProductionRule*>> productionRules;

  // map production-rule pointers to corresponding production-rule-id
  unordered_map<ProductionRule*, int> prToId;

  // Relative path of grammar's folder from current directory
  string dirPath;

  // hash of sets of LR(1) canonical items mapped to ptrs to sets
  unordered_map<string, SetOfItems*> hashToDFAStates;
  // integer id mapped to sets of LR(1) canonical items
  unordered_map<int, SetOfItems*> idToDFAState;

  // lookup for which new state to go from a state-id and a symbol
  unordered_map<int, unordered_map<Symbol*, int>> gotoNewState;

  // Map [sym -> syms in the first set of the given sym]
  unordered_map<Symbol*, unordered_set<Symbol*>> firstSetsMap;

  // Pointer to the first production rule
  ProductionRule* firstPr;

  void computeFirst();
  void computeFirstForSym(Symbol* sym);

  SetOfItems* createState0();

 public:
  LR1();
  void readCFG();
  void printCFG();
  void buildDFA();
  void setDirPath(const string& dirPath);
  // void createParseTableFile() const;
};

// Operator-overloadings for easy printing
// print stack
ostream& operator<<(ostream& os, stack<Symbol*> st);
// print vector from pair.second.first to pair.second.second
// (pair.second.first inclusive and pair.second.second exclusive)
ostream& operator<<(ostream& os,
                    const pair<vector<string>, pair<int, int>>& vec);
#endif