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
  hash = "]";
  cout << "[DEBUG Item Hash] " << hash << "\n";
  return hash;
}

// Methods for SetOfItems class

SetOfItems::SetOfItems(const set<Item*>& items, int stateIndex)
    : items(items), stateIndex(stateIndex) {}

int SetOfItems::getStateIndex() const { return this->stateIndex; }

void SetOfItems::setStateIndex(int stateId) { this->stateIndex = stateId; }

// void SetOfItems::addItem(Item* item) {}

SetOfItems SetOfItems::getClosure(
    unordered_map<Symbol*, unordered_set<ProductionRule*>>& productionRules,
    unordered_map<Symbol*, unordered_set<Symbol*>>& firstSetsMap) const {
  // Note:
  // It is possible that while finding the closure, canonical items with a same
  // core are added multiple times (sometimes with different lookup symbol, for
  // efficiency, I want to combine such rules)

  // Map [pr -> Map [dot-index -> lookup-syms]]
  unordered_map<const ProductionRule*, unordered_map<int, set<Symbol*>>>
      closureMap;

  // Map [pr -> lookup-syms]
  // stores the rules that are to be processed (dotIndex = 1 for all)
  queue<pair<const ProductionRule*, set<Symbol*>>> processingQueue;

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

    // Check if current rule will add new items in the closure
    if (it->dotIndex < it->pr->rhs.size() &&
        !it->pr->rhs[it->dotIndex]->isTerminal) {
      const auto& prodRulesPtr =
          productionRules.find(it->pr->rhs[it->dotIndex]);
      if (prodRulesPtr == productionRules.end()) continue;
      const auto& prodRules = *prodRulesPtr;

      // compute lookup for the rule to be processed
      set<Symbol*> newItemLookup;
      bool isEps = true;
      const auto& itProdRHS = it->pr->rhs;
      for (int i = it->dotIndex + 1; i < itProdRHS.size(); ++i) {
        isEps = false;
        for (Symbol* sym : firstSetsMap[itProdRHS[i]]) {
          if (sym->symbol != EPSILON_SYMBOL) {
            newItemLookup.insert(sym);
          } else {
            isEps = true;
          }
        }
        if (!isEps) break;
      }
      if (isEps) {
        for (Symbol* sym : it->lookup) {
          newItemLookup.insert(sym);
        }
      }

      for (const ProductionRule* prodRule : prodRules.second) {
        processingQueue.push({prodRule, newItemLookup});
      }
    }
  }

  // process items in the processingQueue
  while (!processingQueue.empty()) {
    const auto& item = processingQueue.front();
    processingQueue.pop();
    if (closureMap.find(item.first) != closureMap.end()) {
      if (closureMap[item.first].find(1) != closureMap[item.first].end()) {
        // try to add new lookup syms, if a new sym is added -> a new item must
        // be added
				
      }
    }
  }

  set<Item*> closureItems;
  for (const auto& pr : closureMap) {
    for (const auto& item : pr.second) {
      closureItems.insert(new Item(pr.first, item.first, item.second));
    }
  }
  return SetOfItems(closureItems, this->stateIndex);
}
// SetOfItems SetOfItems::goToNewState(Symbol* sym) const;
// string SetOfItems::computeHash() const;
// void SetOfItems::print() const;
