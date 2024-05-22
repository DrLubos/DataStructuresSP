#include <libds/adt/sorts.h>
#include <libds/heap_monitor.h>

auto comparePrefix = [](const RoutingTableRow* first, const RoutingTableRow* second) {
    if (first->ipAddress.to_ulong() < second->ipAddress.to_ulong()) {
        return true;
    }
    if (first->ipAddress.to_ulong() == second->ipAddress.to_ulong()) {
        return first->prefix < second->prefix;
    }
    return false;
};

auto compareTime = [](const RoutingTableRow* first, const RoutingTableRow* second) {
    return first->lifetime < second->lifetime;
};

class SortingManager {
public:
    template<typename T, typename Comp>
    static void sortData(ds::amt::IS<T*>& sequence, Comp& comparator) {
        if (sequence.isEmpty()) {
            std::cout << "========================= No data to sort! =========================" << std::endl;
            return;
        }
        auto sorting = ds::adt::QuickSort<T*>();
        sorting.sort(sequence, comparator);
        std::cout << "============V====== By IP ======V======== Data Sorted ========V================ By Lifetime ================V" << std::endl;
        for (auto& item : sequence) {
            RoutingTableOperations::sortPrint(*item);
        }
    }
};