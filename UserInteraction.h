#include <iostream>
#include "RoutingTable.h"
#include <libds/heap_monitor.h>

class UserInteraction {
	public:
		static void printOptions();
		static void selectNameOfCSVFile(std::string& defaultName);
		template<typename P>
        static void savingPrompt(std::vector<P>& routingTableVector, std::string& filename);
};

void UserInteraction::printOptions() {
    std::cout << std::endl << "Options:" << std::endl;
    std::cout << "\t[-1] To exit program" << std::endl;
    std::cout << "\t-------------- Standard filtering --------------" << std::endl;
    std::cout << "\t[0] Filter by matching IP address and lifetime" << std::endl;
    std::cout << "\t[1] Filter by matching IP address" << std::endl;
    std::cout << "\t[2] Filter by matching lifetime" << std::endl;
    std::cout << "\t------------ Save and print options ------------" << std::endl;
    std::cout << "\t[3] Print whole routing table" << std::endl;
    std::cout << "\t[4] Save whole table to CSV" << std::endl;
    std::cout << "\t[5] Print filtered routing table" << std::endl;
    std::cout << "\t[6] Save filtered routing table to CSV" << std::endl;
    std::cout << "\t---------------- Hierarchy mode ----------------" << std::endl;
    std::cout << "\t[10] Filter hierarchy by matching IP address and lifetime" << std::endl;
    std::cout << "\t[11] Filter hierarchy by matching IP address" << std::endl;
    std::cout << "\t[12] Filter hierarchy by matching lifetime" << std::endl;
    std::cout << "\t[13] Print Node information" << std::endl;
    std::cout << "\t[14] Go to parent" << std::endl;
    std::cout << "\t[15] Go to son" << std::endl;
    std::cout << "\t[16] Print leafs" << std::endl;
    std::cout << "Your option: ";
}

void UserInteraction::selectNameOfCSVFile(std::string& defaultName) {
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
void UserInteraction::savingPrompt(std::vector<P>& routingTableVector, std::string& filename) {
    if (!routingTableVector.empty()) {
        char choice;
        std::cout << "Do you want to save the filtered table to a CSV file? (y/n):";
        std::cin >> choice;
        if (choice == 'y' || choice == 'Y') {
            filename = "RT_Filtered.csv";
            UserInteraction::selectNameOfCSVFile(filename);
            if (!filename.empty()) {
                RoutingTableOperations::saveToCSV(filename, routingTableVector);
                std::cout << "Filtered routing table saved to " << filename << std::endl;
            } else {
                std::cout << "Saving cancelled!" << std::endl;
            }
        }
    }
}