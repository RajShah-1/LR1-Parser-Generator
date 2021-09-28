#include "./SetOfItems.hpp"

// Methods of Item class
Item::Item(const ProductionRule* pr, int dotIndex, const set<Symbol*>& lookup)
    : pr(pr), dotIndex(dotIndex), lookup(lookup) {}

/**
 * - hash-format: [ prod-rule-id # dot-index # space seperated lookup syms ]
 * @return string hash for the given item
 * */
string Item::computeHash() const {
  string hash =
      "[ " + to_string(this->pr->id) + " # " + to_string(this->dotIndex) + " #";
  for (const Symbol* sym : lookup) {
    hash += " " + to_string(sym->id);
  }
  hash += "]";
  // cout << "[DEBUG Item Hash] " << hash << "\n";
  return hash;
}

void Item::print() const {
  cout << this->pr->lhs->symbol << " -> [ ";
  int index = 0;
  for (Symbol* rhsSym : this->pr->rhs) {
    if (index == this->dotIndex) {
      cout << "\033[1;32m.\033[0m ";
    }
    cout << rhsSym->symbol << " ";
    index++;
  }
  if (index == this->dotIndex) {
    cout << "\033[1;32m.\033[0m ";
  }
  cout << "] ";

  cout << "Dot-index: " << this->dotIndex << " Lookup: [ ";
  for (Symbol* sym : this->lookup) {
    cout << sym->symbol << " ";
  }
  cout << "]";
}

// Methods for SetOfItems class

SetOfItems::SetOfItems(const set<Item*>& items, int stateIndex)
    : items(items), stateIndex(stateIndex) {}

SetOfItems::~SetOfItems() {
  for (Item* it : this->items) {
    delete it;
  }
}

int SetOfItems::getStateIndex() const { return this->stateIndex; }

void SetOfItems::setStateIndex(int stateId) { this->stateIndex = stateId; }

// void SetOfItems::addItem(Item* item) {}

vector<Item> SetOfItems::closureOneItem(
    const Item& it,
    const unordered_map<Symbol*, unordered_set<ProductionRule*>>&
        productionRules,
    const unordered_map<Symbol*, unordered_set<Symbol*>>& firstSetsMap) const {
  vector<Item> closure;
  // cout << "[DEBUG] closureOneItem " << it.dotIndex << " " <<
  // it.pr->rhs.size()
  //      << " "
  //      << "\n";
  if (it.dotIndex < it.pr->rhs.size() && !it.pr->rhs[it.dotIndex]->isTerminal) {
    const auto& prodRulesPtr = productionRules.find(it.pr->rhs[it.dotIndex]);
    if (prodRulesPtr == productionRules.end()) return {};
    // compute lookup for the rule to be processed
    bool isEps = true;
    set<Symbol*> newItemLookup;
    const auto& itProdRHS = it.pr->rhs;
    for (int i = it.dotIndex + 1; i < itProdRHS.size(); ++i) {
      isEps = false;
      auto firstSetPtr = firstSetsMap.find(itProdRHS[i]);
      if (firstSetPtr == firstSetsMap.end()) {
        continue;
      }
      auto firstSet = *firstSetPtr;
      for (Symbol* sym : firstSet.second) {
        if (sym->symbol != EPSILON_SYMBOL) {
          newItemLookup.insert(sym);
        } else {
          isEps = true;
        }
      }
      if (!isEps) break;
    }
    if (isEps) {
      set<Symbol*> itLookup = it.lookup;
      // cout << "[DEBUG] lookup-size: " << itLookup.size() << "\n";
      for (auto sym : itLookup) {
        //  cout << sym->symbol << " ";
        newItemLookup.insert(sym);
      }
      // cout << "\n";
    }

    const auto& prodRules = *prodRulesPtr;
    for (const ProductionRule* prodRule : prodRules.second) {
      closure.push_back(Item(prodRule, 0, newItemLookup));
    }
    // cout << "[DEBUG] Closure of item computed\n";
  }
  return closure;
}

SetOfItems* SetOfItems::getClosure(
    const unordered_map<Symbol*, unordered_set<ProductionRule*>>&
        productionRules,
    const unordered_map<Symbol*, unordered_set<Symbol*>>& firstSetsMap) const {
  // Note:
  // It is possible that while finding the closure, canonical items with a same
  // core are added multiple times (sometimes with different lookup symbol, for
  // efficiency, I want to combine such rules)

  // Map [pr -> Map [dot-index -> lookup-syms]]
  unordered_map<const ProductionRule*, unordered_map<int, set<Symbol*>>>
      closureMap;

  // Add items from current set to the closure
  for (Item* it : this->items) {
    if (closureMap.find(it->pr) != closureMap.end() &&
        closureMap[it->pr].find(it->dotIndex) != closureMap[it->pr].end()) {
      for (Symbol* sym : it->lookup) {
        closureMap[it->pr][it->dotIndex].insert(sym);
      }
    } else {
      closureMap[it->pr][it->dotIndex] = it->lookup;
    }
  }

  // stores the rules that are to be processed
  queue<Item> processingQueue;
  for (const auto& pr : closureMap) {
    for (const auto& item : pr.second) {
      processingQueue.push(Item(pr.first, item.first, item.second));
    }
  }

  // process items in the processingQueue
  while (!processingQueue.empty()) {
    const Item& item = processingQueue.front();
    processingQueue.pop();

    // find out the rules added by the item
    auto itemClosure =
        this->closureOneItem(item, productionRules, firstSetsMap);
    for (const auto& newItem : itemClosure) {
      // cout << "[DEBUG] ";
      // newItem.print();

      bool doesItemExist = false;
      // -> attempt to add them to the closureMap and
      if (closureMap.find(newItem.pr) != closureMap.end() &&
          closureMap[newItem.pr].find(newItem.dotIndex) !=
              closureMap[newItem.pr].end()) {
        // Item with the same core is present, check for additional
        // lookup/lookahead symbols
        doesItemExist = true;
        for (Symbol* sym : newItem.lookup) {
          if (closureMap[newItem.pr][newItem.dotIndex].find(sym) ==
              closureMap[newItem.pr][newItem.dotIndex].end()) {
            doesItemExist = false;
            closureMap[newItem.pr][newItem.dotIndex].insert(sym);
          }
        }
      } else {
        closureMap[newItem.pr][newItem.dotIndex] = newItem.lookup;
      }
      if (!doesItemExist) {
        processingQueue.push(newItem);
      }
      // cout << "[DEBUG] item processed\n";
    }
  }
  // cout << "[DEBUG] Queue processed\n";

  // Note that new items are created!
  // (If this was not done then destructor of the current set will delete the
  // item allocated by it and hence making the ptr in the closure-set invalid)
  set<Item*> closureItems;
  for (const auto& pr : closureMap) {
    for (const auto& item : pr.second) {
      closureItems.insert(new Item(pr.first, item.first, item.second));
    }
  }

  return new SetOfItems(closureItems, this->stateIndex);
}

unordered_set<Symbol*> SetOfItems::getNextSymbols() {
  unordered_set<Symbol*> nextSyms;
  for (Item* item : this->items) {
    if (item->dotIndex < item->pr->rhs.size()) {
      nextSyms.insert(item->pr->rhs[item->dotIndex]);
    }
  }
  return nextSyms;
}

// sets stateIndex to -1 -> it must be changed later on
SetOfItems* SetOfItems::goToNewState(Symbol* sym) const {
  set<Item*> newStateItems;
  for (Item* item : this->items) {
    if (item->dotIndex < item->pr->rhs.size() &&
        item->pr->rhs[item->dotIndex] == sym) {
      newStateItems.insert(
          new Item(item->pr, item->dotIndex + 1, item->lookup));
    }
  }
  return new SetOfItems(newStateItems, -1);
}

string SetOfItems::computeHash() const {
  vector<string> itemHashes;
  for (Item* item : this->items) {
    itemHashes.push_back(item->computeHash());
  }
  sort(itemHashes.begin(), itemHashes.end());
  string hash = "";
  for (string str : itemHashes) {
    hash = str + " | ";
  }
  return hash;
}

void SetOfItems::print() const {
  cout << "=====\n";
  cout << "State-index: " << this->stateIndex << ":\n";
  for (Item* item : this->items) {
    item->print();
    cout << "\n";
    // cout << item->pr << "Dot-index: " << item->dotIndex << " Lookup: [ ";
    // for (Symbol* sym : item->lookup) {
    //   cout << sym->symbol << " ";
    // }
    // cout << "]\n";
  }
  cout << "=====\n";
}
