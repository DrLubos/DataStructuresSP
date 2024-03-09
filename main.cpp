#include "RoutingTable.h"

int main() {
    RoutingTable routingTable;
    try {
        routingTable.loadFromCSV("RT.csv");
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    routingTable.printAll();
    //routingTable.saveToCSV("RT2.csv");
    return 0;
}