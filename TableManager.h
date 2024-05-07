#include <libds/amt/implicit_sequence.h>
#include <libds/adt/table.h>
#include <libds/adt/list.h>
#include "RoutingTable.h"
#include <libds/heap_monitor.h>
#include <bitset>

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
    table = new ds::adt::SortedSequenceTable<std::bitset<32>, ds::adt::ImplicitList<RoutingTableRow*>*>();
}

TableManager::~TableManager() {
    std::vector<std::string> ips = {
            "128.223.51.1",
            "12.0.1.63",
            "114.31.199.16",
            "202.232.0.2",
            "144.228.241.130",
            "4.68.4.46",
            "64.71.137.241",
            "212.66.96.126",
            "217.192.89.50",
            "154.11.12.212",
            "203.181.248.195",
            "89.149.178.10",
            "94.142.247.3",
            "209.124.176.223",
            "132.198.255.253",
            "193.0.0.56",
            "37.139.139.17",
            "91.218.184.60",
            "208.51.134.254",
            "162.250.137.254",
            "206.24.210.80"
    };
    // for (auto k : ips) {
    //     std::bitset<32> key = RoutingTableOperations::processIPAddress(k, nullptr);
    //     auto value = table->find(key);
    //     delete value;
    // }
    // table->find(RoutingTableOperations::processIPAddress("128.223.51.1", nullptr));
    // table->find(RoutingTableOperations::processIPAddress("12.0.1.63", nullptr));
    // table->find(RoutingTableOperations::processIPAddress("114.31.199.16", nullptr));
    // table->find(RoutingTableOperations::processIPAddress("202.232.0.2", nullptr));
    // table->find(RoutingTableOperations::processIPAddress("144.228.241.130", nullptr));
    // table->find(RoutingTableOperations::processIPAddress("4.68.4.46", nullptr));
    // table->find(RoutingTableOperations::processIPAddress("64.71.137.24", nullptr));
    // table->find(RoutingTableOperations::processIPAddress("212.66.96.126", nullptr));
    // table->find(RoutingTableOperations::processIPAddress("217.192.89.50", nullptr));
    // table->find(RoutingTableOperations::processIPAddress("154.11.12.212", nullptr));
    // table->find(RoutingTableOperations::processIPAddress("203.181.248.195", nullptr));
    // table->find(RoutingTableOperations::processIPAddress("89.149.178.10", nullptr));
    // table->find(RoutingTableOperations::processIPAddress("94.142.247.3", nullptr));
    // table->find(RoutingTableOperations::processIPAddress("209.124.176.223", nullptr));
    // table->find(RoutingTableOperations::processIPAddress("132.198.255.253", nullptr));
    // table->find(RoutingTableOperations::processIPAddress("193.0.0.56", nullptr));
    // table->find(RoutingTableOperations::processIPAddress("37.139.139.17", nullptr));
    // table->find(RoutingTableOperations::processIPAddress("91.218.184.60", nullptr));
    // table->find(RoutingTableOperations::processIPAddress("208.51.134.254", nullptr));
    // table->find(RoutingTableOperations::processIPAddress("162.250.137.254", nullptr));
    // table->find(RoutingTableOperations::processIPAddress("114.31.199.16", nullptr));
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
