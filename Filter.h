#include "RoutingTable.h"

auto matchLifetime = [](const RoutingTableRow& row, unsigned int startTime, unsigned int endTime) {
    if (row.lifetime >= startTime && row.lifetime <= endTime) {
        return true;
    }
    return false;
};

auto matchWithAddress = [](const RoutingTableRow& row, const std::bitset<32>& addressToCompare) {
    std::bitset<32> ipAddress = row.ipAddress;
    for (int i = 0; i < row.prefix; ++i) {
        if (ipAddress[ipAddress.size() - i - 1] != addressToCompare[addressToCompare.size() - i - 1]) {
            return false;
        }
    }
    return true;
};

class Filter {
    public:
        Filter() {}
        template<typename P, typename Iterator>
        std::vector<P*> filterEntries(Iterator begin, Iterator end, std::function<bool(const P&)> pred);
        void chooseLifetime(unsigned int& startingLifetime, unsigned int& endingLifetime, RoutingTableOperations& routingTableOperations);
        void chooseAddress(std::bitset<32>& ipAddressToCompare, RoutingTableOperations& routingTableOperations);
};

template<typename P, typename Iterator>
std::vector<P*> Filter::filterEntries(Iterator begin, Iterator end, std::function<bool(const P&)> pred) {
    std::vector<P*> filtered;
    for (auto it = begin; it != end; ++it) {
        if (pred(*it)) {
            filtered.push_back(&(*it));
        }
    }
    return filtered;
}

void Filter::chooseLifetime(unsigned int& startingLifetime, unsigned int& endingLifetime, RoutingTableOperations& routingTableOperations) {
    std::string start;
    std::string end;
    std::cout << "Insert minimum possible starting lifetime (s) or (XwXdXhXmXs) or (HH:MM:SS): ";
    std::cin >> start;
    startingLifetime = isStringNumeric(start) ? std::stoul(start) : routingTableOperations.processLifetime(start);
    std::cout << "Insert maximum possible ending lifetime (s) or (XwXdXhXmXs) or (HH:MM:SS) or (-1) to check without maximum possible ending: ";
    std::cin >> end;
    if (end[0] == '-' && end[1] == '1') {
        endingLifetime = UINT_MAX;
    } else if (isStringNumeric(end)) {
        endingLifetime = std::stoul(end);
    } else {
        endingLifetime = routingTableOperations.processLifetime(end);
    }
    if (startingLifetime > endingLifetime) {
        std::swap(startingLifetime, endingLifetime);
    }
    std::cout << "Selected lifetime: " << startingLifetime << "(s) - " << endingLifetime << "(s)" << std::endl;
}

void Filter::chooseAddress(std::bitset<32>& ipAddressToCompare, RoutingTableOperations& routingTableOperations) {
    bool validIpAddress = false;
    while (!validIpAddress) {
        std::cout << "Insert IP address to filter: ";
        std::string ipAddressString;
        std::cin >> ipAddressString;
        try {
            ipAddressToCompare = routingTableOperations.processIPAddress(ipAddressString, nullptr);
            validIpAddress = true;
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }
}