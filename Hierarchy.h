#include "RoutingTable.h"
#include <libds/amt/explicit_hierarchy.h>

struct Node {
    Node* parent;
    RoutingTableRow* data;
    std::vector<Node*> children;
    std::bitset<8> octet;
};

// koren  parent = nullptr,  data = nullptr, children != empty(), octet = NULL
// vrchol parent != nullptr, data = nullptr, children != empty()
// list   parent != nullptr, data != nullptr,children = empty()
// Viac listov moze byt viac pointerov (ta ista IP s inou maskou)

// Astah UML