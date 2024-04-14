#include "RoutingTable.h"
#include "Filter.h"
#include "Hierarchy.h"

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
void savingPrompt(std::vector<P>& routingTableVector, std::string& filename, RoutingTableOperations& routingTableOperations) {
    if (routingTableVector.size() > 0) {
        char choice;
        std::cout << "Do you want to save the filtered table to a CSV file? (y/n):";
        std::cin >> choice;
        if (choice == 'y' || choice == 'Y') {
            filename = "RT_Filtered.csv";
            selectNameOfCSVFile(filename);
            if (filename != "") {
                routingTableOperations.saveToCSV(filename, routingTableVector);
                std::cout << "Filtered routing table saved to " << filename << std::endl;
            } else {
                std::cout << "Saving cancelled!" << std::endl;
            }
        }
    }
}

void mainLoop(RoutingTableOperations& routingTableOperations, std::vector<RoutingTableRow>& loadedRoutingTable) {
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
                filter.chooseAddress(ipAddressToCompare, routingTableOperations);
                filter.chooseLifetime(startingLifetime, endingLifetime, routingTableOperations);
                filtering = filter.filterEntries<RoutingTableRow>(loadedRoutingTable.begin(), loadedRoutingTable.end(), [&](const RoutingTableRow& row) {
                    return matchLifetime(row, startingLifetime, endingLifetime) && matchWithAddress(row, ipAddressToCompare);
                });
                break;
            case 1:
                filter.chooseAddress(ipAddressToCompare, routingTableOperations);
                filtering = filter.filterEntries<RoutingTableRow>(loadedRoutingTable.begin(), loadedRoutingTable.end(), [&](const RoutingTableRow& row) {
                    return matchWithAddress(row, ipAddressToCompare);
                });
                break;
            case 2:
                filter.chooseLifetime(startingLifetime, endingLifetime, routingTableOperations);
                filtering = filter.filterEntries<RoutingTableRow>(loadedRoutingTable.begin(), loadedRoutingTable.end(), [&](const RoutingTableRow& row) {
                    return matchLifetime(row, startingLifetime, endingLifetime);
                });
                break;
            case 3:
                filename = "RT_Loaded.csv";
                selectNameOfCSVFile(filename);
                if (filename == "") {
                    std::cout << "Save cancelled!" << std::endl;
                } else {
                    routingTableOperations.saveToCSV(filename, loadedRoutingTable);
                    std::cout << "Loaded routing table saved to " << filename << std::endl;
                }
                break;
            case 4:
                routingTableOperations.print(loadedRoutingTable);
                std::cout << "------------------------------------------\nPrinted " << loadedRoutingTable.size() << " values." << std::endl;
                break;
            case 5:
                if (filtering.size() > 0) {
                    savingPrompt(filtering, filename, routingTableOperations);
                } else {
                    std::cout << "No filtered routing table values to save!" << std::endl;
                }
                break;
            case 6:
                routingTableOperations.print(filtering);
                std::cout << "------------------------------------------\nPrinted " << filtering.size() << " values." << std::endl;
            case 7:
                autaSavePrompt = !autaSavePrompt;
                autaSavePrompt ? std::cout << "Auto printing filtered table enabled!" << std::endl : std::cout << "Auto printing filtered table disabled!" << std::endl;
                break;
            case 9:
                std::cout << "Exiting..." << std::endl;
                return;
            default:
                std::cout << "Invalid option!" << std::endl;
        }
        if (option > -1 && option < 3) {
            routingTableOperations.print(filtering);
            std::cout << "------------------------------------------\nFound " << filtering.size() << " values." << std::endl;
            if (autaSavePrompt) {
                savingPrompt(filtering, filename, routingTableOperations);
            }
        }
        option = -1;
    } while (true);
}

int main() {
    RoutingTableOperations routingTableOperations;
    std::vector<RoutingTableRow> loadedRoutingTable;
    try {
        routingTableOperations.loadFromCSV("RT.csv", loadedRoutingTable);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    std::cout << "Routing table loaded successfully!" << std::endl;
    mainLoop(routingTableOperations, loadedRoutingTable);
    return 0;
}