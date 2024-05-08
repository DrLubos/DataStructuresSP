#include "HierarchyManager.h"
#include "TableManager.h"
#include "RoutingTable.h"
#include <libds/heap_monitor.h>

class Loader {
public:
    static void loadFromCSV(const std::string& filename, std::vector<RoutingTableRow>& saveToVector, HierarchyManager& hierarchy, TableManager& tableManager);
};

void Loader::loadFromCSV(const std::string& filename, std::vector<RoutingTableRow>& saveToVector, HierarchyManager& hierarchy, TableManager& tableManager) {
    std::ifstream file(filename);
    saveToVector.reserve(20000);
    if (!file.is_open()) {
        throw std::runtime_error("Error: Could not open file.\n");
    }
    std::string line;
    unsigned int rowNumber = 1;
    std::getline(file, line);
    while (std::getline(file, line)) {
        ++rowNumber;
        std::istringstream ss(line);
        std::vector<std::string> cells;
        std::string cell;
        while (std::getline(ss, cell, ';')) {
            cells.push_back(cell);
        }
        if (cells.size() == 5 || cells.size() == 4) {
            RoutingTableRow entry;
            try {
                entry.ipAddress = RoutingTableOperations::processIPAddress(cells[1], &entry.prefix);
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << " Check line " << rowNumber << std::endl;
                continue;
            }
            if (cells[3][0] == 'v' && cells[3][1] == 'i' && cells[3][2] == 'a') {
                entry.destinationIP = cells[3][3] == ' ' ? RoutingTableOperations::processIPAddress(cells[3].substr(4), nullptr) : RoutingTableOperations::processIPAddress(cells[3].substr(3), nullptr);
            } else {
                std::cout << "Supported only next-hop ip address, check line " << rowNumber << std::endl;
            }
            if (cells.size() > 4 && (std::isdigit(cells[4][0]) || std::isalpha(cells[4][0]))) {
                entry.lifetime = RoutingTableOperations::processLifetime(cells[4]);
            } else {
                entry.lifetime = UINT_MAX;
            }
            saveToVector.push_back(entry);
            hierarchy.addBranch(entry.ipAddress, &saveToVector.back());
            tableManager.addEntry(entry.destinationIP, &saveToVector.back());
        } else {
            throw std::runtime_error("Error: Invalid CSV format. Error found on line: " + std::to_string(rowNumber) + "\n");
        }
    }
}