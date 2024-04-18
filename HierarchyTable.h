#include <libds/amt/explicit_hierarchy.h>
#include "RoutingTable.h"

struct Node {
    std::bitset<8> octet;
    RoutingTableRow* pData;
    bool operator==(const Node& other) const {
        return octet == other.octet && pData == other.pData;
    }
};

class MyHierarchy {
    public:
        ds::amt::MultiWayExplicitHierarchy<Node> hierarchy = ds::amt::MultiWayExplicitHierarchy<Node>();

        auto findSon(ds::amt::MultiWayExplicitHierarchyBlock<Node>& node, std::bitset<8> octetParam) {
            auto predicate = [&](ds::amt::MemoryBlock<ds::amt::MultiWayExplicitHierarchyBlock<Node>*>* block) {
                return block->data_->data_.octet == octetParam;
            };
            auto result = node.sons_->findBlockWithProperty(predicate);
            return result;
        }
        auto findLeaf(ds::amt::MultiWayExplicitHierarchyBlock<Node>& node, RoutingTableRow* pVector){
            auto predicate = [&pVector](ds::amt::MemoryBlock<ds::amt::MultiWayExplicitHierarchyBlock<Node>*>* block) {
                return block->data_->data_.pData == pVector;
            };
            auto result = node.sons_->findBlockWithProperty(predicate);
            return result;
        }

        MyHierarchy();
        void addBranch(ds::amt::MultiWayExplicitHierarchy<Node>& hierarchyParam, std::bitset<32> sourceIP, RoutingTableRow* pVector);
        void printNodeInfo(ds::amt::MultiWayExplicitHierarchyBlock<Node>& node);
        void printSons(ds::amt::MultiWayExplicitHierarchyBlock<Node>& node);
        std::string getOctetsToNode(ds::amt::MultiWayExplicitHierarchyBlock<Node>& node);
        void print(ds::amt::MultiWayExplicitHierarchyBlock<Node>& node);
};

MyHierarchy::MyHierarchy() {
    auto& root = hierarchy.emplaceRoot();
}

void MyHierarchy::addBranch(ds::amt::MultiWayExplicitHierarchy<Node>& hierarchyParam, std::bitset<32> sourceIP, RoutingTableRow* pVector) {
    auto root = hierarchyParam.accessRoot();
    auto firstLevel = findSon(*root, std::bitset<8>((sourceIP >> 24).to_ulong()));
    if (firstLevel != nullptr) {
        auto secondLevel = findSon(*firstLevel->data_, std::bitset<8>((sourceIP >> 16).to_ulong() & 0xFF));
        if (secondLevel != nullptr) {
            auto thirdLevel = findSon(*secondLevel->data_, std::bitset<8>((sourceIP >> 8).to_ulong() & 0xFF));
            if (thirdLevel != nullptr) {
                auto fourthLevel = findLeaf(*thirdLevel->data_, pVector);
                if (fourthLevel != nullptr) {
                    return;
                } else {
                    auto& fourthLevel = hierarchyParam.emplaceSon(*thirdLevel->data_, 0);
                    fourthLevel.data_.octet = std::bitset<8>((sourceIP.to_ulong()) & 0xFF);
                    fourthLevel.data_.pData = pVector;
                }
            } else {
                auto& thirdLevel = hierarchyParam.emplaceSon(*secondLevel->data_, 0);
                thirdLevel.data_.octet = std::bitset<8>((sourceIP >> 8).to_ulong() & 0xFF);
                auto& fourthLevel = hierarchyParam.emplaceSon(thirdLevel, 0);
                fourthLevel.data_.octet = std::bitset<8>((sourceIP.to_ulong()) & 0xFF);
                fourthLevel.data_.pData = pVector;
            }
        } else {
            auto& secondLevel = hierarchyParam.emplaceSon(*firstLevel->data_, 0);
            secondLevel.data_.octet = std::bitset<8>((sourceIP >> 16).to_ulong() & 0xFF);
            auto& thirdLevel = hierarchyParam.emplaceSon(secondLevel, 0);
            thirdLevel.data_.octet = std::bitset<8>((sourceIP >> 8).to_ulong() & 0xFF);
            auto& fourthLevel = hierarchyParam.emplaceSon(thirdLevel, 0);
            fourthLevel.data_.octet = std::bitset<8>((sourceIP.to_ulong()) & 0xFF);
            fourthLevel.data_.pData = pVector;
        }
    } else {
        auto& firstLevel = hierarchyParam.emplaceSon(*root, 0);
        firstLevel.data_.octet = std::bitset<8>((sourceIP >> 24).to_ulong());
        auto& secondLevel = hierarchyParam.emplaceSon(firstLevel, 0);
        secondLevel.data_.octet = std::bitset<8>((sourceIP >> 16).to_ulong() & 0xFF);
        auto& thirdLevel = hierarchyParam.emplaceSon(secondLevel, 0);
        thirdLevel.data_.octet = std::bitset<8>((sourceIP >> 8).to_ulong() & 0xFF);
        auto& fourthLevel = hierarchyParam.emplaceSon(thirdLevel, 0);
        fourthLevel.data_.octet = std::bitset<8>((sourceIP.to_ulong()) & 0xFF);
        fourthLevel.data_.pData = pVector;
    }
}

void MyHierarchy::printNodeInfo(ds::amt::MultiWayExplicitHierarchyBlock<Node>& node) {
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

void MyHierarchy::printSons(ds::amt::MultiWayExplicitHierarchyBlock<Node>& node) {
    std::cout << "---------------------" << std::endl;
    if (!node.sons_->isEmpty()) {
        std::cout << "#   " << (hierarchy.level(node)) + 1 << ". Octet Values" << std::endl;
        for (size_t i = 0; i < node.sons_->size(); ++i) {
            std::cout << i << ". Son octet: " << node.sons_->access(i)->data_->data_.octet.to_ulong() << std::endl;
        }
    }
    std::cout << "---------------------" << std::endl;
}

std::string MyHierarchy::getOctetsToNode(ds::amt::MultiWayExplicitHierarchyBlock<Node>& node) {
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

void MyHierarchy::print(ds::amt::MultiWayExplicitHierarchyBlock<Node>& node) {
    size_t index = 0;
    hierarchy.processPostOrder(&node, std::function<void(ds::amt::MultiWayExplicitHierarchyBlock<Node>*)>([&](ds::amt::MultiWayExplicitHierarchyBlock<Node>* node) {
        if (node->data_.pData != nullptr) {
            RoutingTableOperations::printRow(*node->data_.pData);
            ++index;
        }
    }));
    std::cout << "-------------------------\nPrinted: " << index << " values" << std::endl;
}