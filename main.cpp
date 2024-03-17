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

void savingPrompt(RoutingTableOperations* routingTable, std::vector<RoutingTableRow>& routingTableVector, std::string& filename) {
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

void filterByLifetime(RoutingTableOperations* routingTable, std::vector<RoutingTableRow>& filteredRoutingTableVector, std::vector<RoutingTableRow>& loadedRT, Filter& filter) {
    std::string start;
    std::string end;
    std::cout << "Insert minimum possible starting lifetime (s) or (XwXdXhXmXs) or (HH:MM:SS): ";
    std::cin >> start;
    unsigned int startLifetime = isStringNumeric(start) ? std::stoul(start) : routingTable->processLifetime(start);
    std::cout << "Insert maximum possible ending lifetime (s) or (XwXdXhXmXs) or (HH:MM:SS) or (-1) to check without maximum possible ending: ";
    std::cin >> end;
    unsigned int endLifetime;
    if (end[0] == '-' && end[1] == '1'){
        endLifetime = std::numeric_limits<unsigned int>::max();
    } else if (isStringNumeric(end)) {
        endLifetime = std::stoul(end);
    } else {
        endLifetime = routingTable->processLifetime(end);
    }
    if (startLifetime > endLifetime) {
        std::swap(startLifetime, endLifetime);
    }
    std::cout << "Selected lifetime: " << startLifetime << "(s) - " << endLifetime << "(s)" << std::endl;
    filter.filterAndAppend(loadedRT.begin(), loadedRT.end(), [&](const RoutingTableRow& row) {
        return matchLifetime(row, startLifetime, endLifetime);
    }, filteredRoutingTableVector);
}

void filterByAddress(RoutingTableOperations* routingTable, std::vector<RoutingTableRow>& filteredRoutingTableVector, std::vector<RoutingTableRow>& loadedRT, Filter& filter) {
    std::string ipAddressString;
    std::cout << "Insert IP address to filter: ";
    std::cin >> ipAddressString;
    filter.filterAndAppend(loadedRT.begin(), loadedRT.end(), [&](const RoutingTableRow& row) {
        return matchWithAddress(row, ipAddressString);
    }, filteredRoutingTableVector);
}

int main() {
    std::vector<RoutingTableRow> filteredRoutingTableVector;
    RoutingTableOperations* routingTable = new RoutingTableOperations();
    std::vector<RoutingTableRow> loadedRoutingTable;
    try {
        routingTable->loadFromCSV("RT.csv", loadedRoutingTable);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        delete routingTable;
        routingTable = nullptr;
        return 1;
    }
    std::cout << "Routing table loaded successfully!" << std::endl;
    Filter filter;
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
        std::cout << "\t[8] Clear filtered table" << std::endl;
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
                filterByAddress(routingTable, filteredRoutingTableVector, loadedRoutingTable, filter);
                filterByLifetime(routingTable, filteredRoutingTableVector, loadedRoutingTable, filter);
                if (autoPrintAndSavePrompt) {
                    routingTable->print(filteredRoutingTableVector);
                    savingPrompt(routingTable, filteredRoutingTableVector, filename);
                }
                std::cout << "------------------------------------------\nFound " << filteredRoutingTableVector.size() << " values." << std::endl;
                break;
            case 1:
                filterByAddress(routingTable, filteredRoutingTableVector, loadedRoutingTable, filter);
                if (autoPrintAndSavePrompt) {
                    routingTable->print(filteredRoutingTableVector);
                    savingPrompt(routingTable, filteredRoutingTableVector, filename);
                }
                std::cout << "------------------------------------------\nFound " << filteredRoutingTableVector.size() << " values." << std::endl;
                break;
            case 2:
                filterByLifetime(routingTable, filteredRoutingTableVector, loadedRoutingTable, filter);
                if (autoPrintAndSavePrompt) {
                    routingTable->print(filteredRoutingTableVector);
                    savingPrompt(routingTable, filteredRoutingTableVector, filename);
                }
                std::cout << "------------------------------------------\nFound " << filteredRoutingTableVector.size() << " values." << std::endl;
                break;
            case 3:
                filename = "RT_Loaded.csv";
                selectNameOfCSVFile(filename);
                if (filename == "") {
                    std::cout << "Save cancelled!" << std::endl;
                } else {
                    routingTable->saveToCSV(filename, loadedRoutingTable);
                    std::cout << "Loaded routing table saved to " << filename << std::endl;
                }
                break;
            case 4:
                routingTable->print(loadedRoutingTable);
                std::cout << "------------------------------------------\nPrinted " << loadedRoutingTable.size() << " rows." << std::endl;
                break;
            case 5:
                if (filteredRoutingTableVector.size() > 0) {
                    savingPrompt(routingTable, filteredRoutingTableVector, filename);
                } else {
                    std::cout << "No filtered routing table values to save!" << std::endl;
                }
                break;
            case 6:
                if (filteredRoutingTableVector.size() > 0) {
                    routingTable->print(filteredRoutingTableVector);
                    std::cout << "------------------------------------------\nPrinted " << filteredRoutingTableVector.size() << " rows." << std::endl;
                } else {
                    std::cout << "No filtered routing table values to print!" << std::endl;
                }
                break;
            case 7:
                autoPrintAndSavePrompt = !autoPrintAndSavePrompt;
                autoPrintAndSavePrompt ? std::cout << "Auto printing filtered table enabled!" << std::endl : std::cout << "Auto printing filtered table disabled!" << std::endl;
                break;
            case 8:
                filteredRoutingTableVector.clear();
                std::cout << "Filtered table is cleared!" << std::endl;
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