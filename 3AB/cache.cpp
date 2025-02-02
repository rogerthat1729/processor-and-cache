#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

int log2(int n)
{
    int ans = 0;
    while (n > 1)
    {
        n /= 2;
        ans++;
    }
    return ans;
}

struct Block
{
    vector<ll> words;
    ll tag;
    bool valid;
    bool dirty;
    Block(int numwords)
    {
        words = vector<ll>(numwords, 0);
        tag = -1;
        valid = false;
        dirty = false;
    }
    void create(ll address, ll tag, ll numwords)
    {
        this->tag = tag;
        valid = true;
        dirty = false;
        for (int i = 0; i < numwords; i++)
            words[i] = (address + (i * 4));
    }
};

class FIFOCache
{
public:
    vector<vector<Block>> data;
    vector<unordered_map<ll, int>> exists;

    int numsets;
    int numblocks;
    int numwords;
    int bytewidth;
    int setwidth;
    string allocate;
    string write;

    int total_loads = 0;
    int total_stores = 0;
    int load_hits = 0;
    int load_misses = 0;
    int store_hits = 0;
    int store_misses = 0;
    ll total_cycles = 0;

    int hit_time = 1;
    int miss_penalty = 100;
    int block_miss_penalty;

    FIFOCache(int numsets, int numblocks, int numwords, string allocate, string write)
    {
        this->numsets = numsets;
        this->numblocks = numblocks;
        this->numwords = numwords;
        this->bytewidth = log2(4 * numwords);
        this->setwidth = log2(numsets);
        this->allocate = allocate;
        this->write = write;

        this->block_miss_penalty = miss_penalty * numwords;

        data = vector<vector<Block>>(numsets, vector<Block>());
        exists = vector<unordered_map<ll, int>>(numsets, unordered_map<ll, int>());
    }

    void operate(vector<pair<char, ll>> instructions)
    {
        for (pair<char, ll> p : instructions)
        {
            if (p.first == 'l')
            {
                total_loads++;
                read(p.second);
                // printSet((p.second>>2) % numsets);
                // printData();
            }
            else
            {
                total_stores++;
                if (write == "write-through")
                    write_through(p.second);
                else
                    write_back(p.second);
            }
            // printSet((p.second >> lognumbytes) % numsets);
        }
    }

    void replace(ll address)
    {
        int idx = (address >> bytewidth) % numsets;
        ll tag = address >> (bytewidth + setwidth);
        Block block(numwords);
        block.create(address, tag, numwords);
        if ((int)data[idx].size() >= numblocks)
        {
            Block old = data[idx].front();
            data[idx].erase(data[idx].begin());
            exists[idx][old.tag] = 0;
            if (old.dirty && (write == "write-back"))
                total_cycles += block_miss_penalty;
        }
        data[idx].push_back(block);
        exists[idx][tag] = 1;
    }

    void read(ll address)
    {
        int idx = (address >> bytewidth) % numsets;
        ll tag = address >> (bytewidth + setwidth);

        if (exists[idx][tag])
        {
            load_hits++;
            total_cycles += hit_time;
        }
        else
        {
            load_misses++;
            total_cycles += hit_time + block_miss_penalty;
            replace(address);
        }
    }

    void write_through(ll address)
    {
        int idx = (address >> bytewidth) % numsets;
        ll tag = address >> (bytewidth + setwidth);

        if (exists[idx][tag])
        {
            store_hits++;
            total_cycles += hit_time + miss_penalty;
        }
        else
        {
            store_misses++;
            if (allocate == "write-allocate")
            {
                total_cycles += hit_time + miss_penalty + block_miss_penalty;
                replace(address);
            }
            else
                total_cycles += miss_penalty;
        }
    }

    void write_back(ll address)
    {
        int idx = (address >> bytewidth) % numsets;
        ll tag = address >> (bytewidth + setwidth);

        if (exists[idx][tag])
        {
            store_hits++;
            total_cycles += hit_time;
            for (int i = 0; i < (int)data[idx].size(); ++i)
            {
                if (data[idx][i].tag == tag)
                {
                    data[idx][i].dirty = true;
                    break;
                }
            }
        }
        else
        {
            store_misses++;
            if (allocate == "write-allocate")
            {
                total_cycles += hit_time + block_miss_penalty;
                replace(address);
                for (int i = 0; i < (int)data[idx].size(); ++i)
                {
                    if (data[idx][i].tag == tag)
                    {
                        data[idx][i].dirty = true;
                        break;
                    }
                }
            }
            else
                total_cycles += miss_penalty;
        }
    }

    void printStats()
    {
        cout << "Total loads: " << total_loads << endl;
        cout << "Total stores: " << total_stores << endl;
        cout << "Load hits: " << load_hits << endl;
        cout << "Load misses: " << load_misses << endl;
        cout << "Store hits: " << store_hits << endl;
        cout << "Store misses: " << store_misses << endl;
        cout << "Total cycles: " << total_cycles << endl;
    }
};

class LRUCache
{
public:
    vector<list<Block>> data;
    vector<unordered_map<ll, list<Block>::iterator>> nodes;

    int numsets;
    int numblocks;
    int numwords;
    int bytewidth;
    int setwidth;
    string allocate;
    string write;

    int total_loads = 0;
    int total_stores = 0;
    int load_hits = 0;
    int load_misses = 0;
    int store_hits = 0;
    int store_misses = 0;
    ll total_cycles = 0;

    int hit_time = 1;
    int miss_penalty = 100;
    int block_miss_penalty;

    LRUCache(int numsets, int numblocks, int numwords, string allocate, string write)
    {
        this->numsets = numsets;
        this->numblocks = numblocks;
        this->numwords = numwords;
        this->bytewidth = log2(4 * numwords);
        this->setwidth = log2(numsets);
        this->allocate = allocate;
        this->write = write;

        this->block_miss_penalty = miss_penalty * numwords;

        data = vector<list<Block>>(numsets, list<Block>());
        nodes = vector<unordered_map<ll, list<Block>::iterator>>(numsets, unordered_map<ll, list<Block>::iterator>());
    }

    void operate(vector<pair<char, ll>> instructions)
    {
        for (pair<char, ll> p : instructions)
        {
            if (p.first == 'l')
            {
                total_loads++;
                read(p.second);
                // printSet((p.second>>2) % numsets);
                // printData();
            }
            else
            {
                total_stores++;
                if (write == "write-through")
                    write_through(p.second);
                else
                    write_back(p.second);
            }
            // printSet((p.second >> bytewidth) % numsets);
        }
    }

    void replace(ll address)
    {
        int idx = (address >> bytewidth) % numsets;
        ll tag = address >> (bytewidth + setwidth);
        Block block(numwords);
        block.create(address, tag, numwords);
        if ((int)data[idx].size() >= numblocks)
        {
            Block old = data[idx].front();
            data[idx].pop_front();
            nodes[idx].erase(old.tag);
            if (old.dirty && (write == "write-back"))
                total_cycles += block_miss_penalty;
        }
        data[idx].push_back(block);
        nodes[idx][tag] = --data[idx].end();
    }

    void read(ll address)
    {
        int idx = (address >> bytewidth) % numsets;
        ll tag = address >> (bytewidth + setwidth);

        if (nodes[idx].find(tag) != nodes[idx].end())
        {
            load_hits++;
            total_cycles += hit_time;

            data[idx].push_back(*nodes[idx][tag]);
            data[idx].erase(nodes[idx][tag]);
            nodes[idx][tag] = --data[idx].end();
        }
        else
        {
            load_misses++;
            total_cycles += hit_time + block_miss_penalty;
            replace(address);
        }
    }

    void write_through(ll address)
    {
        int idx = (address >> bytewidth) % numsets;
        ll tag = address >> (bytewidth + setwidth);

        if (nodes[idx].find(tag) != nodes[idx].end())
        {
            store_hits++;
            total_cycles += hit_time + miss_penalty;
            // total_cycles += hit_time + block_miss_penalty;
            data[idx].erase(nodes[idx][tag]);
            data[idx].push_back(*(nodes[idx][tag]));
            nodes[idx][tag] = --data[idx].end();
        }
        else
        {
            store_misses++;
            if (allocate == "write-allocate")
            {
                total_cycles += hit_time + miss_penalty + block_miss_penalty;
                replace(address);
                // cout << "WA WT\n";
            }
            else
                total_cycles += miss_penalty;
            // total_cycles += hit_time + block_miss_penalty
        }
    }

    void write_back(ll address)
    {
        int idx = (address >> bytewidth) % numsets;
        ll tag = address >> (bytewidth + setwidth);

        if (nodes[idx].find(tag) != nodes[idx].end())
        {
            store_hits++;
            total_cycles += hit_time;

            data[idx].push_back(*nodes[idx][tag]);
            data[idx].erase(nodes[idx][tag]);
            nodes[idx][tag] = --data[idx].end();
            nodes[idx][tag]->dirty = true;
        }
        else
        {
            store_misses++;
            if (allocate == "write-allocate")
            {
                total_cycles += hit_time + block_miss_penalty;
                replace(address);
                nodes[idx][tag]->dirty = true;
            }
            else
                total_cycles += miss_penalty;
        }
    }

    void printStats()
    {
        cout << "Total loads: " << total_loads << endl;
        cout << "Total stores: " << total_stores << endl;
        cout << "Load hits: " << load_hits << endl;
        cout << "Load misses: " << load_misses << endl;
        cout << "Store hits: " << store_hits << endl;
        cout << "Store misses: " << store_misses << endl;
        cout << "Total cycles: " << total_cycles << endl;
    }

    void printSet(int index)
    {
        cout << "Set " << index << ": " << total_loads + total_stores << endl;
        for (int j = 0; j < numblocks; j++)
        {
            cout << "Block " << j << ": ";
            for (Block el : data[index])
            {
                for (int i = 0; i < numwords; ++i)
                {
                    cout << el.words[i] << " ";
                }
                cout << "Dirty: " << el.dirty << " ";
            }
            cout << endl;
        }
    }
};

int main(int argc, char *argv[])
{
    if (argc < 7)
    {
        cerr << "Not enough arguments\n";
        exit(EXIT_FAILURE);
    }

    int numsets, numblocks, numbytes;
    string allocate, write, replacement;

    try
    {
        numsets = stoi(argv[1]);
        numblocks = stoi(argv[2]);
        numbytes = stoi(argv[3]);
        allocate = string(argv[4]);
        write = string(argv[5]);
        replacement = string(argv[6]);
    }
    catch (const invalid_argument &e)
    {
        cerr << "Invalid argument: " << e.what() << endl;
        exit(EXIT_FAILURE);
    }
    catch (const out_of_range &e)
    {
        cerr << "Argument out of range: " << e.what() << endl;
        exit(EXIT_FAILURE);
    }

    vector<pair<char, ll>> instructions;
    char ch;
    ll address;
    int extra;
    while (cin >> ch >> hex >> address >> extra)
        instructions.push_back(make_pair(ch, address));
    if (replacement == "lru")
    {
        LRUCache c = LRUCache(numsets, numblocks, (numbytes / 4), allocate, write);
        c.operate(instructions);
        c.printStats();
    }
    else
    {
        FIFOCache c = FIFOCache(numsets, numblocks, (numbytes / 4), allocate, write);
        c.operate(instructions);
        c.printStats();
    }
    return 0;
}