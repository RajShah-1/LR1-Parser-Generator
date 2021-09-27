#ifndef __SET_OF_ITEMS_HPP__
#define __SET_OF_ITEMS_HPP__

#include <queue>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "../Common/structs.hpp"
#include "../Common/constants.hpp"

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

 public:
  SetOfItems(const set<Item*>& items, int stateIndex);
  ~SetOfItems();
  int getStateIndex() const;
  void setStateIndex(int stateId);
  // void addItem(Item* item);
  SetOfItems getClosure(
      unordered_map<Symbol*, unordered_set<ProductionRule*>>& productionRules,
      unordered_map<Symbol*, unordered_set<Symbol*>>& firstSetsMap) const;
  SetOfItems goToNewState(Symbol* sym) const;
  string computeHash() const;
  void print() const;
};

#endif