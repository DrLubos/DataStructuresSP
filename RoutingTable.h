#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

struct RoutingTableRow {
    unsigned int lifetime;
    unsigned char firstOctet;
    unsigned char secondOctet;
    unsigned char thirdOctet;
    unsigned char fourthOctet;
    unsigned char prefix;
    unsigned char nextHopFirstOctet;
    unsigned char nextHopSecondOctet;
    unsigned char nextHopThirdOctet;
    unsigned char nextHopFourthOctet;
};

class RoutingTable {
private:
    std::vector<RoutingTableRow> routingTable;
public:
    void loadFromCSV(const std::string& filename);
    void printAll();
    std::vector<unsigned char> processIPAddress(const std::string& ipAddressString);
    unsigned int processLifetime(const std::string& lifetimeString);
    void saveToCSV(const std::string& filename);
    std::string convertLifetime(unsigned int lifetime);
    std::vector<RoutingTableRow> getRoutingTable() { return routingTable; }
};

void RoutingTable::loadFromCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Error: Could not open file.\n");
    }
    std::string line;
    unsigned int rowNumber = 1;
    std::getline(file, line);
    while (std::getline(file, line)) {
        rowNumber++;
        std::istringstream ss(line);
        std::vector<std::string> cells;
        std::string cell;
        while (std::getline(ss, cell, ';')) {
            cells.push_back(cell);
        }
        if (cells.size() == 5 || cells.size() == 4) {
            RoutingTableRow entry;
            std::vector<unsigned char> octets = this->processIPAddress(cells[1]);
            for (size_t i = 0; i < octets.size(); i++) {
                switch (i) {
                case 0:
                    entry.firstOctet = octets[i];
                    break;
                case 1:
                    entry.secondOctet = octets[i];
                    break;
                case 2:
                    entry.thirdOctet = octets[i];
                    break;
                case 3:
                    entry.fourthOctet = octets[i];
                    break;
                case 4:
                    entry.prefix = octets[i];
                    break;
                }
            }
            octets.clear();
            if (cells[3][0] == 'v' && cells[3][1] == 'i' && cells[3][2] == 'a') {
                octets = cells[3][3] == ' ' ? this->processIPAddress(cells[3].substr(4)) : this->processIPAddress(cells[3].substr(3));
                for (size_t i = 0; i < octets.size(); i++) {
                    switch (i) {
                    case 0:
                        entry.nextHopFirstOctet = octets[i];
                        break;
                    case 1:
                        entry.nextHopSecondOctet = octets[i];
                        break;
                    case 2:
                        entry.nextHopThirdOctet = octets[i];
                        break;
                    case 3:
                        entry.nextHopFourthOctet = octets[i];
                        break;
                    }
                }
            } else {
                std::cout << "Supported only next-hop ip address";
            }
            if (cells.size() > 4 && (isdigit(cells[4][0]) || isalpha(cells[4][0]))) {
                entry.lifetime = this->processLifetime(cells[4]);
            } else {
                entry.lifetime = UINT_MAX;
            }
            routingTable.push_back(entry);
        } else {
            throw std::runtime_error("Error: Invalid CSV format. Error found on line: " + std::to_string(rowNumber) + "\n");
        }
    }
}

void RoutingTable::printAll() {
    for (const auto& row : routingTable) {
        std::cout << "===============================================" << std::endl;
        std::cout << "Destination: " << int(row.firstOctet) << "." << int(row.secondOctet) << "." << int(row.thirdOctet) << "." << int(row.fourthOctet) << "/" << int(row.prefix) << std::endl;
        std::cout << "Next Hop: " << int(row.nextHopFirstOctet) << "." << int(row.nextHopSecondOctet) << "." << int(row.nextHopThirdOctet) << "." << int(row.nextHopFourthOctet) << std::endl;
        std::cout << "Lifetime: ";
        row.lifetime > 59 ? std::cout << row.lifetime << "(s) " << this->convertLifetime(row.lifetime) << std::endl : std::cout << row.lifetime << "s" << std::endl;
    }
}

std::vector<unsigned char> RoutingTable::processIPAddress(const std::string& ipAddressString) {
    std::vector<unsigned char> octets;
    std::istringstream iss(ipAddressString);
    std::string octetString;
    while (std::getline(iss, octetString, '.')) {
        int octet = std::stoi(octetString);
        if (octet >= 0 && octet < 256) {
            octets.push_back(octet);
        }
    }
    if (octets.size() > 4) {
        throw std::runtime_error("Error: Invalid IP address format.\n");
    }
    while (octets.size() < 4) {
        octets.push_back(0);
    }
    size_t startingPrefixIndex = ipAddressString.find('/');
    if (startingPrefixIndex != std::string::npos) {
         octets.push_back(std::stoi(ipAddressString.substr(startingPrefixIndex + 1)));
    }
    return octets;
}

unsigned int RoutingTable::processLifetime(const std::string& lifetimeString) {
    std::vector<unsigned int> lifetime;
    unsigned int processingNumber = 0;
    bool hoursMuliply = true;
    bool minutesMuliply = true;
    size_t i = 0;
    if (lifetimeString.size() > 0 && !isdigit(lifetimeString[0])) {
        switch (lifetimeString[0]) {
            case 'w':
                lifetime.push_back(60 * 60 * 24 * 7);
                i++;
                break;
            case 'd':
                lifetime.push_back(60 * 60 * 24);
                i++;
                break;
            case 'h':
                lifetime.push_back(60 * 60);
                i++;
                break;
            case 'm':
                lifetime.push_back(60);
                i++;
                break;
            case 's':
                return 1;
        }
    }
    for (; i < lifetimeString.size(); i++) {
        char charValue = lifetimeString[i];
        if (isdigit(charValue)) {
            processingNumber += std::atoi(&charValue);
            if (i + 1 < lifetimeString.size()) {
                if (isdigit(lifetimeString[i + 1])) {
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

std::string RoutingTable::convertLifetime(unsigned int lifetime) {
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

void RoutingTable::saveToCSV(const std::string& filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << "Destination;Next-Hop;Lifetime\n";
        for (size_t i = 0; i < this->routingTable.size(); i++) {
            file << int(this->routingTable[i].firstOctet) << "." << int(this->routingTable[i].secondOctet) << "." << int(this->routingTable[i].thirdOctet) << "." << int(this->routingTable[i].fourthOctet) << "/" << int(this->routingTable[i].prefix);
            file << ";via " << int(this->routingTable[i].nextHopFirstOctet) << "." << int(this->routingTable[i].nextHopSecondOctet) << "." << int(this->routingTable[i].nextHopThirdOctet) << "." << int(this->routingTable[i].nextHopFourthOctet);
            file << ";" << this->convertLifetime(this->routingTable[i].lifetime);
            if (i != this->routingTable.size() - 1) {
                file << std::endl;
            }
        }
    }
    file.close();
}