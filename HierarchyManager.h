#include <libds/amt/explicit_hierarchy.h>
#include "RoutingTable.h"
#include <libds/heap_monitor.h>

struct Node {
    std::bitset<8> octet;
    RoutingTableRow* pData = nullptr;
    bool operator==(const Node& other) const {
        return octet == other.octet && pData == other.pData;
    }
};

auto matchLifetimeHierarchy = [](const Node& node, unsigned int startTime, unsigned int endTime) {
    if (node.pData != nullptr) {
        if (node.pData->lifetime >= startTime && node.pData->lifetime <= endTime) {
            return true;
        }
        return false;
    }
    return false;
};

auto matchWithAddressHierarchy = [](const Node& node, const std::bitset<32>& addressToCompare) {
    if (node.pData != nullptr) {
        std::bitset<32> ipAddress = node.pData->ipAddress;
        for (int i = 0; i < node.pData->prefix; ++i) {
            if (ipAddress[ipAddress.size() - i - 1] != addressToCompare[addressToCompare.size() - i - 1]) {
                return false;
            }
        }
        return true;
    }
    return false;
};

class HierarchyManager {
public:
    ds::amt::MultiWayEH<Node> hierarchy;

    auto findSon(ds::amt::MWEHBlock<Node>& node, std::bitset<8> octetParam) {
        auto predicate = [&](ds::amt::MemoryBlock<ds::amt::MWEHBlock<Node>*>* block) {
            return block->data_->data_.octet == octetParam;
            };
        return node.sons_->findBlockWithProperty(predicate);
    }

    HierarchyManager();
    void addBranch(std::bitset<32> sourceIP, RoutingTableRow* pVector);
    void print(ds::amt::MWEHBlock<Node>& node);
    void printNodeInfo(ds::amt::MWEHBlock<Node>& node);
    void printSons(ds::amt::MWEHBlock<Node>& node);
    std::string getOctetsToNode(ds::amt::MWEHBlock<Node>& node);
};

HierarchyManager::HierarchyManager() {
    auto& root = hierarchy.emplaceRoot();
}

void HierarchyManager::addBranch(std::bitset<32> sourceIP, RoutingTableRow* pVector) {
    auto root = hierarchy.accessRoot();
    auto firstLevel = findSon(*root, std::bitset<8>((sourceIP >> 24).to_ulong()));
    if (firstLevel != nullptr) {
        auto secondLevel = findSon(*firstLevel->data_, std::bitset<8>((sourceIP >> 16).to_ulong() & 0xFF));
        if (secondLevel != nullptr) {
            auto thirdLevel = findSon(*secondLevel->data_, std::bitset<8>((sourceIP >> 8).to_ulong() & 0xFF));
            if (thirdLevel != nullptr) {
                auto& fourthLevel = hierarchy.emplaceSon(*thirdLevel->data_, 0);
                fourthLevel.data_.octet = std::bitset<8>((sourceIP.to_ulong()) & 0xFF);
                fourthLevel.data_.pData = pVector;
            } else {
                auto& thirdLevel = hierarchy.emplaceSon(*secondLevel->data_, 0);
                thirdLevel.data_.octet = std::bitset<8>((sourceIP >> 8).to_ulong() & 0xFF);
                auto& fourthLevel = hierarchy.emplaceSon(thirdLevel, 0);
                fourthLevel.data_.octet = std::bitset<8>((sourceIP.to_ulong()) & 0xFF);
                fourthLevel.data_.pData = pVector;
            }
        } else {
            auto& secondLevel = hierarchy.emplaceSon(*firstLevel->data_, 0);
            secondLevel.data_.octet = std::bitset<8>((sourceIP >> 16).to_ulong() & 0xFF);
            auto& thirdLevel = hierarchy.emplaceSon(secondLevel, 0);
            thirdLevel.data_.octet = std::bitset<8>((sourceIP >> 8).to_ulong() & 0xFF);
            auto& fourthLevel = hierarchy.emplaceSon(thirdLevel, 0);
            fourthLevel.data_.octet = std::bitset<8>((sourceIP.to_ulong()) & 0xFF);
            fourthLevel.data_.pData = pVector;
        }
    } else {
        auto& firstLevel = hierarchy.emplaceSon(*root, 0);
        firstLevel.data_.octet = std::bitset<8>((sourceIP >> 24).to_ulong());
        auto& secondLevel = hierarchy.emplaceSon(firstLevel, 0);
        secondLevel.data_.octet = std::bitset<8>((sourceIP >> 16).to_ulong() & 0xFF);
        auto& thirdLevel = hierarchy.emplaceSon(secondLevel, 0);
        thirdLevel.data_.octet = std::bitset<8>((sourceIP >> 8).to_ulong() & 0xFF);
        auto& fourthLevel = hierarchy.emplaceSon(thirdLevel, 0);
        fourthLevel.data_.octet = std::bitset<8>((sourceIP.to_ulong()) & 0xFF);
        fourthLevel.data_.pData = pVector;
    }
}

void HierarchyManager::print(ds::amt::MWEHBlock<Node>& node) {
    size_t index = 0;
    hierarchy.processLevelOrder(&node, std::function<void(ds::amt::MWEHBlock<Node>*)>([&](ds::amt::MWEHBlock<Node>* node) {
        if (node->data_.pData != nullptr && hierarchy.level(*node) == 4) {
            RoutingTableOperations::printRow(*node->data_.pData);
            ++index;
        }
    }));
    std::cout << "-------------------------\nPrinted: " << index << " values" << std::endl;
}

void HierarchyManager::printNodeInfo(ds::amt::MWEHBlock<Node>& node) {
    if (node.parent_ == nullptr) {
        std::cout << "You are on root node!" << std::endl;
    } else {
        if (hierarchy.level(node) == 1) {
            std::cout << "Parent octet is root" << std::endl;
        } else {
            std::cout << "Parent octet value: " << node.parent_->data_.octet.to_ulong() << std::endl;
        }
        std::cout << "You are on octet number: " << hierarchy.level(node) << " Node octet value: " << node.data_.octet.to_ulong() << std::endl;
    }
    std::cout << "---------------------" << std::endl;
    if (node.sons_->isEmpty()) {
        std::cout << "No sons" << std::endl;
    } else {
        std::cout << "Number of sons: " << (hierarchy.degree(node)) << std::endl;
    }
}

void HierarchyManager::printSons(ds::amt::MWEHBlock<Node>& node) {
    std::cout << "---------------------" << std::endl;
    if (!node.sons_->isEmpty()) {
        std::cout << "#   " << (hierarchy.level(node)) + 1 << ". Octet Values" << std::endl;
        for (size_t i = 0; i < node.sons_->size(); ++i) {
            std::cout << i << ". Son octet: " << node.sons_->access(i)->data_->data_.octet.to_ulong() << std::endl;
        }
    }
    std::cout << "---------------------" << std::endl;
}

std::string HierarchyManager::getOctetsToNode(ds::amt::MWEHBlock<Node>& node) {
    switch (hierarchy.level(node)) {
    case 0:
        return "";
    case 1:
        return std::to_string(node.data_.octet.to_ulong()) + ".";
    case 2:
        return std::to_string(node.parent_->data_.octet.to_ulong()) + "." + std::to_string(node.data_.octet.to_ulong()) + ".";
    case 3:
        return std::to_string(node.parent_->parent_->data_.octet.to_ulong()) + "." + std::to_string(node.parent_->data_.octet.to_ulong()) + "." + std::to_string(node.data_.octet.to_ulong()) + ".";
    case 4:
        return std::to_string(node.parent_->parent_->parent_->data_.octet.to_ulong()) + "." + std::to_string(node.parent_->parent_->data_.octet.to_ulong()) + "." + std::to_string(node.parent_->data_.octet.to_ulong()) + "." + std::to_string(node.data_.octet.to_ulong()) + ".";
    default:
        return "";
    }
}