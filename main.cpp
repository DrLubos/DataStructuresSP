#include "RoutingTable.h"

int main() {
    RoutingTable routingTable;
    try {
        routingTable.loadFromCSV("RT.csv");
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    routingTable.print(routingTable.getRoutingTable());
    std::vector<RoutingTableRow> matching = routingTable.matchLifetime("0", "d");
    routingTable.print(matching);
    //routingTable.saveToCSV("RT2.csv");
    return 0;
}