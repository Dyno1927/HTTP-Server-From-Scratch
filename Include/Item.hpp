#ifndef ITEM_HPP
#define ITEM_HPP

#include <string>

// one store entry. kept tiny on purpose — fields grow only when a phase needs
// them (Phase 7 might add a price, etc.).
struct Item
{
    int id = 0;
    std::string name;
};

#endif
