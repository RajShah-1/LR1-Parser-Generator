#ifndef __SET_OF_ITEMS_HPP__
#define __SET_OF_ITEMS_HPP__

#include <algorithm>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "../Common/constants.hpp"
#include "../Common/structs.hpp"

using namespace std;

/**
 * - LR1 Canonical Item, contains:
 *  - const ProductionRule* : a production rule
 *  - int: index of the dot
 *  - set<Symbol*> : lookup symbols
 * */
struct Item {
  const ProductionRule* pr;
  int dotIndex;
  set<Symbol*> lookup;
  Item(const ProductionRule* pr, int dotIndex, const set<Symbol*>& lookup);
  string computeHash() const;
  void print() const;
};

/**
 * - Defines a set of canonical LR(1) items with functions to help find closure
 * and goto
 * */
class SetOfItems {
  // An ordered set ensures an ordering and hence hash computation will not be
  // affected by the order in which the items were added to the set
  set<Item*> items;
  int stateIndex;

  vector<Item> closureOneItem(
      const Item& it,
      const unordered_map<Symbol*, unordered_set<ProductionRule*>>&
          productionRules,
      const unordered_map<Symbol*, unordered_set<Symbol*>>& firstSetsMap) const;

 public:
  SetOfItems(const set<Item*>& items, int stateIndex);
  ~SetOfItems();
  int getStateIndex() const;
  void setStateIndex(int stateId);
  // void addItem(Item* item);
  unordered_set<Symbol*> getNextSymbols();
  SetOfItems* getClosure(
      const unordered_map<Symbol*, unordered_set<ProductionRule*>>&
          productionRules,
      const unordered_map<Symbol*, unordered_set<Symbol*>>& firstSetsMap) const;
  SetOfItems* goToNewState(Symbol* sym) const;
  string computeHash() const;
  void print() const;
};

#endif