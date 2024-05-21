#include <iostream>
#include "RoutingTable.h"
#include <libds/heap_monitor.h>

class UserInteraction {
	public:
        static void hierarchyFilteredToNormalSequence(ds::amt::IS<Node*>& hierarchy, ds::amt::IS<RoutingTableRow*>& sequence);
        static void printFilteredSequence(ds::amt::IS<RoutingTableRow*>& sequence);
		static void printOptions();
		static void selectNameOfCSVFile(std::string& defaultName);
        static void savingPrompt(ds::amt::IS<RoutingTableRow*>& sequence, std::string& filename);
};

void UserInteraction::hierarchyFilteredToNormalSequence(ds::amt::IS<Node*>& hierarchy, ds::amt::IS<RoutingTableRow*>& sequence) {
    for (auto& node : hierarchy) {
        sequence.insertLast().data_ = node->pData;
    }
}

void UserInteraction::printFilteredSequence(ds::amt::IS<RoutingTableRow*> &sequence) {
    if (!sequence.isEmpty()) {
        ds::amt::IS<RoutingTableRow*>::ImplicitSequenceIterator beginSequence = sequence.begin();
        ds::amt::IS<RoutingTableRow*>::ImplicitSequenceIterator endSequence = sequence.end();
        for (auto it = beginSequence; it != endSequence; ++it) {
            RoutingTableOperations::printRow(*(*it));
        }
    }
    std::cout << "------------------------------------------\nPrinted " << sequence.size() << " values." << std::endl;
}

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
    std::cout << "\t------------------ Table mode ------------------" << std::endl;
    std::cout << "\t[21] Filter table by destination IP address" << std::endl;
    std::cout << "\t----------------- Sorting mode -----------------" << std::endl;
    std::cout << "\t[31] Sort filtered by IP address" << std::endl;
    std::cout << "\t[32] Sort filtered by lifetime" << std::endl;
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

void UserInteraction::savingPrompt(ds::amt::IS<RoutingTableRow*>& sequence, std::string& filename) {
    if (!sequence.isEmpty()) {
        char choice;
        std::cout << "Do you want to save the filtered table to a CSV file? (y/n):";
        std::cin >> choice;
        if (choice == 'y' || choice == 'Y') {
            filename = "RT_Filtered.csv";
            UserInteraction::selectNameOfCSVFile(filename);
            if (!filename.empty()) {
                RoutingTableOperations::saveFilteredToCSV(filename, sequence);
                std::cout << "Filtered routing table saved to " << filename << std::endl;
            } else {
                std::cout << "Saving cancelled!" << std::endl;
            }
        }
    }
}