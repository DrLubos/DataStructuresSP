#include "RoutingTable.h"

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

void savingPrompt(RoutingTable* routingTable, std::vector<RoutingTableRow>& routingTableVector, std::string& filename) {
    if (routingTableVector.size() > 0) {
        char choice;
        std::cout << "Do you want to save the filtered table to a CSV file? (y/n):";
        std::cin >> choice;
        if (choice == 'y' || choice == 'Y') {
            filename = "RT_Filtered.csv";
            selectNameOfCSVFile(filename);
            if (filename != "") {
                routingTable->saveToCSV(filename, routingTableVector);
                std::cout << "Filtered routing table saved to " << filename << std::endl;
            } else {
                std::cout << "Saving cancelled!" << std::endl;
            }
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
    routingTable->matchLifetime(start, end, routingTableVector);

}

void filterByAddress(RoutingTable* routingTable, std::vector<RoutingTableRow>& routingTableVector) {
    std::string ipAddressString;
    std::cout << "Insert IP address to filter: ";
    std::cin >> ipAddressString;
    //routingTable->matchWithAddress(ipAddressString, routingTableVector);
}

int main() {
    std::vector<RoutingTableRow> routingTableVector;
    std::vector<RoutingTableRow> filteredRoutingTableVector;
    RoutingTable* routingTable = new RoutingTable();
    try {
        routingTable->loadFromCSV("RT.csv");
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        delete routingTable;
        routingTable = nullptr;
        return 1;
    }
    std::cout << "Routing table loaded successfully!" << std::endl;

    
    //routingTable->fillVector(routingTableVector); 
    routingTableVector= routingTable->getRoutingTable();
    Filter filter;
    filter.filterAndAppend(routingTableVector.begin(), routingTableVector.end(), [&](const RoutingTableRow& row) {
        return matchLifetime(row, 0, 3600);
    }, filteredRoutingTableVector);
    // std::string zaciatok = "0";
    // std::string koniec = "3600";
    // std::vector<RoutingTableRow> routingTableVector;
    // std::vector<RoutingTableRow> filteredRoutingTableVector;
    // RoutingTable* routingTable = new RoutingTable();
    // try {
    //     routingTable->loadFromCSV("RT.csv");
    // } catch (const std::exception& e) {
    //     std::cerr << "Error: " << e.what() << std::endl;
    //     delete routingTable;
    //     routingTable = nullptr;
    //     return 1;
    // }
    // std::cout << "Routing table loaded successfully!" << std::endl;
    // Filter<std::vector<RoutingTableRow>::iterator> filter(routingTableVector.begin(), routingTableVector.end());
    // filter.filterAndAppend([&](const RoutingTableRow& row) {
    //     printf("Row lifetime: %d\n", row.lifetime);
    //     return matchLifetime(row, 0, 3600);
    // }, filteredRoutingTableVector);

    std::cout << filteredRoutingTableVector.size() << std::endl;
    for (auto i : filteredRoutingTableVector) {
        std::cout << i.lifetime << std::endl;
    }
    std::string optionString;
    int option;
    bool autoPrintAndSavePrompt = true;
    do {
        std::cout << std::endl << "Options:\n\t[0] Filter by mathing IP address and lifetime" << std::endl;
        std::cout << "\t[1] Filter by matching IP address" << std::endl;
        std::cout << "\t[2] Filter by matching lifetime" << std::endl;
        std::cout << "\t[3] Save whole table to CSV" << std::endl;
        std::cout << "\t[4] Print whole routing table" << std::endl;
        std::cout << "\t[5] Save filtered routing table to CSV" << std::endl;
        std::cout << "\t[6] Print filtered routing table" << std::endl;
        autoPrintAndSavePrompt ? std::cout << "\t[7] Disable auto printing and asking for save filtered table" << std::endl : std::cout << "\t[7] Enable auto printing and asking for save filtered table" << std::endl;
        std::cout << "\t[8] Reset filtered table" << std::endl;
        std::cout << "\t[9] To exit program" << std::endl;
        std::cout << "Your option: ";
        std::cin >> optionString;
        std::cout << "------------------------------------------" << std::endl;
        try {
            option = std::stoi(optionString);
        } catch (const std::exception& e) {
            option = -1;
        }
        std::string filename;
        switch (option) {
            case 0:
                filterByAddress(routingTable, routingTableVector);
                filterByLifetime(routingTable, routingTableVector);
                if (autoPrintAndSavePrompt) {
                    routingTable->print(routingTableVector);
                    savingPrompt(routingTable, routingTableVector, filename);
                }
                std::cout << "------------------------------------------\nFound " << routingTableVector.size() << " values." << std::endl;
                break;
            case 1:
                filterByAddress(routingTable, routingTableVector);
                if (autoPrintAndSavePrompt) {
                    routingTable->print(routingTableVector);
                    savingPrompt(routingTable, routingTableVector, filename);
                }
                std::cout << "------------------------------------------\nFound " << routingTableVector.size() << " values." << std::endl;
                break;
            case 2:
                filterByLifetime(routingTable, routingTableVector);
                if (autoPrintAndSavePrompt) {
                    routingTable->print(routingTableVector);
                    savingPrompt(routingTable, routingTableVector, filename);
                }
                std::cout << "------------------------------------------\nFound " << routingTableVector.size() << " values." << std::endl;
                break;
            case 3:
                filename = "RT_Loaded.csv";
                selectNameOfCSVFile(filename);
                if (filename == "") {
                    std::cout << "Save cancelled!" << std::endl;
                } else {
                    routingTable->saveToCSV(filename, routingTable->getRoutingTable());
                    std::cout << "Loaded routing table saved to " << filename << std::endl;
                }
                break;
            case 4:
                routingTable->print(routingTable->getRoutingTable());
                std::cout << "------------------------------------------\nPrinted " << routingTable->getRoutingTable().size() << " rows." << std::endl;
                break;
            case 5:
                if (routingTableVector.size() > 0) {
                    savingPrompt(routingTable, routingTableVector, filename);
                } else {
                    std::cout << "No filtered routing table values to save!" << std::endl;
                }
                break;
            case 6:
                if (routingTableVector.size() > 0) {
                    routingTable->print(routingTableVector);
                    std::cout << "------------------------------------------\nPrinted " << routingTableVector.size() << " rows." << std::endl;
                } else {
                    std::cout << "No filtered routing table values to print!" << std::endl;
                }
                break;
            case 7:
                autoPrintAndSavePrompt = !autoPrintAndSavePrompt;
                autoPrintAndSavePrompt ? std::cout << "Auto printing filtered table enabled!" << std::endl : std::cout << "Auto printing filtered table disabled!" << std::endl;
                break;
            case 8:
                routingTableVector = routingTable->getRoutingTable();
                std::cout << "Filtered table reseted!" << std::endl;
                break;
            case 9:
                std::cout << "Exiting..." << std::endl;
                delete routingTable;
                routingTable = nullptr;
                return 0;
            default:
                std::cout << "Invalid option!" << std::endl;
        }
        option = -1;
    } while (true);
    return 0;
}