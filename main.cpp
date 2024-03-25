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
            } else {
                std::cout << "Saving cancelled!" << std::endl;
            }
        }
    }
}

void chooseLifetime(unsigned int& startingLifetime, unsigned int& endingLifetime) {
    std::string start;
    std::string end;
    std::cout << "Insert minimum possible starting lifetime (s) or (XwXdXhXmXs) or (HH:MM:SS): ";
    std::cin >> start;
    startingLifetime = isStringNumeric(start) ? std::stoul(start) : RoutingTableOperations::processLifetime(start);
    std::cout << "Insert maximum possible ending lifetime (s) or (XwXdXhXmXs) or (HH:MM:SS) or (-1) to check without maximum possible ending: ";
    std::cin >> end;
    if (end[0] == '-' && end[1] == '1') {
        endingLifetime = UINT_MAX;
    } else if (isStringNumeric(end)) {
        endingLifetime = std::stoul(end);
    } else {
        endingLifetime = RoutingTableOperations::processLifetime(end);
    }
    if (startingLifetime > endingLifetime) {
        std::swap(startingLifetime, endingLifetime);
    }
    std::cout << "Selected lifetime: " << startingLifetime << "(s) - " << endingLifetime << "(s)" << std::endl;
}

void chooseAddress(std::bitset<32>& ipAddressToCompare) {
    bool validIpAddress = false;
    while (!validIpAddress) {
        std::cout << "Insert IP address to filter: ";
        std::string ipAddressString;
        std::cin >> ipAddressString;
        try {
            ipAddressToCompare = RoutingTableOperations::processIPAddress(ipAddressString, nullptr);
            validIpAddress = true;
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }
}

int main() {
    //RoutingTableOperations routingTableOperations;
    std::vector<RoutingTableRow> loadedRoutingTable;
    std::vector<RoutingTableRow*> filtering;
    try {
        RoutingTableOperations::loadFromCSV("RT.csv", loadedRoutingTable);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    std::cout << "Routing table loaded successfully!" << std::endl;
    Filter filter;
    std::string optionString;
    int option;
    bool autaSavePrompt = true;
    do {
        std::cout << std::endl << "Options:\n\t[0] Filter by mathing IP address and lifetime" << std::endl;
        std::cout << "\t[1] Filter by matching IP address" << std::endl;
        std::cout << "\t[2] Filter by matching lifetime" << std::endl;
        std::cout << "\t[3] Save whole table to CSV" << std::endl;
        std::cout << "\t[4] Print whole routing table" << std::endl;
        std::cout << "\t[5] Save filtered routing table to CSV" << std::endl;
        autaSavePrompt ? std::cout << "\t[6] Disable auto save prompt" << std::endl : std::cout << "\t[7] Enable auto save prompt" << std::endl;
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
        //option = 3;
        switch (option) {
            case 0:
                chooseAddress(ipAddressToCompare);
                chooseLifetime(startingLifetime, endingLifetime);
                filtering = filter.filterEntries<RoutingTableRow>(loadedRoutingTable.begin(), loadedRoutingTable.end(), [&](const RoutingTableRow& row) {
                    return matchLifetime(row, startingLifetime, endingLifetime) && matchWithAddress(row, ipAddressToCompare);
                });
                break;
            case 1:
                chooseAddress(ipAddressToCompare);
                filtering = filter.filterEntries<RoutingTableRow>(loadedRoutingTable.begin(), loadedRoutingTable.end(), [&](const RoutingTableRow& row) {
                    return matchWithAddress(row, ipAddressToCompare);
                });
                break;
            case 2:
                chooseLifetime(startingLifetime, endingLifetime);
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
                    RoutingTableOperations::saveToCSV(filename, loadedRoutingTable);
                    std::cout << "Loaded routing table saved to " << filename << std::endl;
                }
                break;
            case 4:
                RoutingTableOperations::print(loadedRoutingTable);
                std::cout << "------------------------------------------\nPrinted " << loadedRoutingTable.size() << " rows." << std::endl;
                break;
            case 5:
                if (filtering.size() > 0) {
                    savingPrompt(filtering, filename);
                } else {
                    std::cout << "No filtered routing table values to save!" << std::endl;
                }
                break;
            case 6:
                autaSavePrompt = !autaSavePrompt;
                autaSavePrompt ? std::cout << "Auto printing filtered table enabled!" << std::endl : std::cout << "Auto printing filtered table disabled!" << std::endl;
                break;
            case 9:
                std::cout << "Exiting..." << std::endl;
                return 0;
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
    return 0;
}