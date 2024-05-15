#include "Filter.h"
#include "Loader.h"
#include "UserInteraction.h"
#include <libds/heap_monitor.h>

auto matchLifetimeHierarchy = [](const Node& node, unsigned int startTime, unsigned int endTime) {
    if (node.pData != nullptr) {
        if (node.pData->lifetime >= startTime && node.pData->lifetime <= endTime) {
            return true;
        }
        return false;
    }
    return false;
};

auto matchWithAddressHierarchy = [](const Node& node, const std::bitset<32>& addressToCompare) {
    if (node.pData != nullptr) {
        std::bitset<32> ipAddress = node.pData->ipAddress;
        for (int i = 0; i < node.pData->prefix; ++i) {
            if (ipAddress[ipAddress.size() - i - 1] != addressToCompare[addressToCompare.size() - i - 1]) {
                return false;
            }
        }
        return true;
    }
    return false;
};

void mainLoop(std::vector<RoutingTableRow>& loadedRoutingTable, HierarchyManager& hierarchyManager, TableManager& tableManager) {
    ds::amt::IS<RoutingTableRow*> filteringSequence;
    ds::amt::IS<Node*> filteringSequenceHierarchy;
    std::string optionString;
    int option;
    auto* actualNode = hierarchyManager.hierarchy.accessRoot();
    do {
        UserInteraction::printOptions();
        std::cin >> optionString;
        std::cout << "------------------------------------------" << std::endl;
        unsigned int startingLifetime = 0;
        unsigned int endingLifetime = UINT_MAX;
        std::bitset<32> ipAddressToCompare;
        try {
            option = std::stoi(optionString);
        } catch (const std::exception& e) {
            option = -1;
        }
        std::string filename;
        switch (option) {
            case -1:
                std::cout << "Exiting..." << std::endl;
                return;
            case 0:
                filteringSequence.clear();
                Filter::chooseAddress(ipAddressToCompare);
                Filter::chooseLifetime(startingLifetime, endingLifetime);
                Filter::filterEntries(loadedRoutingTable.begin(), loadedRoutingTable.end(),
                                      [&](const RoutingTableRow& row) {
                                          return matchWithAddress(row, ipAddressToCompare) &&
                                                 matchLifetime(row, startingLifetime, endingLifetime);
                                      }, filteringSequence);
                break;
            case 1:
                filteringSequence.clear();
                Filter::chooseAddress(ipAddressToCompare);
                Filter::filterEntries(loadedRoutingTable.begin(), loadedRoutingTable.end(),
                                      [&](const RoutingTableRow& row) {
                                          return matchWithAddress(row, ipAddressToCompare);
                                      }, filteringSequence);
                break;
            case 2:
                filteringSequence.clear();
                Filter::chooseLifetime(startingLifetime, endingLifetime);
                Filter::filterEntries(loadedRoutingTable.begin(), loadedRoutingTable.end(),
                                      [&](const RoutingTableRow& row) {
                                          return matchLifetime(row, startingLifetime, endingLifetime);
                                      }, filteringSequence);
                break;
            case 3:
                RoutingTableOperations::print(loadedRoutingTable);
                std::cout << "------------------------------------------\nPrinted " << loadedRoutingTable.size() << " values." << std::endl;
                break;
            case 4:
                filename = "RT_Loaded.csv";
                UserInteraction::selectNameOfCSVFile(filename);
                if (filename.empty()) {
                    std::cout << "Save cancelled!" << std::endl;
                } else {
                    RoutingTableOperations::saveToCSV(filename, loadedRoutingTable);
                    std::cout << "Loaded routing table saved to " << filename << std::endl;
                }
                break;
            case 5:
                if (filteringSequence.isEmpty()) {
                    option = 10;
                } else {
                    option = 0;
                }
                break;
            case 6:
                if (!filteringSequence.isEmpty()) {
                    UserInteraction::savingPrompt(filteringSequence, filename);
                } else {
                    std::cout << "No filtered routing table values to save!" << std::endl;
                }
                break;
            case 10:
            {
                filteringSequenceHierarchy.clear();
                std::cout << "Your IP should start with: " << hierarchyManager.getOctetsToNode(*actualNode) << std::endl;
                Filter::chooseAddress(ipAddressToCompare);
                Filter::chooseLifetime(startingLifetime, endingLifetime);
                ds::amt::MultiWayExplicitHierarchy<Node>::PreOrderHierarchyIterator begin(&hierarchyManager.hierarchy, actualNode);
                ds::amt::MultiWayExplicitHierarchy<Node>::PreOrderHierarchyIterator end(&hierarchyManager.hierarchy, nullptr);
                Filter::filterEntries(begin, end, [&](const Node& node) {
                    return matchWithAddressHierarchy(node, ipAddressToCompare) &&
                           matchLifetimeHierarchy(node, startingLifetime, endingLifetime);
                }, filteringSequenceHierarchy);
                break;
            }
            case 11:
            {
                filteringSequenceHierarchy.clear();
                std::cout << "Your IP should start with: " << hierarchyManager.getOctetsToNode(*actualNode) << std::endl;
                Filter::chooseAddress(ipAddressToCompare);
                ds::amt::MultiWayExplicitHierarchy<Node>::PreOrderHierarchyIterator begin(&hierarchyManager.hierarchy, actualNode);
                ds::amt::MultiWayExplicitHierarchy<Node>::PreOrderHierarchyIterator end(&hierarchyManager.hierarchy, nullptr);
                Filter::filterEntries(begin, end, [&](const Node& node) {
                    return matchWithAddressHierarchy(node, ipAddressToCompare);
                }, filteringSequenceHierarchy);
                break;
            }
            case 12:
            {
                filteringSequenceHierarchy.clear();
                Filter::chooseLifetime(startingLifetime, endingLifetime);
                ds::amt::MultiWayExplicitHierarchy<Node>::PreOrderHierarchyIterator begin(&hierarchyManager.hierarchy, actualNode);
                ds::amt::MultiWayExplicitHierarchy<Node>::PreOrderHierarchyIterator end(&hierarchyManager.hierarchy, nullptr);
                Filter::filterEntries(begin, end, [&](const Node& node) {
                    return matchLifetimeHierarchy(node, startingLifetime, endingLifetime);
                }, filteringSequenceHierarchy);
                break;
            }
            case 13:
                hierarchyManager.printNodeInfo(*actualNode);
                break;
            case 14:
                if (actualNode->parent_ != nullptr) {
                    actualNode = hierarchyManager.hierarchy.accessParent(*actualNode);
                } else {
                    std::cout << "You are at the root node!" << std::endl;
                }
                break;
            case 15:
                hierarchyManager.printNodeInfo(*actualNode);
                hierarchyManager.printSons(*actualNode);
                if (actualNode->sons_->isEmpty()) {
                    std::cout << "No sons to go to!" << std::endl;
                    break;
                }
                std::cout << "Choose son (#) to go to: ";
                std::cin >> optionString;
                try {
                    option = std::stoi(optionString);
                } catch (const std::exception& e) {
                    option = -1;
                }
                if (option < 0 || option >= actualNode->sons_->size()) {
                    std::cout << "Invalid son number!" << std::endl;
                    break;
                }
                actualNode = actualNode->sons_->access(option)->data_;
                option = -1;
                break;
            case 16:
                hierarchyManager.printNodeInfo(*actualNode);
                hierarchyManager.print(*actualNode);
                break;
            case 21:
                filteringSequence.clear();
                Filter::chooseAddress(ipAddressToCompare);
                tableManager.findRowWithKey(ipAddressToCompare, filteringSequence);
                break;
            default:
                std::cout << "Invalid option!" << std::endl;
        }
        if (option > -1 && option < 3 || option == 21) {
            filteringSequenceHierarchy.clear();
            if (!filteringSequence.isEmpty()) {
                ds::amt::IS<RoutingTableRow*>::ImplicitSequenceIterator begin = filteringSequence.begin();
                ds::amt::IS<RoutingTableRow*>::ImplicitSequenceIterator end = filteringSequence.end();
                for (auto it = begin; it != end; ++it) {
                    RoutingTableOperations::printRow(*(*it));
                }
            }
            std::cout << "------------------------------------------\nPrinted " << filteringSequence.size() << " values." << std::endl;
        }
        if (option > 9 && option < 13) {
            filteringSequence.clear();
            if (!filteringSequenceHierarchy.isEmpty()) {
                ds::amt::IS<Node*>::ImplicitSequenceIterator begin = filteringSequenceHierarchy.begin();
                ds::amt::IS<Node*>::ImplicitSequenceIterator end = filteringSequenceHierarchy.end();
                for (auto it = begin; it != end; ++it) {
                    RoutingTableOperations::printRow(*(*it)->pData);
                }
            }
            std::cout << "------------------------------------------\nPrinted " << filteringSequenceHierarchy.size() << " values." << std::endl;
        }
        option = -10;
    } while (true);
}

void deleteTable(std::vector<RoutingTableRow>& loadedRoutingTable, TableManager& tableManager) {
    ds::adt::ImplicitList<std::bitset<32>> destAddresses;
    for (auto& row : loadedRoutingTable) {
        if (!destAddresses.contains(row.destinationIP)) {
            destAddresses.insertLast(row.destinationIP);
        }
    }
    tableManager.removeEntries(destAddresses);
}

int main() {
    //initHeapMonitor();
    std::vector<RoutingTableRow> loadedRoutingTable;
    HierarchyManager hierarchyManager;
    TableManager tableManager;
    try {
        Loader::loadFromCSV("RT.csv", loadedRoutingTable, hierarchyManager, tableManager);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    std::cout << "Routing table loaded successfully! Values in vector: " << loadedRoutingTable.size() << ". Values in hierarchy: " << hierarchyManager.hierarchy.size() << ". Table size: " << tableManager.table->size() << std::endl;
    mainLoop(loadedRoutingTable, hierarchyManager, tableManager);
    deleteTable(loadedRoutingTable, tableManager);
    return 0;
}