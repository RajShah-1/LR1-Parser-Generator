#include "LR1.hpp"

LR1::LR1() {
  this->totNumSyms = 0;
  this->totNumPr = 0;
  this->totNumStates = 0;
  // take cfg as an input from the user
  // fills up: startSymbol, terminals, nonTerminals, and production-rules
  this->readCFG();
  cout << "\n You entered:\n";
  this->printCFG();
  this->computeFirst();
  // Add new start symbol S_'
  auto newStart = new Symbol(this->totNumSyms, "S_'", false);
  this->symToPtr["S_'"] = newStart;
  this->symToId[newStart] = this->totNumSyms;
  this->nonTerminals.push_back(newStart);
  vector<Symbol*> newPrRhs = {this->startSymbol, this->dollarSymbol};
  this->firstPr = new ProductionRule(this->totNumPr, newStart, newPrRhs);
  this->productionRules[newStart].insert(this->firstPr);
  this->startSymbol = newStart;
  this->totNumSyms++;
  this->totNumPr++;
  this->buildDFA();
}

LR1::~LR1() {
  for (const auto& states : this->idToDFAState) {
    delete states.second;
  }
  for (const auto& symPr : this->productionRules) {
    for (ProductionRule* pr : symPr.second) {
      delete pr;
    }
  }
  for (Symbol* sym : this->terminals) {
    delete sym;
  }
  for (Symbol* sym : this->nonTerminals) {
    delete sym;
  }
}

bool LR1::parseTokens(const vector<string>& tokens) {
  stack<int> states;
  stack<string> stateSyms;
  states.push(0);

  int numTokens = tokens.size();
  for (int tokenIndex = 0; tokenIndex < numTokens; ++tokenIndex) {
    string tokenType = tokens[tokenIndex];
    cout << "===\n";
    // Parsing logic here
    int currState = states.top();
    cout << "Processing for state: " << currState << " Token: " << tokenType
         << "\n";
    cout << "Stack: " << states << "\n";
    cout << "Syms: " << stateSyms << "\n";
    if (this->symToPtr.find(tokenType) == this->symToPtr.end()) {
      cout << "[ERROR] Token is valid, but corresponding terminal symbol is "
              "not in the grammar!\n";
      cout << "Ignoring " << tokenType << "...\n";
      continue;
    }
    Symbol* sym = this->symToPtr[tokenType];
    if (this->gotoNewState[currState].find(sym) !=
        this->gotoNewState[currState].end()) {
      // shift
      if (sym == this->dollarSymbol) return true;
      states.push(this->gotoNewState[currState][sym]);
      stateSyms.push(sym->symbol);
      cout << "Action: Shift-" << this->gotoNewState[currState][sym] << "\n";
    } else if (this->reduction[currState].find(sym) !=
               this->reduction[currState].end()) {
      // reduce
      cout << "Action: Reduce-" << this->reduction[currState][sym] << "\n";
      ProductionRule* pr = this->idToPr[this->reduction[currState][sym]];
      // if pr is of the form "NonTer -> [ EPS ]" do not pop anything from the
      // state
      if (pr->rhs.size() != 1 || pr->rhs[0] != this->epsSymbol) {
        for (int count = 1; count <= pr->rhs.size(); ++count) {
          if (states.empty()) {
            cout << "[ERROR] Empty stack\n";
            cout << "Exiting from the fn...\n";
            return false;
          }
          states.pop();
          stateSyms.pop();
        }
      }
      if (states.empty()) {
        cout << "[ERROR] Empty stack\n";
        cout << "Exiting from the fn...\n";
        return false;
      }
      currState = states.top();
      if (this->gotoNewState[currState].find(pr->lhs) ==
          this->gotoNewState[currState].end()) {
        cout << "No goto for the non-terminal " << pr->lhs->symbol << "\n";
        cout << "Exiting from the fn...\n";
        return false;
      }
      states.push(this->gotoNewState[currState][pr->lhs]);
      stateSyms.push(pr->lhs->symbol);
      --tokenIndex;
    } else {
      cout << "Unexpected token: " << tokenType << "\n";
      cout << "Exiting from the fn...\n";
      return false;
    }
  }
  cout << "AT END " << states;
  return false;
}

bool LR1::readAndParse(int (*nextToken)(), const int& lineNum, char*& tokenType,
                       char*& tokenText) {
  vector<string> tokens;
  stack<int> states;
  states.push(0);

  int token = nextToken();
  bool isEOF = false;
  while (token) {
    cout << "===\n";
    if (token == -1) {
      cout << "Error in line " << lineNum << ", Rejecting: " << tokenText
           << "\n";
      continue;
    } else {
      cout << "Token: " << tokenType << "\n";
      tokens.push_back(tokenType);
    }

    // Parsing logic here
    int currState = states.top();
    string currTokenType = tokenType;
    cout << "Processing for state: " << currState << " Token: " << tokenType
         << "\n";
    if (isEOF) {
      currTokenType = DOLLAR_SYMBOL;
    } else if (this->symToPtr.find(currTokenType) == this->symToPtr.end()) {
      cout << "[ERROR] Token is valid, but corresponding terminal symbol is "
              "not in the grammar!\n";
      cout << "Ignoring " << currTokenType << "...\n";
      continue;
    }
    Symbol* sym = this->symToPtr[currTokenType];
    if (this->gotoNewState[currState].find(sym) !=
        this->gotoNewState[currState].end()) {
      // shift
      states.push(this->gotoNewState[currState][sym]);
      cout << "Action: Shift-" << this->gotoNewState[currState][sym] << "\n";
    } else if (this->reduction[currState].find(sym) !=
               this->reduction[currState].end()) {
      // reduce
      cout << "Action: Reduce-" << this->reduction[currState][sym] << "\n";
      ProductionRule* pr = this->idToPr[this->reduction[currState][sym]];
      for (int count = 1; count <= pr->rhs.size(); ++count) {
        if (states.empty()) {
          cout << "[ERROR] Empty stack\n";
          cout << "Exiting from the fn...\n";
          return false;
        }
        states.pop();
      }
      if (states.empty()) {
        cout << "[ERROR] Empty stack\n";
        cout << "Exiting from the fn...\n";
        return false;
      }
      currState = states.top();
      if (this->gotoNewState[currState].find(pr->lhs) ==
          this->gotoNewState[currState].end()) {
        cout << "No goto for the non-terminal " << pr->lhs->symbol << "\n";
        cout << "Exiting from the fn...\n";
        return false;
      }
      states.push(this->gotoNewState[currState][pr->lhs]);
    } else {
      cout << "Unexpected token: " << currTokenType << "\n";
      cout << "Exiting from the fn...\n";
      return false;
    }
    if (isEOF) break;
    token = nextToken();
    if (token == 0 && !isEOF) {
      token = 1;
      isEOF = true;
    }
  }
  return states.empty();
}

void LR1::buildDFA() {
  SetOfItems* state0 = this->createState0();
  SetOfItems* state0Closure =
      state0->getClosure(this->productionRules, this->firstSetsMap);
  this->idToDFAState[this->totNumStates++] = state0Closure;
  this->hashToDFAStates[state0->computeHash()] = state0Closure;
  delete state0;  // deleting as we have its closure

  queue<SetOfItems*> processingQueue;
  processingQueue.push(state0Closure);
  while (!processingQueue.empty()) {
    SetOfItems* currState = processingQueue.front();
    processingQueue.pop();
    auto nextSyms = currState->getNextSymbols();
    for (Symbol* sym : nextSyms) {
      SetOfItems* newState = currState->goToNewState(sym);
      string newStateHash = newState->computeHash();
      if (this->hashToDFAStates.find(newStateHash) ==
          this->hashToDFAStates.end()) {
        SetOfItems* newStateClosure =
            newState->getClosure(this->productionRules, this->firstSetsMap);
        delete newState;
        newStateClosure->setStateIndex(this->totNumStates);
        this->idToDFAState[this->totNumStates] = newStateClosure;
        this->hashToDFAStates[newStateHash] = newStateClosure;
        this->totNumStates++;
        processingQueue.push(newStateClosure);
      } else {
        delete newState;
      }
      this->gotoNewState[currState->getStateIndex()][sym] =
          this->hashToDFAStates[newStateHash]->getStateIndex();
    }
  }

  // Fill reductions for each state
  for (const auto& state : this->idToDFAState) {
    SetOfItems* itemsSet = state.second;
    int stateIndex = state.first;
    auto redns = itemsSet->getReductions();
    for (const auto& redn : redns) {
      this->reduction[stateIndex][redn.first] = redn.second->id;
    }
  }

  // Check for shift-reduce conflicts
  for (const auto& state : this->idToDFAState) {
    SetOfItems* itemsSet = state.second;
    int stateIndex = state.first;
    auto& shifts = this->gotoNewState[stateIndex];
    auto& reductions = this->reduction[stateIndex];
    for (auto sym : shifts) {
      if (reductions.find(sym.first) != reductions.end()) {
        cout << "[ERROR] Shift-Reduce conflict!!! State: " << stateIndex
             << " Symbol:" << sym.first->symbol << "\n";
        throw NOT_LR1_EXCEPTION;
      }
    }
  }

  cout << "DFA States:\n";
  for (const auto& states : this->idToDFAState) {
    states.second->print();
  }

  cout << "Action and Goto for all states:\n";
  for (const auto& state : this->idToDFAState) {
    int stateIndex = state.first;
    cout << "State-" << stateIndex << "\n";
    for (const auto& gotoEntry : this->gotoNewState[stateIndex]) {
      cout << "GOTO( " << stateIndex << ", " << gotoEntry.first->symbol
           << ") = "
           << "S" << gotoEntry.second << "\n";
    }
    for (const auto& rednEntry : this->reduction[stateIndex]) {
      cout << "REDUCE( " << stateIndex << ", " << rednEntry.first->symbol
           << ") = "
           << "S" << rednEntry.second << "\n";
    }

    cout << "===\n";
  }
}

SetOfItems* LR1::createState0() {
  this->totNumStates = 0;
  set<Item*> items;
  set<Symbol*> lookup;
  lookup.insert({this->dollarSymbol});
  items.insert(new Item(this->firstPr, 0, lookup));
  return new SetOfItems(items, this->totNumStates);
}

void LR1::computeFirstForSym(Symbol* sym) {
  if (this->firstSetsMap.find(sym) != firstSetsMap.end()) {
    return;
  }

  auto& firstSet = this->firstSetsMap[sym];

  if (sym->isTerminal) {
    firstSet.insert(sym);
    return;
  }

  // for non-terminals -> iterate over each production rule
  for (ProductionRule* pr : this->productionRules[sym]) {
    bool isEps = false;
    // compute first of the first sym on RHS
    // if it contains eps -> compute first of the next sym and so on...
    for (Symbol* rhsSym : pr->rhs) {
      this->computeFirstForSym(rhsSym);

      const auto& rhsFirstSet = this->firstSetsMap[rhsSym];
      for (Symbol* rhsFirstSym : rhsFirstSet) {
        firstSet.insert(rhsFirstSym);
      }
      isEps = (rhsFirstSet.find(epsSymbol) != rhsFirstSet.end());
      if (!isEps) {
        break;
      }
    }
    if (isEps) {
      firstSet.insert(this->epsSymbol);
    }
  }
}

void LR1::computeFirst() {
  // cout << "First-sets: \n";
  for (Symbol* ter : this->terminals) {
    this->computeFirstForSym(ter);
    // cout << ter->symbol << ": [ ";
    // for (Symbol* sym : this->firstSetsMap[ter]) {
    //   cout << sym->symbol << " ";
    // }
    // cout << "]\n";
  }
  for (Symbol* nonTer : this->nonTerminals) {
    this->computeFirstForSym(nonTer);
    // cout << nonTer->symbol << ": [ ";
    // for (Symbol* sym : this->firstSetsMap[nonTer]) {
    //   cout << sym->symbol << " ";
    // }
    // cout << "]\n";
  }
}

void LR1::readCFG() {
  this->symToPtr.clear();
  this->symToId.clear();
  string sym;

  cout << "Note: terminal symbols and non-terminal symbols can be strings. "
          "The string must not contain either of whitespace, tab, newline, "
          "']', " +
              (DOLLAR_SYMBOL) + ", and '_'. Use \""
       << EPSILON_SYMBOL << "\" as epsilon.\n\n";

  // add epsilon to grammar
  this->symToPtr[EPSILON_SYMBOL] =
      new Symbol(this->totNumSyms, EPSILON_SYMBOL, true);
  this->epsSymbol = this->symToPtr[EPSILON_SYMBOL];
  this->terminals.push_back(this->epsSymbol);
  symToId[this->epsSymbol] = this->totNumSyms;
  // this->terminals.push_back(this->symToPtr[EPSILON_SYMBOL]);
  this->totNumSyms++;

  //  add $ symbol to grammar
  this->symToPtr[DOLLAR_SYMBOL] =
      new Symbol(this->totNumSyms, DOLLAR_SYMBOL, true);
  this->dollarSymbol = this->symToPtr[DOLLAR_SYMBOL];
  this->terminals.push_back(this->dollarSymbol);
  symToId[this->dollarSymbol] = this->totNumSyms;
  // this->terminals.push_back(this->symToPtr[DOLLAR_SYMBOL]);
  this->totNumSyms++;

  int numNonTers, numTers, numProdRules;
  cout << "Enter number of non-terminals: ";
  cin >> numNonTers;
  cout << "Enter " << numNonTers << " non-terminals:\n";
  for (int i = 0; i < numNonTers; ++i) {
    cin >> sym;
    this->symToPtr[sym] = new Symbol(this->totNumSyms, sym, false);
    symToId[this->symToPtr[sym]] = this->totNumSyms;
    this->nonTerminals.push_back(this->symToPtr[sym]);
    ++this->totNumSyms;
  }

  cout << "Enter number of terminals: ";
  cin >> numTers;
  cout << "Enter " << numTers << " terminals:\n";
  for (int i = 0; i < numTers; ++i) {
    cin >> sym;
    this->symToPtr[sym] = new Symbol(this->totNumSyms, sym, true);
    symToId[this->symToPtr[sym]] = this->totNumSyms;
    this->terminals.push_back(this->symToPtr[sym]);
    ++this->totNumSyms;
  }

  cout << "\n===\n";
  cout << "Production rule must have the "
          "following format:\n";
  cout << "Non-terminal -> [ a space separated list of terminals and "
          "non-terminals ] (first and last symbols on rhs must be separated "
          "from "
          "the square brackets by at least one space)\n";
  cout << "pipe-sign ('|') can be used \"within square brackets\" (not "
          "outside) as OR for "
          "multiple RHS\n";
  cout << "Example: A -> [ a B C d a A | X y z W s ]\n";
  cout << "NOTE: Do not include S' -> [ S $ ] as it is added by the program\n";
  cout << "===\n\n";

  cout << "Enter number of production rules:\n";
  cin >> numProdRules;
  cout << "Enter " << numProdRules << " production rules:\n";
  for (int i = 0; i < numProdRules; ++i) {
    string tmpStr, lhsStr;
    // parsing the production rules

    cin >> sym;
    assert(this->symToPtr.find(sym) != this->symToPtr.end());
    assert(!this->symToPtr[sym]->isTerminal);
    Symbol* lhs = this->symToPtr[sym];
    lhsStr = sym;

    cin >> tmpStr;
    assert(tmpStr == "->");

    cin >> tmpStr;
    assert(tmpStr == "[");
    cin >> sym;
    vector<Symbol*> rhs;
    ProductionRule* newPrPtr;
    while (sym != "]") {
      if (sym == PRODUCTION_RULE_OR) {
        newPrPtr = new ProductionRule(this->totNumPr, lhs, rhs);
        this->productionRules[this->symToPtr[lhsStr]].insert(newPrPtr);
        this->prToId[newPrPtr] = this->totNumPr;
        this->idToPr[this->totNumPr] = newPrPtr;
        ++this->totNumPr;
        rhs.clear();
      } else {
        assert(this->symToPtr.find(sym) != this->symToPtr.end());
        rhs.push_back(this->symToPtr[sym]);
      }
      cin >> sym;
    }
    newPrPtr = new ProductionRule(this->totNumPr, lhs, rhs);
    this->productionRules[this->symToPtr[lhsStr]].insert(newPrPtr);
    this->prToId[newPrPtr] = this->totNumPr;
    this->idToPr[this->totNumPr] = newPrPtr;
    ++this->totNumPr;
  }

  cout << "Enter start symbol: ";
  cin >> sym;
  assert(this->symToPtr.find(sym) != this->symToPtr.end());
  assert(!this->symToPtr[sym]->isTerminal);
  this->startSymbol = this->symToPtr[sym];
}

void LR1::printCFG() {
  cout << "=== CFG\n";
  cout << "Terminals: ";
  for (Symbol* terminal : this->terminals) {
    cout << terminal->symbol << " ";
  }
  cout << "\n";

  cout << "Non-terminals: ";
  for (Symbol* nonTerminal : this->nonTerminals) {
    cout << nonTerminal->symbol << " ";
  }
  cout << "\n";

  cout << "Start symbol: " << this->startSymbol->symbol << "\n";

  cout << "\nProduction rules:\n";
  for (const auto& pr : this->productionRules) {
    for (ProductionRule* productionRule : pr.second) {
      cout << "Id: " << productionRule->id << " ";
      cout << productionRule << "\n";
    }
  }
  cout << "===\n";
}

void LR1::setDirPath(const string& dirPath) { this->dirPath = dirPath; }

ostream& operator<<(ostream& os, const Symbol* sym) {
  os << sym->symbol << " ";
  // os << "[ " << sym->symbol << " " << sym->id << " " << sym->isTerminal
  //    << " ] ";
  return os;
}

ostream& operator<<(ostream& os, const ProductionRule* pr) {
  os << pr->lhs->symbol << " -> [ ";
  for (Symbol* rhsSym : pr->rhs) {
    os << rhsSym->symbol << " ";
  }
  os << "] ";
  return os;
}

ostream& operator<<(ostream& os, stack<Symbol*> st) {
  os << "[ ";
  while (!st.empty()) {
    os << st.top()->symbol << " ";
    st.pop();
  }
  os << "] ";
  return os;
}

ostream& operator<<(ostream& os, stack<int> st) {
  os << "[ ";
  while (!st.empty()) {
    os << st.top() << " ";
    st.pop();
  }
  os << "] ";
  return os;
}

ostream& operator<<(ostream& os, stack<string> st) {
  os << "[ ";
  while (!st.empty()) {
    os << st.top() << " ";
    st.pop();
  }
  os << "] ";
  return os;
}

ostream& operator<<(ostream& os,
                    const pair<vector<string>, pair<int, int>>& vec) {
  os << "[ ";
  for (int i = vec.second.first; i < vec.second.second; ++i) {
    os << vec.first[i] << " ";
  }
  os << "] ";
  return os;
}