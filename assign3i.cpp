#include <iostream>
#include <fstream>
#include <deque>
#include <unordered_set>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include <iomanip>
#include <sstream>

using namespace std;

// Memory configuration
#define MEMORY_SIZE (64 * 1024) // 64K words
#define BLOCK_SIZE 16           // 16 words per block
#define NUM_BLOCKS (MEMORY_SIZE / BLOCK_SIZE)  // Total blocks in memory

// Cache configuration
#define L1_CACHE_SIZE (2 * 1024)    // 2K words
#define L2_CACHE_SIZE (16 * 1024)   // 16K words
#define L1_BLOCKS (L1_CACHE_SIZE / BLOCK_SIZE)
#define L2_BLOCKS (L2_CACHE_SIZE / BLOCK_SIZE)
#define L2_WAYS 4
#define BUFFER_SIZE 4   // Size for write, victim, and stream buffers

// Cache block structure
struct CacheBlock {
    bool valid;
    bool dirty;
    int tag;
    vector<int> data;
    
    CacheBlock() : valid(false), dirty(false), tag(0) {
        data.resize(BLOCK_SIZE);
    }
};

// L2 Cache Set structure
struct L2Set {
    vector<CacheBlock> ways;
    vector<int> lru_counter;
    
    L2Set() : ways(L2_WAYS), lru_counter(L2_WAYS, 0) {}
};

class ExtendedCacheSystem {
private:
    vector<CacheBlock> l1_cache;
    vector<L2Set> l2_cache;
    deque<CacheBlock> write_buffer;
    deque<CacheBlock> victim_cache;
    deque<CacheBlock> inst_stream_buffer;
    deque<CacheBlock> data_stream_buffer;
    
    // Statistics
    int l1_hits = 0;
    int l1_misses = 0;
    int l2_hits = 0;
    int l2_misses = 0;
    int victim_hits = 0;
    int write_buffer_hits = 0;
    int stream_buffer_hits = 0;
    int write_backs = 0;

    ofstream logfile;
    string current_access_type;

    void writeBackToMemory(const CacheBlock& block) {
        // Simulate write to memory
        write_backs++;
        logfile << "[WRITE BACK] Block " << block.tag << " to memory\n";
    }

public:
    ExtendedCacheSystem() : 
        l1_cache(L1_BLOCKS),
        l2_cache(L2_BLOCKS / L2_WAYS) {
        logfile.open("cache_access_log.txt");
        logfile << "Access\tAddress\t\tResult\t\tDetails\n";
        logfile << "-------------------------------------------------\n";
    }

    ~ExtendedCacheSystem() {
        // Flush write buffer before destruction
        drainWriteBuffer();
        logfile.close();
    }

    void drainWriteBuffer() {
        while (!write_buffer.empty()) {
            CacheBlock block = write_buffer.front();
            writeBackToMemory(block);
            write_buffer.pop_front();
        }
    }

    void setAccessType(bool isInstruction) {
        current_access_type = isInstruction ? "INST" : "DATA";
    }

    int getL1Index(int address) {
        return (address / BLOCK_SIZE) % L1_BLOCKS;
    }

    int getL2Index(int address) {
        return (address / BLOCK_SIZE) % (L2_BLOCKS / L2_WAYS);
    }

    int getTag(int address) {
        return address / BLOCK_SIZE;
    }

    void addToWriteBuffer(CacheBlock& block) {
        if (write_buffer.size() >= BUFFER_SIZE) {
            // Write buffer full, drain oldest entry
            CacheBlock oldest = write_buffer.front();
            writeBackToMemory(oldest);
            write_buffer.pop_front();
        }
        write_buffer.push_back(block);
        logfile << "[WRITE BUFFER] Added block " << block.tag << "\n";
    }

    void addToVictimCache(CacheBlock& block) {
        if (victim_cache.size() >= BUFFER_SIZE) {
            victim_cache.pop_front();
        }
        victim_cache.push_back(block);
    }

    void updateStreamBuffer(int address, bool isInstruction) {
        CacheBlock block;
        block.valid = true;
        block.tag = getTag(address + BLOCK_SIZE);  // Prefetch next block
        
        auto& buffer = isInstruction ? inst_stream_buffer : data_stream_buffer;
        if (buffer.size() >= BUFFER_SIZE) {
            buffer.pop_front();
        }
        buffer.push_back(block);
    }

    void updateL2LRU(int set_index, int way_index) {
        for (int i = 0; i < L2_WAYS; i++) {
            if (i != way_index) {
                l2_cache[set_index].lru_counter[i]++;
            }
        }
        l2_cache[set_index].lru_counter[way_index] = 0;
    }

    void loadFromMemory(int address, CacheBlock& block) {
        block.valid = true;
        block.dirty = false;
        block.tag = getTag(address);
        for(int i = 0; i < BLOCK_SIZE; i++) {
            block.data[i] = address + i;
        }
    }
    
    bool accessMemory(int address, bool isInstruction = false, bool isWrite = false) {
        setAccessType(isInstruction);
        stringstream logEntry;
        logEntry << current_access_type << "\t0x" << hex << setw(4) << setfill('0') 
                 << address << dec << "\t";

        int l1_index = getL1Index(address);
        int tag = getTag(address);
        
        // Check L1 Cache
        if (l1_cache[l1_index].valid && l1_cache[l1_index].tag == tag) {
            l1_hits++;
            if (isWrite) {
                l1_cache[l1_index].dirty = true;
                logEntry << "L1 WRITE HIT\t";
            } else {
                logEntry << "L1 READ HIT\t";
            }
            logEntry << "Block: " << (address/BLOCK_SIZE);
            logfile << logEntry.str() << endl;
            return true;
        }

        l1_misses++;
        logEntry << "L1 MISS\t";

        // Check Victim Cache
        for (auto it = victim_cache.begin(); it != victim_cache.end(); ++it) {
            if (it->valid && it->tag == tag) {
                victim_hits++;
                logEntry << "VICTIM HIT\tReplaced L1[" << l1_index << "]";
                
                if (l1_cache[l1_index].valid) {
                    if (l1_cache[l1_index].dirty) {
                        addToWriteBuffer(l1_cache[l1_index]);
                    } else {
                        addToVictimCache(l1_cache[l1_index]);
                    }
                    logEntry << " (Moved L1[" << l1_index << "] to " 
                             << (l1_cache[l1_index].dirty ? "write buffer" : "victim cache") << ")";
                }
                
                l1_cache[l1_index] = *it;
                if (isWrite) {
                    l1_cache[l1_index].dirty = true;
                }
                victim_cache.erase(it);
                logfile << logEntry.str() << endl;
                return true;
            }
        }

        // Check Stream Buffers
        auto& stream_buffer = isInstruction ? inst_stream_buffer : data_stream_buffer;
        for (auto it = stream_buffer.begin(); it != stream_buffer.end(); ++it) {
            if (it->valid && it->tag == tag) {
                stream_buffer_hits++;
                logEntry << "STREAM HIT\t";
                
                if (l1_cache[l1_index].valid) {
                    if (l1_cache[l1_index].dirty) {
                        addToWriteBuffer(l1_cache[l1_index]);
                    } else {
                        addToVictimCache(l1_cache[l1_index]);
                    }
                    logEntry << "Replaced L1[" << l1_index << "]";
                }
                
                l1_cache[l1_index] = *it;
                if (isWrite) {
                    l1_cache[l1_index].dirty = true;
                }
                stream_buffer.erase(it);
                logfile << logEntry.str() << endl;
                return true;
            }
        }

        // Check L2 Cache
        int l2_set_index = getL2Index(address);
        for (int i = 0; i < L2_WAYS; i++) {
            if (l2_cache[l2_set_index].ways[i].valid && 
                l2_cache[l2_set_index].ways[i].tag == tag) {
                l2_hits++;
                updateL2LRU(l2_set_index, i);
                logEntry << "L2 HIT\t\tSet:" << l2_set_index << " Way:" << i;
                
                if (l1_cache[l1_index].valid) {
                    if (l1_cache[l1_index].dirty) {
                        addToWriteBuffer(l1_cache[l1_index]);
                    } else {
                        addToVictimCache(l1_cache[l1_index]);
                    }
                    logEntry << " (Moved L1[" << l1_index << "] to "
                             << (l1_cache[l1_index].dirty ? "write buffer" : "victim cache") << ")";
                }
                l1_cache[l1_index] = l2_cache[l2_set_index].ways[i];
                if (isWrite) {
                    l1_cache[l1_index].dirty = true;
                }
                
                updateStreamBuffer(address, isInstruction);
                logEntry << " Prefetch queued";
                logfile << logEntry.str() << endl;
                return true;
            }
        }

        // L2 Cache miss
        l2_misses++;
        logEntry << "L2 MISS\t\tLoading from memory";

        // Find replacement way in L2
        int victim_way = 0;
        int max_lru = -1;
        for (int i = 0; i < L2_WAYS; i++) {
            if (!l2_cache[l2_set_index].ways[i].valid) {
                victim_way = i;
                break;
            }
            if (l2_cache[l2_set_index].lru_counter[i] > max_lru) {
                max_lru = l2_cache[l2_set_index].lru_counter[i];
                victim_way = i;
            }
        }

        // Write back if dirty
        if (l2_cache[l2_set_index].ways[victim_way].valid && 
            l2_cache[l2_set_index].ways[victim_way].dirty) {
            addToWriteBuffer(l2_cache[l2_set_index].ways[victim_way]);
            logEntry << " (Writeback L2[" << l2_set_index << "][" << victim_way << "] to write buffer)";
        }

        // Load from memory
        loadFromMemory(address, l2_cache[l2_set_index].ways[victim_way]);
        updateL2LRU(l2_set_index, victim_way);
        logEntry << " -> L2[" << l2_set_index << "][" << victim_way << "]";

        // Move to L1
        if (l1_cache[l1_index].valid) {
            if (l1_cache[l1_index].dirty) {
                addToWriteBuffer(l1_cache[l1_index]);
            } else {
                addToVictimCache(l1_cache[l1_index]);
            }
            logEntry << " (Moved L1[" << l1_index << "] to "
                     << (l1_cache[l1_index].dirty ? "write buffer" : "victim cache") << ")";
        }
        l1_cache[l1_index] = l2_cache[l2_set_index].ways[victim_way];
        if (isWrite) {
            l1_cache[l1_index].dirty = true;
        }

        // Prefetch
        updateStreamBuffer(address, isInstruction);
        logEntry << " Prefetch queued";
        logfile << logEntry.str() << endl;

        return false;
    }

    void printDetailedStats() {
        cout << "\nDetailed Cache Statistics:\n";
        cout << "L1 Hits: " << l1_hits << "\n";
        cout << "L1 Misses: " << l1_misses << "\n";
        cout << "L2 Hits: " << l2_hits << "\n";
        cout << "L2 Misses: " << l2_misses << "\n";
        cout << "Victim Cache Hits: " << victim_hits << "\n";
        cout << "Stream Buffer Hits: " << stream_buffer_hits << "\n";
        cout << "Write Buffer Operations: " << write_backs << "\n";
        cout << "Total Hit Rate: " 
             << (float)(l1_hits + l2_hits + victim_hits + stream_buffer_hits)/
                (l1_hits + l1_misses) * 100 << "%\n";
    }
};

void runTestCasesFromFile(ExtendedCacheSystem& cache, const string& filename) {
    ifstream inputFile(filename);
    if (!inputFile) {
        cerr << "Error opening input file: " << filename << endl;
        return;
    }

    string line;
    int lineNum = 0;
    while (getline(inputFile, line)) {
        lineNum++;
        if (line.empty() || line[0] == '#') continue;
        
        try {
            bool isWrite = false;
            bool isInstruction = false;
            int address;
            
            if (line[0] == '+') {
                isWrite = true;
                address = stoi(line.substr(1));
            } else if (line[0] == 'i') {
                isInstruction = true;
                address = stoi(line.substr(1));
            } else {
                address = stoi(line);
            }

            if (address < 0 || address >= MEMORY_SIZE) {
                cerr << "Line " << lineNum << ": Address out of range: " << address << endl;
                continue;
            }
            cache.accessMemory(address, isInstruction, isWrite);
        } catch (...) {
            cerr << "Line " << lineNum << ": Invalid address: " << line << endl;
        }
    }
    inputFile.close();
}

int main() {
    cout << "Starting Cache Simulation...\n";
    
    ExtendedCacheSystem cache;
    cout << "\nRunning test cases from input.txt...\n";
    runTestCasesFromFile(cache, "input.txt");
    cache.printDetailedStats();

    cout << "\nSimulation complete. Detailed log saved to cache_access_log.txt\n";
    return 0;
}