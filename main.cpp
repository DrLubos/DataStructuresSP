#include "RoutingTable.h"
#include "Filter.h"
#include "Loader.h"

void selectNameOfCSVFile(std::string& defaultName) {
    std::cin.ignore();
    std::cout << "Insert filename (Press 'Enter' to use default name '" << defaultName << "' or 'Escape' and then 'Enter' to abort): ";
    std::string filename;
    std::getline(std::cin, filename);
    if (filename.empty()) {
        filename = defaultName;
    }
    if (!filename.empty() && filename[0] == '\x1b') {
        defaultName = "";
        return;
    }
    if (filename.size() < 4 || filename.substr(filename.size() - 4) != ".csv") {
        filename += ".csv";
    }
    defaultName = filename;
}

template<typename P>
void savingPrompt(std::vector<P>& routingTableVector, std::string& filename) {
    if (routingTableVector.size() > 0) {
        char choice;
        std::cout << "Do you want to save the filtered table to a CSV file? (y/n):";
        std::cin >> choice;
        if (choice == 'y' || choice == 'Y') {
            filename = "RT_Filtered.csv";
            selectNameOfCSVFile(filename);
            if (filename != "") {
                RoutingTableOperations::saveToCSV(filename, routingTableVector);
                std::cout << "Filtered routing table saved to " << filename << std::endl;
            }
            else {
                std::cout << "Saving cancelled!" << std::endl;
            }
        }
    }
}

void hierarchyLoop(MyHierarchy& hierarchy) {
    Filter filter;
    std::vector<RoutingTableRow*> filtering;
    std::string optionString;
    int option;
    bool autaSavePrompt = true;
    auto root = *hierarchy.hierarchy.accessRoot();
    auto actualNode = *root.sons_->accessFirst()->data_;
    do {
        std::cout << std::endl << "Options:\n\t[0] Filter by matching IP address and lifetime" << std::endl;
        std::cout << "\t[1] Filter by matching IP address" << std::endl;
        std::cout << "\t[2] Filter by matching lifetime" << std::endl;
        std::cout << "\t[3] Print Node information" << std::endl;
        std::cout << "\t[4] Go to parent" << std::endl;
        std::cout << "\t[5] Go to son" << std::endl;
        std::cout << "\t[6] Print leafs" << std::endl;
        std::cout << "\t[8] Exit hierarchy mode" << std::endl;
        std::cout << "\t[9] Exit program" << std::endl;
        unsigned int startingLifetime = 0;
        unsigned int endingLifetime = UINT_MAX;
        std::bitset<32> ipAddressToCompare;
        std::cout << "Your option: ";
        std::cin >> optionString;
        std::cout << "------------------------------------------" << std::endl;
        try {
            option = std::stoi(optionString);
        } catch (const std::exception& e) {
            option = -1;
        }
        switch (option) {
            case 0:
                std::cout << "Your IP should start with: " << hierarchy.getOctetsToNode(actualNode) << std::endl;
                Filter::chooseAddress(ipAddressToCompare);
                Filter::chooseLifetime(startingLifetime, endingLifetime);
                hierarchy.hierarchy.processPostOrder(&actualNode, std::function<void(ds::amt::MultiWayExplicitHierarchyBlock<Node>*)>([&](ds::amt::MultiWayExplicitHierarchyBlock<Node>* node) {
                    if (node->data_.pData != nullptr) {
                        if (matchLifetime(*node->data_.pData, startingLifetime, endingLifetime) && matchWithAddress(*node->data_.pData, ipAddressToCompare)) {
                            filtering.push_back(node->data_.pData);
                        }
                    }
                }));
                break;
            case 3:
                hierarchy.printNodeInfo(actualNode);
                break;
            case 4:
                 if (actualNode.parent_ != nullptr) {
                    auto a = *actualNode.parent_;
                    std::cout << "a " << typeid(a).name() << std::endl;
                    std::cout << "actualNode " << typeid(actualNode).name() << std::endl;
                    auto b = *actualNode.parent_->parent_;
                }
                else {
                    std::cout << "You are at the root node!" << std::endl;
                }
                break;
            case 5:
                hierarchy.printNodeInfo(actualNode);
                if (actualNode.sons_->isEmpty()) {
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
                actualNode = *actualNode.sons_->access(option)->data_;
                break;
            case 6:
                hierarchy.printNodeInfo(actualNode);
                hierarchy.print(actualNode);
                break;
            case 8:
                option = -2;
                break;
            case 9:
                std::cout << "Exiting..." << std::endl;
                hierarchy.hierarchy.clear();
                return;
            default:
                std::cout << "Invalid option!" << std::endl;
        }
        if (option == -2) {
            break;
        }
        option = -1;
    } while (true);
}

void mainLoop(std::vector<RoutingTableRow>& loadedRoutingTable, MyHierarchy& hierarchy) {
    Filter filter;
    std::vector<RoutingTableRow*> filtering;
    std::string optionString;
    int option;
    bool autaSavePrompt = true;
    do {
        std::cout << std::endl << "Options:\n\t[0] Filter by matching IP address and lifetime" << std::endl;
        std::cout << "\t[1] Filter by matching IP address" << std::endl;
        std::cout << "\t[2] Filter by matching lifetime" << std::endl;
        std::cout << "\t[3] Save whole table to CSV" << std::endl;
        std::cout << "\t[4] Print whole routing table" << std::endl;
        std::cout << "\t[5] Save filtered routing table to CSV" << std::endl;
        std::cout << "\t[6] Print filtered routing table" << std::endl;
        autaSavePrompt ? std::cout << "\t[7] Disable auto save prompt" << std::endl : std::cout << "\t[7] Enable auto save prompt" << std::endl;
        std::cout << "\t[8] Hierarchy mode" << std::endl;
        std::cout << "\t[9] To exit program" << std::endl;
        std::cout << "Your option: ";
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
                filename = "RT_Loaded.csv";
                selectNameOfCSVFile(filename);
                if (filename == "") {
                    std::cout << "Save cancelled!" << std::endl;
                }
                else {
                    RoutingTableOperations::saveToCSV(filename, loadedRoutingTable);
                    std::cout << "Loaded routing table saved to " << filename << std::endl;
                }
                break;
            case 4:
                RoutingTableOperations::print(loadedRoutingTable);
                std::cout << "------------------------------------------\nPrinted " << loadedRoutingTable.size() << " values." << std::endl;
                break;
            case 5:
                if (filtering.size() > 0) {
                    savingPrompt(filtering, filename);
                }
                else {
                    std::cout << "No filtered routing table values to save!" << std::endl;
                }
                break;
            case 6:
                RoutingTableOperations::print(filtering);
                std::cout << "------------------------------------------\nPrinted " << filtering.size() << " values." << std::endl;
                break;
            case 7:
                autaSavePrompt = !autaSavePrompt;
                autaSavePrompt ? std::cout << "Auto printing filtered table enabled!" << std::endl : std::cout << "Auto printing filtered table disabled!" << std::endl;
                break;
            case 8:
                hierarchyLoop(hierarchy);
                break;
            case 9:
                std::cout << "Exiting..." << std::endl;
                hierarchy.hierarchy.clear();
                return;
            default:
                std::cout << "Invalid option!" << std::endl;
        }
        if (option > -1 && option < 3) {
            RoutingTableOperations::print(filtering);
            std::cout << "------------------------------------------\nFound " << filtering.size() << " values." << std::endl;
            if (autaSavePrompt) {
                savingPrompt(filtering, filename);
            }
        }
        option = -1;
    } while (true);
}

int main() {
    std::vector<RoutingTableRow> loadedRoutingTable;
    MyHierarchy hierarchy;
    try {
        Loader::loadFromCSV("RT.csv", loadedRoutingTable, hierarchy);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    std::cout << "Routing table loaded successfully! Values in vector: " << loadedRoutingTable.size() << ". Values in hierarchy: " << hierarchy.hierarchy.size() << std::endl;
    mainLoop(loadedRoutingTable, hierarchy);
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
// vytvorit metodu na pohyb v hiearchii a nejaky atrubut, ktory sa bude aktualizovat pri pohybe