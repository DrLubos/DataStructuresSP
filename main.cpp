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

void savingPrompt(RoutingTable* routingTable, std::vector<RoutingTableRow>& routingTableVector, std::string& filename, bool autoPrint = true) {
    if (autoPrint) {
        routingTable->print(routingTableVector);
    }
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
    routingTable->matchLifetime(start, end, routingTableVector);
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
    std::vector<RoutingTableRow> routingTableVector = routingTable->getRoutingTable();
    std::cout << "Routing table loaded successfully!" << std::endl;
    std::string optionString;
    int option;
    bool autoPrint = true;
    do {
        std::cout << std::endl << "Options:\n\t[0] Filter by mathing IP address and lifetime" << std::endl;
        std::cout << "\t[1] Filter by matching IP address" << std::endl;
        std::cout << "\t[2] Filter by matching lifetime" << std::endl;
        std::cout << "\t[3] Save whole table to CSV" << std::endl;
        std::cout << "\t[4] Print whole routing table" << std::endl;
        std::cout << "\t[5] Save filtered routing table to CSV" << std::endl;
        std::cout << "\t[6] Print filtered routing table" << std::endl;
        autoPrint ? std::cout << "\t[7] Disable auto printing filtered table" << std::endl : std::cout << "\t[7] Enable auto printing filtered table" << std::endl;
        std::cout << "\t[9] To exit program" << std::endl;
        std::cout << "Your option: ";
        std::cin >> optionString;
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
                savingPrompt(routingTable, routingTableVector, filename, autoPrint);
                break;
            case 1:
                filterByAddress(routingTable, routingTableVector);
                savingPrompt(routingTable, routingTableVector, filename, autoPrint);
                break;
            case 2:
                filterByLifetime(routingTable, routingTableVector);
                savingPrompt(routingTable, routingTableVector, filename, autoPrint);
                break;
            case 3:
                filename = "RT_Loaded.csv";
                selectNameOfCSVFile(filename);
                routingTable->saveToCSV(filename, loadedRoutingTable);
                std::cout << "Loaded routing table saved to " << filename << std::endl;
                break;
            case 4:
                routingTable->print(loadedRoutingTable);
                std::cout << "------------------------------------------\nPrinted " << loadedRoutingTable.size() << " rows." << std::endl;
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
                autoPrint = !autoPrint;
                autoPrint ? std::cout << "Auto printing filtered table enabled!" << std::endl : std::cout << "Auto printing filtered table disabled!" << std::endl;
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