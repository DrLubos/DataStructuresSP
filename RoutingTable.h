#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <bitset>
#include <functional>
#include <libds/amt/implicit_sequence.h>
#include <libds/heap_monitor.h>

bool operator>(const std::bitset<32>& first, const std::bitset<32>& second) {
    return (first.to_ulong() > second.to_ulong());
}

bool operator<(const std::bitset<32>& first, const std::bitset<32>& second) {
    return (first.to_ulong() < second.to_ulong());
}

struct RoutingTableRow {
    std::bitset<32> ipAddress;
    std::bitset<32> destinationIP;
    unsigned int lifetime;
    unsigned char prefix;
};

class RoutingTableOperations {
private:
    static bool isStringIPMaskFormat(const std::string& str);
    static void saveRowToCSV(std::ofstream& file, const RoutingTableRow& row);
public:
    static bool isStringNumeric(const std::string& str);
    static void print(const std::vector<RoutingTableRow>& vectorToPrint);
    static void printRow(const RoutingTableRow& row);
    static void sortPrint(const RoutingTableRow& row);
    static std::bitset<32> processIPAddress(const std::string& ipAddressString, unsigned char* prefix);
    static unsigned int processLifetime(const std::string& lifetimeString);
    static std::string convertLifetime(unsigned int lifetime);
    static void saveToCSV(const std::string& filename, const std::vector<RoutingTableRow>& vectorToPrint);
    static void saveFilteredToCSV(const std::string& filename, ds::amt::IS<RoutingTableRow*>& sequence);
};

bool RoutingTableOperations::isStringNumeric(const std::string &str) {
    for (char c : str) {
        if (!std::isdigit(c)) {
            return false;
        }
    }
    return true;
}

bool RoutingTableOperations::isStringIPMaskFormat(const std::string& str) {
    for (char c : str) {
        if (!std::isdigit(c) && c != '/' && c != '.' && c != ' ') {
            return false;
        }
    }
    return true;
}

void RoutingTableOperations::print(const std::vector<RoutingTableRow>& vectorToPrint) {
    for (const RoutingTableRow& row : vectorToPrint) {
        printRow(row);
    }
}

void RoutingTableOperations::printRow(const RoutingTableRow& row) {
    std::cout << "==========================================" << std::endl;
    std::cout << "IP Address: ";
    for (int i = row.ipAddress.size() - 8; i >= 0; i -= 8) {
        std::cout << std::bitset<8>(row.ipAddress.to_ulong() >> i).to_ulong();
        if (i > 0) {
            std::cout << ".";
        }
    }
    std::cout << "/" << int(row.prefix) << std::endl;
    std::cout << "Next Hop: ";
    for (int i = row.destinationIP.size() - 8; i >= 0; i -= 8) {
        std::cout << std::bitset<8>(row.destinationIP.to_ulong() >> i).to_ulong();
        if (i > 0) {
            std::cout << ".";
        }
    }
    std::cout << std::endl;
    std::cout << "Lifetime: ";
    row.lifetime > 59 ? std::cout << row.lifetime << "(s) " << convertLifetime(row.lifetime) << std::endl : std::cout << row.lifetime << "s" << std::endl;
}

void RoutingTableOperations::sortPrint(const RoutingTableRow &row) {
    std::string firstPart = "IP address: ";
    for (int i = row.ipAddress.size() - 8; i >= 0; i -= 8) {
        firstPart += std::to_string(int(std::bitset<8>(row.ipAddress.to_ulong() >> i).to_ulong()));
        if (i > 0) {
            firstPart += ".";
        }
    }
    firstPart += "/" + std::to_string(int(row.prefix));
    while (firstPart.size() < 35) {
        firstPart += " ";
    }
    firstPart += "Next Hop: ";
    for (int i = row.destinationIP.size() - 8; i >= 0; i -= 8) {
        firstPart += std::to_string(int(std::bitset<8>(row.destinationIP.to_ulong() >> i).to_ulong()));
        if (i > 0) {
            firstPart += ".";
        }
    }
    while (firstPart.size() < 62) {
        firstPart += " ";
    }
    std::string lifetimeString = convertLifetime(row.lifetime);
    firstPart += "Lifetime: " + lifetimeString;
    if (lifetimeString.empty()) {
        firstPart += "UNLIMITED";
    }
    firstPart += " in seconds:    " + std::to_string(row.lifetime);
    while (firstPart.size() < 109) {
        size_t index = firstPart.find("seconds:");
        firstPart.insert(index + 8, " ");
    }
    std::cout << firstPart << std::endl;
}

std::bitset<32> RoutingTableOperations::processIPAddress(const std::string& ipAddressString, unsigned char* prefix) {
    std::bitset<32> ipAddressBits;
    std::istringstream iss(ipAddressString);
    std::string octetString;
    int index = 3;
    while (std::getline(iss, octetString, '.')) {
        if (!isStringIPMaskFormat(octetString)) {
            std::cout << octetString << std::endl;
            throw std::runtime_error("Error: Invalid IP address format.\n");
        }
        int octet = std::stoi(octetString);
        if (octet >= 0 && octet < 256) {
            ipAddressBits |= (std::bitset<32>(octet) << (index * 8));
        }
        --index;
    }
    size_t startingPrefixIndex = ipAddressString.find('/');
    if (startingPrefixIndex != std::string::npos) {
        std::string prefixString = ipAddressString.substr(startingPrefixIndex + 1);
        if (!isStringIPMaskFormat(prefixString)) {
            throw std::runtime_error("Error: Invalid prefix format.\n");
        }
        int prefixValue = std::stoi(prefixString);
        if (prefixValue < 0 || prefixValue > 32) {
            throw std::runtime_error("Error: Invalid prefix value.\n");
        }
        if (prefix != nullptr) {
            *prefix = prefixValue;
        }
    }
    return ipAddressBits;
}

unsigned int RoutingTableOperations::processLifetime(const std::string& lifetimeString) {
    std::vector<unsigned int> lifetime;
    unsigned int processingNumber = 0;
    bool hoursMuliply = true;
    bool minutesMuliply = true;
    size_t i = 0;
    if (lifetimeString.size() > 0 && !std::isdigit(lifetimeString[0])) {
        switch (lifetimeString[0]) {
            case 'w':
                lifetime.push_back(60 * 60 * 24 * 7);
                ++i;
                break;
            case 'd':
                lifetime.push_back(60 * 60 * 24);
                ++i;
                break;
            case 'h':
                lifetime.push_back(60 * 60);
                ++i;
                break;
            case 'm':
                lifetime.push_back(60);
                ++i;
                break;
            case 's':
                return 1;
        }
    }
    for (; i < lifetimeString.size(); ++i) {
        char charValue = lifetimeString[i];
        if (std::isdigit(charValue)) {
            processingNumber += std::atoi(&charValue);
            if (i + 1 < lifetimeString.size()) {
                if (std::isdigit(lifetimeString[i + 1])) {
                    processingNumber *= 10;
                    continue;
                } else if (lifetimeString[i + 1] == ':') {
                    if (hoursMuliply) {
                        lifetime.push_back(60 * 60 * processingNumber);
                        hoursMuliply = false;
                        processingNumber = 0;
                        continue;
                    }
                    if (minutesMuliply) {
                        lifetime.push_back(60 * processingNumber);
                        minutesMuliply = false;
                        processingNumber = 0;
                        continue;
                    }
                }
                if (!minutesMuliply) {
                    lifetime.push_back(processingNumber);
                    processingNumber = 0;
                    continue;
                }
                switch (lifetimeString[i + 1]) {
                    case 'w':
                        lifetime.push_back(60 * 60 * 24 * 7 * processingNumber);
                        processingNumber = 0;
                        break;
                    case 'd':
                        lifetime.push_back(60 * 60 * 24 * processingNumber);
                        processingNumber = 0;
                        break;
                    case 'h':
                        lifetime.push_back(60 * 60 * processingNumber);
                        processingNumber = 0;
                        break;
                    case 'm':
                        lifetime.push_back(60 * processingNumber);
                        processingNumber = 0;
                        break;
                    case 's':
                        lifetime.push_back(processingNumber);
                        processingNumber = 0;
                        break;
                }
            }
            lifetime.push_back(processingNumber);
            processingNumber = 0;
        }
    }
    processingNumber = 0;
    for (auto i : lifetime) {
        processingNumber += i;
    }
    return processingNumber;
}

std::string RoutingTableOperations::convertLifetime(unsigned int lifetime) {
    if (lifetime == UINT_MAX) {
        return "";
    }
    std::string result;
    unsigned int weeks = lifetime / (60 * 60 * 24 * 7);
    lifetime -= weeks * (60 * 60 * 24 * 7);
    unsigned int days = lifetime / (60 * 60 * 24);
    lifetime -= days * (60 * 60 * 24);
    unsigned int hours = lifetime / (60 * 60);
    lifetime -= hours * (60 * 60);
    unsigned int minutes = lifetime / 60;
    lifetime -= minutes * 60;
    unsigned int seconds = lifetime;
    if (weeks == 0 && days == 0 && minutes == 0 && seconds == 0) {
        result = std::to_string(hours) + "h";
        return result;
    }
    if (days > 0 || weeks > 0) {
        if (weeks > 0) {
            result += std::to_string(weeks) + "w";
        }
        if (days > 0 || weeks > 0) {
            result += std::to_string(days) + "d";
        }
        if (hours > 0 || (days > 0 && weeks < 1)) {
            if (hours < 10 && (weeks > 0 || days > 0)) {
                result += "0";
            }
            result += std::to_string(hours) + "h";
        }
        if (minutes > 0) {
            if (minutes < 10 && (weeks > 0 || days > 0 || hours > 0)) {
                result += "0";
            }
            result += std::to_string(minutes) + "m";
        }
        if (seconds > 0) {
            if (seconds < 10 && (weeks > 0 || days > 0 || hours > 0 || minutes > 0)) {
                result += "0";
            }
            result += std::to_string(seconds) + "s";
        }
    } else {
        if (hours < 10) {
            result += "0";
        }
        result += std::to_string(hours) + ":";
        if (minutes < 10) {
            result += "0";
        }
        result += std::to_string(minutes) + ":";
        if (seconds < 10) {
            result += "0";
        }
        result += std::to_string(seconds);
    }
    return result;
}

void RoutingTableOperations::saveToCSV(const std::string& filename, const std::vector<RoutingTableRow>& vectorToPrint) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << "IP/Prefix;Next-Hop;Lifetime\n";
        for (size_t i = 0; i < vectorToPrint.size(); ++i) {
            const auto& row = vectorToPrint[i];
            saveRowToCSV(file, row);
            if (i < vectorToPrint.size() - 1) {
                file << "\n";
            }
        }
    }
    file.close();
}

void RoutingTableOperations::saveRowToCSV(std::ofstream& file, const RoutingTableRow& row) {
    std::string ipAddressString = row.ipAddress.to_string();
    std::string destinationIPString = row.destinationIP.to_string();
    std::stringstream ipAddressStream, destinationIPStream;
    for (int i = 0; i <= 24; i += 8) {
        std::bitset<8> ipAddressOctet(ipAddressString.substr(i, 8));
        std::bitset<8> destinationIPOctet(destinationIPString.substr(i, 8));
        ipAddressStream << ipAddressOctet.to_ulong();
        destinationIPStream << destinationIPOctet.to_ulong();
        if (i < 24) {
            ipAddressStream << ".";
            destinationIPStream << ".";
        }
    }
    file << ipAddressStream.str() << "/" << int(row.prefix) << ";via " << destinationIPStream.str() << ";" << convertLifetime(row.lifetime);
}

void RoutingTableOperations::saveFilteredToCSV(const std::string& filename, ds::amt::IS<RoutingTableRow*>& sequence) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << "IP/Prefix;Next-Hop;Lifetime\n";
        ds::amt::IS<RoutingTableRow*>::IteratorType begin = sequence.begin();
        ds::amt::IS<RoutingTableRow*>::IteratorType end = sequence.end();
        while (begin != end) {
            saveRowToCSV(file, **begin);
            if (++begin != end) {
                file << "\n";
            }
        }
    }
    file.close();
}