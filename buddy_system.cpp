// Buddy System Memory Allocation Simulator
// Memory size is a power of two. Requests are rounded UP to a power of two.
// Blocks are split into two "buddies" when needed and merged back when freed.

#include <iostream>
#include <vector>
#include <set>
#include <map>
using namespace std;

class BuddyAllocator {
    int totalSize;   // total memory in KB (power of two)
    int maxOrder;    // totalSize = 2^maxOrder

    // freeList[k] = start addresses of all free blocks of size 2^k
    vector<set<int>> freeList;

    // address -> {order k, requested size}
    map<int, pair<int, int>> allocated;

public:
    BuddyAllocator(int total) {
        totalSize = total;
        maxOrder = 0;
        while ((1 << maxOrder) < total) maxOrder++;

        freeList.resize(maxOrder + 1);
        freeList[maxOrder].insert(0);   // at the start: one big free block at address 0
    }

    // Returns the start address, or -1 if allocation fails
    int allocate(int size) {
        if (size <= 0 || size > totalSize) return -1;

        // Step 1: round up to power of two -> find order k
        int k = 0;
        while ((1 << k) < size) k++;

        // Step 2: find the smallest order j >= k that has a free block
        int j = k;
        while (j <= maxOrder && freeList[j].empty()) j++;
        if (j > maxOrder) return -1;    // no block big enough

        // Step 3: take that block out of the free list
        int addr = *freeList[j].begin();
        freeList[j].erase(freeList[j].begin());

        // Step 4: split until the block is the size we need
        while (j > k) {
            j--;
            // block splits into two halves: [addr] and [addr + 2^j]
            // keep the first half, put the second half (the buddy) in the free list
            freeList[j].insert(addr + (1 << j));
            cout << "   split block into two buddies of size " << (1 << j) << " KB\n";
        }

        // Step 5: remember the allocation
        allocated[addr] = make_pair(k, size);
        return addr;
    }

    // Returns true if the address was a valid allocation
    bool deallocate(int addr) {
        auto it = allocated.find(addr);
        if (it == allocated.end()) return false;

        int k = it->second.first;
        allocated.erase(it);

        // Merge with the buddy as long as the buddy is also free
        while (k < maxOrder) {
            int buddy = addr ^ (1 << k);   // buddy address trick
            if (freeList[k].count(buddy) == 0) break;   // buddy not free -> stop

            freeList[k].erase(buddy);
            addr = min(addr, buddy);       // merged block starts at the lower address
            k++;
            cout << "   merged buddies into a block of size " << (1 << k) << " KB\n";
        }

        freeList[k].insert(addr);
        return true;
    }

    void report() {
        cout << "\n========== MEMORY STATUS ==========\n";

        // Allocated blocks + internal fragmentation
        cout << "Allocated blocks:\n";
        if (allocated.empty()) cout << "  (none)\n";

        int totalFrag = 0;
        for (auto &a : allocated) {
            int blockSize = 1 << a.second.first;
            int requested = a.second.second;
            int frag = blockSize - requested;
            totalFrag += frag;
            cout << "  Address " << a.first
                 << " | Requested " << requested << " KB"
                 << " | Block " << blockSize << " KB"
                 << " | Internal fragmentation " << frag << " KB\n";
        }
        cout << "Total internal fragmentation: " << totalFrag << " KB\n";

        // Free-block availability
        cout << "\nFree blocks:\n";
        int totalFree = 0, largest = 0;
        for (int k = 0; k <= maxOrder; k++) {
            if (freeList[k].empty()) continue;
            cout << "  Size " << (1 << k) << " KB : " << freeList[k].size() << " block(s) at address ";
            for (int addr : freeList[k]) cout << addr << " ";
            cout << "\n";
            totalFree += (int)freeList[k].size() * (1 << k);
            largest = max(largest, 1 << k);
        }
        if (totalFree == 0) cout << "  (none)\n";

        cout << "Total free memory: " << totalFree << " KB\n";
        cout << "Largest free block: " << largest << " KB\n";
        cout << "===================================\n";
    }
};

int main() {
    int total;
    cout << "Buddy System Memory Allocation Simulator\n";
    cout << "Enter total memory size in KB (power of 2, e.g. 1024): ";
    cin >> total;

    // A number is a power of two if it has exactly one bit set
    if (total <= 0 || (total & (total - 1)) != 0) {
        cout << "Memory size must be a power of 2.\n";
        return 1;
    }

    BuddyAllocator mem(total);

    int choice;
    do {
        cout << "\n1. Allocate\n2. Free\n3. Show status\n4. Exit\nChoice: ";
        cin >> choice;

        if (choice == 1) {
            int size;
            cout << "Enter size to allocate (KB): ";
            cin >> size;
            int addr = mem.allocate(size);
            if (addr == -1) cout << "Allocation failed (no free block large enough).\n";
            else cout << "Allocated at address " << addr << "\n";
        }
        else if (choice == 2) {
            int addr;
            cout << "Enter address to free: ";
            cin >> addr;
            if (mem.deallocate(addr)) cout << "Block freed.\n";
            else cout << "No allocated block starts at that address.\n";
        }
        else if (choice == 3) {
            mem.report();
        }
    } while (choice != 4);

    cout << "Bye!\n";
    return 0;
}
