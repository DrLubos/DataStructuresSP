#include "RoutingTable.h"
#include "Filter.h"
#include "Loader.h"
#include "UserInteraction.h"
#include <libds/heap_monitor.h>

void mainLoop(std::vector<RoutingTableRow>& loadedRoutingTable, HierarchyManager& hierarchyManager) {
    Filter filter;
    std::vector<RoutingTableRow*> filtering;
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
            Filter::chooseAddress(ipAddressToCompare);
            Filter::chooseLifetime(startingLifetime, endingLifetime);
            filtering = filter.filterEntries<RoutingTableRow>(loadedRoutingTable.begin(), loadedRoutingTable.end(), [&](const RoutingTableRow& row) {
                return matchLifetime(row, startingLifetime, endingLifetime) && matchWithAddress(row, ipAddressToCompare);
                });
            break;
        case 1:
            Filter::chooseAddress(ipAddressToCompare);
            filtering = filter.filterEntries<RoutingTableRow>(loadedRoutingTable.begin(), loadedRoutingTable.end(), [&](const RoutingTableRow& row) {
                return matchWithAddress(row, ipAddressToCompare);
                });
            break;
        case 2:
            Filter::chooseLifetime(startingLifetime, endingLifetime);
            filtering = filter.filterEntries<RoutingTableRow>(loadedRoutingTable.begin(), loadedRoutingTable.end(), [&](const RoutingTableRow& row) {
                return matchLifetime(row, startingLifetime, endingLifetime);
                });
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
            RoutingTableOperations::print(filtering);
            std::cout << "------------------------------------------\nPrinted " << filtering.size() << " values." << std::endl;
            break;
        case 6:
            if (!filtering.empty()) {
                UserInteraction::savingPrompt(filtering, filename);
            } else {
                std::cout << "No filtered routing table values to save!" << std::endl;
            }
            break;
        case 10:
            std::cout << "Your IP should start with: " << hierarchyManager.getOctetsToNode(*actualNode) << std::endl;
            Filter::chooseAddress(ipAddressToCompare);
            Filter::chooseLifetime(startingLifetime, endingLifetime);
            filtering.clear();
            hierarchyManager.hierarchy.processPostOrder(actualNode, std::function<void(ds::amt::MultiWayExplicitHierarchyBlock<Node>*)>([&](ds::amt::MultiWayExplicitHierarchyBlock<Node>* node) {
                if (node->data_.pData != nullptr) {
                    if (matchLifetime(*node->data_.pData, startingLifetime, endingLifetime) && matchWithAddress(*node->data_.pData, ipAddressToCompare)) {
                        filtering.push_back(node->data_.pData);
                    }
                }
                }));
            break;
        case 11:
        {
            std::cout << "Your IP should start with: " << hierarchyManager.getOctetsToNode(*actualNode) << std::endl;
            Filter::chooseAddress(ipAddressToCompare);
            filtering.clear();
            //ds::amt::MultiWayExplicitHierarchy<Node>::PreOrderHierarchyIterator begin(&hierarchyManager.hierarchyManager, actualNode);
            //ds::amt::MultiWayExplicitHierarchy<Node>::PreOrderHierarchyIterator end(&hierarchyManager.hierarchyManager, nullptr);
            //filtering = filter.filterEntries<RoutingTableRow *>(begin, end, [&](const Node &node) {
            //    return matchWithAddressHierarchy(node, ipAddressToCompare);
            //});
            hierarchyManager.hierarchy.processPostOrder(actualNode, std::function<void(
                ds::amt::MultiWayExplicitHierarchyBlock<Node> *)>(
                    [&](ds::amt::MultiWayExplicitHierarchyBlock<Node>* node) {
                        if (node->data_.pData != nullptr && hierarchyManager.hierarchy.level(*node) == 4) {
                            if (matchWithAddress(*node->data_.pData, ipAddressToCompare)) {
                                filtering.push_back(node->data_.pData);
                            }
                        }
                    }));
            break;
        }
        case 12:
            Filter::chooseLifetime(startingLifetime, endingLifetime);
            filtering.clear();
            hierarchyManager.hierarchy.processPostOrder(actualNode, std::function<void(ds::amt::MultiWayExplicitHierarchyBlock<Node>*)>([&](ds::amt::MultiWayExplicitHierarchyBlock<Node>* node) {
                if (node->data_.pData != nullptr) {
                    if (matchLifetime(*node->data_.pData, startingLifetime, endingLifetime)) {
                        filtering.push_back(node->data_.pData);
                    }
                }
                }));
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
        default:
            std::cout << "Invalid option!" << std::endl;
        }
        if (option > -1 && option < 3 || option > 9 && option < 13) {
            RoutingTableOperations::print(filtering);
            std::cout << "------------------------------------------\nFound " << filtering.size() << " values." << std::endl;
        }
        option = -1;
    } while (true);
}

int main() {
    initHeapMonitor();
    std::vector<RoutingTableRow> loadedRoutingTable;
    HierarchyManager hierarchyManager;
    try {
        Loader::loadFromCSV("RT.csv", loadedRoutingTable, hierarchyManager);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    std::cout << "Routing table loaded successfully! Values in vector: " << loadedRoutingTable.size() << ". Values in hierarchy: " << hierarchyManager.hierarchy.size() << std::endl;
    mainLoop(loadedRoutingTable, hierarchyManager);
    return 0;
}
/*
Debug
 hierarchy.printNodeInfo(*hierarchy.hierarchy.accessRoot());
    hierarchy.printNodeInfo(*hierarchy.hierarchy.accessRoot()->sons_->accessFirst()->data_);
    hierarchy.printNodeInfo(*hierarchy.hierarchy.accessRoot()->sons_->accessLast()->data_->sons_->accessLast()->data_->sons_->accessLast()->data_);
    std::cout << hierarchy.hierarchy.size() << std::endl;
    std::cout << hierarchy.hierarchy.nodeCount() << std::endl;
*/