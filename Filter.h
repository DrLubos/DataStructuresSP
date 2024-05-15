#include "RoutingTable.h"
#include <libds/heap_monitor.h>

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
    static void chooseLifetime(unsigned int& startingLifetime, unsigned int& endingLifetime);
    static void chooseAddress(std::bitset<32>& ipAddressToCompare);
    template<typename Pred, typename Seq, typename Iterator>
    static void filterEntries(Iterator begin, Iterator end, Pred predicate, Seq& sequence);
};

template<typename Pred, typename Seq, typename Iterator>
void Filter::filterEntries(Iterator begin, Iterator end, Pred predicate, Seq& sequence) {
    while(begin != end) {
        if(predicate(*begin)) {
            sequence.insertLast().data_ = &(*begin);
        }
        begin.operator++();
    }
}

void Filter::chooseLifetime(unsigned int& startingLifetime, unsigned int& endingLifetime) {
    std::string start;
    std::string end;
    std::cout << "Insert minimum possible starting lifetime (s) or (XwXdXhXmXs) or (HH:MM:SS): ";
    std::cin >> start;
    startingLifetime = RoutingTableOperations::isStringNumeric(start) ? std::stoul(start) : RoutingTableOperations::processLifetime(start);
    std::cout << "Insert maximum possible ending lifetime (s) or (XwXdXhXmXs) or (HH:MM:SS) or (-1) to check without maximum possible ending: ";
    std::cin >> end;
    if (end[0] == '-' && end[1] == '1') {
        endingLifetime = UINT_MAX;
    } else if (RoutingTableOperations::isStringNumeric(end)) {
        endingLifetime = std::stoul(end);
    } else {
        endingLifetime = RoutingTableOperations::processLifetime(end);
    }
    if (startingLifetime > endingLifetime) {
        std::swap(startingLifetime, endingLifetime);
    }
    std::cout << "Selected lifetime: " << startingLifetime << "(s) - " << endingLifetime << "(s)" << std::endl;
}

void Filter::chooseAddress(std::bitset<32>& ipAddressToCompare) {
    bool validIpAddress = false;
    while (!validIpAddress) {
        std::cout << "Insert IP address to filter (W.X.Y.Z): ";
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