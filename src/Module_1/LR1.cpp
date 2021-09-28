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
  Symbol* newStart = new Symbol(this->totNumSyms, "S_'", false);
  vector<Symbol*> newPrRhs = {this->startSymbol, this->dollarSymbol};
  this->firstPr = new ProductionRule(this->totNumPr, newStart, newPrRhs);
  this->productionRules[newStart].insert(this->firstPr);
  this->startSymbol = newStart;
  this->totNumSyms++;
  this->totNumPr++;
  SetOfItems* state0 = this->createState0();
  state0->print();
}

SetOfItems* LR1::createState0() {
  set<Item*> items;
  items.insert(new Item(this->firstPr, 0, {this->dollarSymbol}));
  SetOfItems state0Kernel(items, this->totNumStates++);
  state0Kernel.print();
  return state0Kernel.getClosure(this->productionRules, this->firstSetsMap);
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
  for (auto pr : this->productionRules) {
    for (ProductionRule* productionRule : pr.second) {
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

ostream& operator<<(ostream& os,
                    const pair<vector<string>, pair<int, int>>& vec) {
  os << "[ ";
  for (int i = vec.second.first; i < vec.second.second; ++i) {
    os << vec.first[i] << " ";
  }
  os << "] ";
  return os;
}