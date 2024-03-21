#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <bitset>
#include <functional>

bool isStringNumeric(const std::string& str) {
    for (char c : str) {
        if (!std::isdigit(c)) {
            return false;
        }
    }
    return true;
}

struct RoutingTableRow {
    std::bitset<32> ipAddress;
    std::bitset<32> destinationIP;
    unsigned int lifetime;
    unsigned char prefix;
};

class RoutingTableOperations {
private:
    bool isStringIPMaskFormat(const std::string& str);
public:
    std::bitset<32> processIPAddress(const std::string& ipAddressString, unsigned char* prefix);
    unsigned int processLifetime(const std::string& lifetimeString);
    std::string convertLifetime(unsigned int lifetime);
    void loadFromCSV(const std::string& filename, std::vector<RoutingTableRow>& saveToVector);
    void print(const std::vector<RoutingTableRow>& vectorToPrint);
    void saveToCSV(const std::string& filename, const std::vector<RoutingTableRow>& vectorToPrint);
};

auto matchLifetime = [](const RoutingTableRow& row, unsigned int startTime, unsigned int endTime) {
    if (row.lifetime >= startTime && row.lifetime <= endTime) {
        return true;
    }
    return false;
};

auto matchWithAddress = [](const RoutingTableRow& row, const std::string& addressToCompare) {
    int N = row.prefix;
    std::bitset<32> ipAddress = row.ipAddress;
    std::bitset<32> ipToCompare = RoutingTableOperations().processIPAddress(addressToCompare, nullptr);
    for (int i = 0; i < N; i++) {
        if (ipAddress[i] != ipToCompare[i]) {
            return false;
        }
    }    
    return true;
};

class Filter {
public:
    Filter() {}
    template<typename P, typename Iterator>
    void filterAndAppend(Iterator begin, Iterator end, std::function<bool(const P&)> pred, std::vector<P>& output) {
        std::vector<P*> filtered;
        for (auto it = begin; it != end; ++it) {
            if (pred(*it)) {
                output.push_back(*it);
            }
        }
    }
};

bool RoutingTableOperations::isStringIPMaskFormat(const std::string& str) {
    for (char c : str) {
        if (!std::isdigit(c) && c != '/' && c != '.' && c != ' ') {
            return false;
        }
    }
    return true;
}

void RoutingTableOperations::loadFromCSV(const std::string& filename, std::vector<RoutingTableRow>& saveToVector) {
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
            unsigned char prefix = 0;
            try {
                entry.ipAddress = this->processIPAddress(cells[1], &entry.prefix);
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << " Check line " << rowNumber << std::endl;
                continue;
            }
            if (cells[3][0] == 'v' && cells[3][1] == 'i' && cells[3][2] == 'a') {
                entry.destinationIP = cells[3][3] == ' ' ? this->processIPAddress(cells[3].substr(4), nullptr) : this->processIPAddress(cells[3].substr(3), nullptr);
            } else {
                std::cout << "Supported only next-hop ip address, check line " << rowNumber << std::endl;
            }
            if (cells.size() > 4 && (std::isdigit(cells[4][0]) || std::isalpha(cells[4][0]))) {
                entry.lifetime = this->processLifetime(cells[4]);
            } else {
                entry.lifetime = UINT_MAX;
            }
            saveToVector.push_back(entry);
        } else {
            throw std::runtime_error("Error: Invalid CSV format. Error found on line: " + std::to_string(rowNumber) + "\n");
        }
    }
}

void RoutingTableOperations::print(const std::vector<RoutingTableRow>& vectorToPrint) {
    std::for_each(vectorToPrint.begin(), vectorToPrint.end(), [=](const RoutingTableRow& row) {
        std::cout << "==========================================" << std::endl;
        std::cout << "IP Address: ";
        for (size_t i = 0; i < row.ipAddress.size(); i += 8) {
            std::cout << std::bitset<8>(row.ipAddress.to_ulong() >> i).to_ulong();
            if (i + 8 < row.ipAddress.size()) {
                std::cout << ".";
            }
        }
        std::cout << "/" << int(row.prefix) << std::endl;
        std::cout << "Next Hop: ";
        for (size_t i = 0; i < row.destinationIP.size(); i += 8) {
            std::cout << std::bitset<8>(row.destinationIP.to_ulong() >> i).to_ulong();
            if (i + 8 < row.destinationIP.size()) {
                std::cout << ".";
            }
        }
        std::cout << std::endl;
        std::cout << "Lifetime: ";
        row.lifetime > 59 ? std::cout << row.lifetime << "(s) " << this->convertLifetime(row.lifetime) << std::endl : std::cout << row.lifetime << "s" << std::endl;
    });
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
            std::string ipAddressString = row.ipAddress.to_string();
            std::string destinationIPString = row.destinationIP.to_string();
            std::stringstream ipAddressStream, destinationIPStream;
            for (int i = 24; i >= 0; i -= 8) {
                std::bitset<8> ipAddressOctet(ipAddressString.substr(i, 8));
                std::bitset<8> destinationIPOctet(destinationIPString.substr(i, 8));
                ipAddressStream << ipAddressOctet.to_ulong() << ".";
                destinationIPStream << destinationIPOctet.to_ulong() << ".";
            }
            std::string ipAddressDec = ipAddressStream.str();
            std::string destinationIPDec = destinationIPStream.str();
            ipAddressDec.pop_back();
            destinationIPDec.pop_back();
            file << ipAddressDec << "/" << int(row.prefix) << ";via " << destinationIPDec << ";" << this->convertLifetime(row.lifetime);
            if (i < vectorToPrint.size() - 1) {
                file << "\n";
            }
        }
    }    
    file.close();
}

std::bitset<32> RoutingTableOperations::processIPAddress(const std::string& ipAddressString, unsigned char* prefix) {
    std::bitset<32> ipAddressBits;
    std::istringstream iss(ipAddressString);
    std::string octetString;
    int index = 3;
    while (std::getline(iss, octetString, '.')) {
        if (!this->isStringIPMaskFormat(octetString)) {
            std::cout << octetString << std::endl;
            throw std::runtime_error("Error: Invalid IP address format.\n");
        }
        int octet = std::stoi(octetString);
        if (octet >= 0 && octet < 256) {
            ipAddressBits |= (std::bitset<32>(octet) << (24 - index * 8));
        }
        index--;
    }
    size_t startingPrefixIndex = ipAddressString.find('/');
    if (startingPrefixIndex != std::string::npos) {
        std::string prefixString = ipAddressString.substr(startingPrefixIndex + 1);
        if (!this->isStringIPMaskFormat(prefixString)) {
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