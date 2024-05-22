#include "Filter.h"
#include "Loader.h"
#include "UserInteraction.h"
#include "SortingManager.h"
#include <libds/heap_monitor.h>

void mainLoop(std::vector<RoutingTableRow>& loadedRoutingTable, HierarchyManager& hierarchyManager, TableManager& tableManager) {
    ds::amt::IS<RoutingTableRow*> filteringSequence;
    ds::amt::IS<Node*> hierarchyFilteringSequence;
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
        ds::amt::MultiWayEH<Node>::PreOrderHierarchyIterator begin(&hierarchyManager.hierarchy, actualNode);
        ds::amt::MultiWayEH<Node>::PreOrderHierarchyIterator end(&hierarchyManager.hierarchy, nullptr);
        try {
            option = std::stoi(optionString);
        } catch (const std::exception& e) {
            option = -1;
        }
        std::string filename;
        if ((option > -1 && option < 3) || option == 21 || (option > 9 && option < 13)) {
            filteringSequence.clear();
        }
        switch (option) {
            case -1:
                std::cout << "Exiting..." << std::endl;
                return;
            case 0:
                Filter::chooseAddress(ipAddressToCompare);
                Filter::chooseLifetime(startingLifetime, endingLifetime);
                Filter::filterEntries(loadedRoutingTable.begin(), loadedRoutingTable.end(),
                                      [&](const RoutingTableRow& row) {
                                          return matchWithAddress(row, ipAddressToCompare) &&
                                                 matchLifetime(row, startingLifetime, endingLifetime);
                                      }, filteringSequence);
                break;
            case 1:
                Filter::chooseAddress(ipAddressToCompare);
                Filter::filterEntries(loadedRoutingTable.begin(), loadedRoutingTable.end(),
                                      [&](const RoutingTableRow& row) {
                                          return matchWithAddress(row, ipAddressToCompare);
                                      }, filteringSequence);
                break;
            case 2:
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
                UserInteraction::printFilteredSequence(filteringSequence);
                break;
            case 6:
                if (!filteringSequence.isEmpty()) {
                    UserInteraction::savingPrompt(filteringSequence, filename);
                } else {
                    std::cout << "No filtered routing table values to save!" << std::endl;
                }
                break;
            case 10:
                std::cout << "Your IP should start with: " << hierarchyManager.getOctetsToNode(*actualNode) << std::endl;
                Filter::chooseAddress(ipAddressToCompare);
                Filter::chooseLifetime(startingLifetime, endingLifetime);
                Filter::filterEntries(begin, end, [&](const Node& node) {
                    return matchWithAddressHierarchy(node, ipAddressToCompare) &&
                           matchLifetimeHierarchy(node, startingLifetime, endingLifetime);
                }, hierarchyFilteringSequence);
                break;
            case 11:
                std::cout << "Your IP should start with: " << hierarchyManager.getOctetsToNode(*actualNode) << std::endl;
                Filter::chooseAddress(ipAddressToCompare);
                Filter::filterEntries(begin, end, [&](const Node& node) {
                    return matchWithAddressHierarchy(node, ipAddressToCompare);
                }, hierarchyFilteringSequence);
                break;
            case 12:
                Filter::chooseLifetime(startingLifetime, endingLifetime);
                Filter::filterEntries(begin, end, [&](const Node& node) {
                    return matchLifetimeHierarchy(node, startingLifetime, endingLifetime);
                }, hierarchyFilteringSequence);
                break;
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
                    option = -10;
                }
                if (option < 0 || option >= actualNode->sons_->size()) {
                    std::cout << "Invalid son number!" << std::endl;
                    break;
                }
                actualNode = actualNode->sons_->access(option)->data_;
                option = -10;
                break;
            case 16:
                hierarchyManager.printNodeInfo(*actualNode);
                hierarchyManager.print(*actualNode);
                break;
            case 21:
                Filter::chooseAddress(ipAddressToCompare);
                tableManager.findRowWithKey(ipAddressToCompare, filteringSequence);
                break;
            case 31:
                SortingManager::sortData(filteringSequence, comparePrefix);
                break;
            case 32:
                SortingManager::sortData(filteringSequence, compareTime);
                break;
            default:
                std::cout << "Invalid option!" << std::endl;
                break;
        }
        if (option > 9 && option < 13) {
            UserInteraction::hierarchyFilteredToNormalSequence(hierarchyFilteringSequence, filteringSequence);
            hierarchyFilteringSequence.clear();
        }
        if ((option > -1 && option < 3) || option == 21 || (option > 9 && option < 13)) {
            UserInteraction::printFilteredSequence(filteringSequence);
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