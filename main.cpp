#include "RoutingTable.h"

void selectNameOfCSVFile(std::string& defaultName) {
    std::cin.ignore();
    std::cout << "Insert filename (Press Enter to use default name '" << defaultName << "'): ";
    std::string filename;
    std::getline(std::cin, filename);
    if (filename.empty()) {
        filename = defaultName;
    }
    if (filename.size() < 4 || filename.substr(filename.size() - 4) != ".csv") {
        filename += ".csv";
    }
    defaultName = filename; 
}

void savingPrompt(RoutingTable* routingTable, std::vector<RoutingTableRow>& routingTableVector, std::string& filename) {
    routingTable->print(routingTableVector);
    std::cout << "------------------------------------------\nFound " << routingTableVector.size() << " values." << std::endl;
    char choice;
    if (routingTableVector.size() > 0) {
        std::cout << "Do you want to save the filtered table to a CSV file? (y/n):";
        std::cin >> choice;
        if (choice == 'y' || choice == 'Y') {
            filename = "RT_Filtered.csv";
            selectNameOfCSVFile(filename);
            routingTable->saveToCSV(filename, routingTableVector);
            std::cout << "Filtered routing table saved to " << filename << std::endl;
        }
    }
}

void filterByLifetime(RoutingTable* routingTable, std::vector<RoutingTableRow>& routingTableVector) {
    std::string start;
    std::string end;
    std::cout << "Insert minimum possible starting lifetime (s) or (XwXdXhXmXs) or (HH:MM:SS): ";
    std::cin >> start;
    std::cout << "Insert maximum possible ending lifetime (s) or (XwXdXhXmXs) or (HH:MM:SS) or (-1) to check without maximum possible ending: ";
    std::cin >> end;
    if (end[0] == '-' && end[1] == '1'){
        end = std::to_string(std::numeric_limits<unsigned int>::max());
    }
    std::vector<RoutingTableRow> tempFilteredTable = routingTable->matchLifetime(start, end);
    std::for_each(tempFilteredTable.begin(), tempFilteredTable.end(), [&routingTableVector](RoutingTableRow& row) {
        routingTableVector.push_back(row);
    });
}

void filterByAddress(RoutingTable* routingTable, std::vector<RoutingTableRow>& routingTableVector) {
    std::string ipAddressString;
    std::cout << "Insert IP address to filter: ";
    std::cin >> ipAddressString;
    routingTableVector = routingTable->matchWithAddress(ipAddressString);
}

int main() {
    RoutingTable* routingTable = new RoutingTable();
    try {
        routingTable->loadFromCSV("RT.csv");
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        delete routingTable;
        routingTable = nullptr;
        return 1;
    }
    std::vector<RoutingTableRow> loadedRoutingTable = routingTable->getRoutingTable();
    std::vector<RoutingTableRow> routingTableVector;
    std::cout << "Routing table loaded successfully!" << std::endl;
    std::string optionString;
    int option;
    do {
        std::cout << "Options:\n\t[1] Filter by mathing IP address and lifetime (Current filtered table will be deleted!)" << std::endl;
        std::cout << "\t[2] Filter by matching IP address (Current filtered table will be deleted!)" << std::endl;
        std::cout << "\t[3] Filter by matching lifetime (Current filtered table will be deleted!)" << std::endl;
        std::cout << "\t[4] Save whole table to CSV" << std::endl;
        std::cout << "\t[5] Print whole routing table" << std::endl;
        std::cout << "\t[6] Save filtered routing table to CSV" << std::endl;
        std::cout << "\t[7] Print filtered routing table" << std::endl;
        std::cout << "\t[9] To exit program" << std::endl;
        std::cin >> optionString;
        try {
            option = std::stoi(optionString);
        } catch (const std::exception& e) {
            option = 0;
        }
        std::string filename;
        switch (option) {
            case 1:
                routingTableVector.clear();
                filterByAddress(routingTable, routingTableVector);
                filterByLifetime(routingTable, routingTableVector);
                savingPrompt(routingTable, routingTableVector, filename);
                break;
            case 2:
                routingTableVector.clear();
                filterByAddress(routingTable, routingTableVector);
                savingPrompt(routingTable, routingTableVector, filename);
                break;
            case 3:
                routingTableVector.clear();
                filterByLifetime(routingTable, routingTableVector);
                savingPrompt(routingTable, routingTableVector, filename);
                break;
            case 4:
                filename = "RT_Loaded.csv";
                selectNameOfCSVFile(filename);
                routingTable->saveToCSV(filename, loadedRoutingTable);
                std::cout << "Loaded routing table saved to " << filename << std::endl;
                break;
            case 5:
                routingTable->print(loadedRoutingTable);
                std::cout << "------------------------------------------\nPrinted " << loadedRoutingTable.size() << " rows." << std::endl;
                break;
            case 6:
                if (routingTableVector.size() > 0) {
                    savingPrompt(routingTable, routingTableVector, filename);
                } else {
                    std::cout << "No filtered routing table values to save!" << std::endl;
                }
                break;
            case 7:
                if (routingTableVector.size() > 0) {
                    routingTable->print(routingTableVector);
                    std::cout << "------------------------------------------\nPrinted " << routingTableVector.size() << " rows." << std::endl;
                } else {
                    std::cout << "No filtered routing table values to print!" << std::endl;
                }
                break;
            case 9:
                std::cout << "Exiting..." << std::endl;
                delete routingTable;
                routingTable = nullptr;
                return 0;
            default:
                std::cout << "Invalid option!" << std::endl;
        }
        option = 0;
    } while (true);
    return 0;
}