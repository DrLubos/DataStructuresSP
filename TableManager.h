#include <libds/adt/table.h>
#include <libds/adt/list.h>
#include "RoutingTable.h"
#include <libds/heap_monitor.h>

class TableManager
{
public:
    ds::adt::Table<std::bitset<32>, ds::adt::ImplicitList<RoutingTableRow*>*> * table;
    TableManager();
    ~TableManager();
    void addEntry(std::bitset<32>& key, RoutingTableRow* value);
    void removeEntries(ds::adt::ImplicitList<std::bitset<32>>& ipAddresses);
    void findRowWithKey(std::bitset<32>& key, ds::amt::IS<RoutingTableRow*>& filteringSequence);
};

TableManager::TableManager() {
    //table = new ds::adt::SortedSequenceTable<std::bitset<32>, ds::adt::ImplicitList<RoutingTableRow*>*>();
    //table = new ds::adt::BinarySearchTree<std::bitset<32>, ds::adt::ImplicitList<RoutingTableRow*>*>();
    table = new ds::adt::Treap<std::bitset<32>, ds::adt::ImplicitList<RoutingTableRow*>*>();
}

TableManager::~TableManager() {
    table->clear();
    delete table;
}

void TableManager::addEntry(std::bitset<32>& key, RoutingTableRow* value) {
    ds::adt::ImplicitList<RoutingTableRow*>** place = nullptr;
    if (!table->tryFind(key, place)) {
        ds::adt::ImplicitList<RoutingTableRow*>* sequence = new ds::adt::ImplicitList<RoutingTableRow*>();
        table->insert(key, sequence);
        sequence->insertLast(value);
    } else {
        (*place)->insertLast(value);
    }
}

void TableManager::removeEntries(ds::adt::ImplicitList<std::bitset<32>>& ipAddresses) {
    ds::adt::ImplicitList<std::bitset<32>>::IteratorType begin = ipAddresses.begin();
    ds::adt::ImplicitList<std::bitset<32>>::IteratorType end = ipAddresses.end();
    while (begin != end) {
        ds::adt::ImplicitList<RoutingTableRow*>** place = nullptr;
        table->tryFind(*begin, place);
        delete *place;
        ++begin;
    }
}

void TableManager::findRowWithKey(std::bitset<32>& key, ds::amt::IS<RoutingTableRow*>& filteringSequence) {
    ds::adt::ImplicitList<RoutingTableRow*>** place = nullptr;
    table->tryFind(key, place);
    if (*place != nullptr) {
        ds::adt::ImplicitList<RoutingTableRow*>::IteratorType begin = (*place)->begin();
        ds::adt::ImplicitList<RoutingTableRow*>::IteratorType end = (*place)->end();
        while (begin != end) {
            filteringSequence.insertLast().data_ = *begin;
            ++begin;
        }
    }
}
