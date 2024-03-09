#include "RoutingTable.h"

void selectNameOfFile(std::string& defaultName) {
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

int main() {
    RoutingTable* routingTable = new RoutingTable();
    try {
        routingTable->loadFromCSV("RT.csv");
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        delete routingTable;
        return 1;
    }
    std::vector<RoutingTableRow> loadedRoutingTable = routingTable->getRoutingTable();
    std::vector<RoutingTableRow> routingTableVector;
    std::cout << "Routing table loaded successfully!" << std::endl;
    std::string optionString;
    int option;
    do {
        std::cout << "Options:\n\t[1] To filter by mathing IP address and lifetime" << std::endl;
        std::cout << "\t[2] Filter by matching IP address" << std::endl;
        std::cout << "\t[3] Filter by matching lifetime" << std::endl;
        std::cout << "\t[4] Save whole table to CSV" << std::endl;
        std::cout << "\t[5] Print whiole routing table" << std::endl;
        std::cout << "\t[9] To exit program" << std::endl;
        std::cin >> optionString;
        try {
            option = std::stoi(optionString);
        } catch (const std::exception& e) {
            option = 0;
        }
        std::string start;
        std::string end;
        std::string filename;
        switch (option) {
            case 1:
                std::cerr << "Not implemented yet." << std::endl;
                break;
            case 2:
                std::cerr << "Not implemented yet." << std::endl;
                break;
            case 3:
                std::cout << "Insert minimum possible starting lifetime (s) or (XwXdXhXmXs) or (HH:MM:SS): ";
                std::cin >> start;
                std::cout << "Insert maximum possible ending lifetime (s) or (XwXdXhXmXs) or (HH:MM:SS) or (-1) to check without maximum possible ending: ";
                std::cin >> end;
                if (end[0] == '-' && end[1] == '1'){
                    end = std::to_string(std::numeric_limits<unsigned int>::max());
                }
                routingTableVector = routingTable->matchLifetime(start, end);
                routingTable->print(routingTableVector);
                char choice;
                std::cout << "Found " << routingTableVector.size() << " values." << std::endl;
                if (routingTableVector.size() > 0) {
                    std::cout << "Do you want to save the filtered table to a CSV file? (y/n):" << std::endl;
                } else {
                    option = 0;
                }
                std::cin >> choice;
                if (choice == 'y' || choice == 'Y') {
                    filename = "RT_Filtered.csv";
                    selectNameOfFile(filename);
                    routingTable->saveToCSV(filename, routingTableVector);
                    std::cout << "Filtered routing table saved to " << filename << std::endl;
                }
                break;
            case 4:
                filename = "RT_Loaded.csv";
                selectNameOfFile(filename);
                routingTable->saveToCSV(filename, loadedRoutingTable);
                std::cout << "Loaded routing table saved to " << filename << std::endl;
                break;
            case 5:
                routingTable->print(loadedRoutingTable);
                break;
            case 9:
                std::cout << "Exiting..." << std::endl;
                delete routingTable;
                return 0;
        }
        option = 0;
    } while (option < 1 || option > 9);
    return 0;
}