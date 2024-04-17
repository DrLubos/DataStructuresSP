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
        void printSons(ds::amt::MultiWayExplicitHierarchyBlock<Node>& node);
};

MyHierarchy::MyHierarchy() {
    auto& root = hierarchy.emplaceRoot();
    //hierarchy = ds::amt::MultiWayExplicitHierarchy<Node>();
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

void MyHierarchy::printSons(ds::amt::MultiWayExplicitHierarchyBlock<Node>& node) {
    if (node.parent_ == nullptr) {
        std::cout << "Root octet: " << node.data_.octet << std::endl;
    } else {
        std::cout << "Parent octet: " << node.parent_->data_.octet << std::endl;
        std::cout << "Node octet: " << node.data_.octet << std::endl;
    }
    if (node.sons_->isEmpty()) {
        std::cout << "No sons" << std::endl;
    } else {
        std::cout << "#   " << hierarchy.level(node) << ". Octet Values" << std::endl;
        for (size_t i = 0; i < node.sons_->size(); ++i) {
            std::cout << i << ".Son octet: " << node.sons_->access(i)->data_->data_.octet << std::endl;
        }
    }
}