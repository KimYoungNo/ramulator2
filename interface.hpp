#ifndef __RAMULATOR_INTERFACE_HPP__
#define __RAMULATOR_INTERFACE_HPP__

#include <deque>
#include <queue>
#include <string>
#include <vector>
#include <cstdint>
#include <functional>
#include <unordered_map>

#include "base/base.h"
#include "base/config.h"
#include "base/request.h"
#include "frontend/frontend.h"
#include "memory_system/memory_system.h"

class Ramulator::IFrontEnd;
class Ramulator::IMemorySystem;

namespace ramulator2
{

struct mem_fetch
{
    uint64_t addr;
    bool write;
    bool request;
    void *origin_data;
    int size;

    void set_reply() { this->request = false; }
    bool is_write() const { return this->write; }
};

class Interface
{
private:
    bool is_gpu;

    std::string std_name;
    std::string config_path;

    std::queue<mem_fetch *> request_queue;
    std::queue<mem_fetch *> return_queue;

    Ramulator::IFrontEnd *ramulator2_frontend;
    Ramulator::IMemorySystem *ramulator2_memorysystem;

    int memory_id;
    int num_channels;
    int num_reqs;
    int num_reads;
    int num_writes;
    int nbl;
    int tot_reqs;
    int tot_reads;
    int tot_writes;
    int log_interval = 10000;
    uint64_t cycle_count = 0;
    
public:
    Interface() = default;
    Interface(unsigned int memory_id, unsigned int num_channels,
              std::string ramulator_config, std::string out,
              int log_interval, int nbl)
        : memory_id(memory_id), num_channels(num_channels),
          config_path(ramulator_config), log_interval(log_interval), nbl(nbl)
    { this->init(); }

    ~Interface() = default;

    void init();
    void cycle();
    void finish();
    
    void push(mem_fetch *mf)
    { request_queue.push(mf); }

    void print(FILE *fp=nullptr)
    { finish(); }

    mem_fetch *return_queue_pop()
    { mem_fetch* mf = return_queue.front();
      return_queue.pop();
      return mf; }

    mem_fetch *return_queue_top() const
    { if(return_queue.empty()) return nullptr; 
      return return_queue.front(); }

    bool is_full(int i=0) const
    { return (request_queue.size()+i) >= 256; }
};

} // namespace ndp
#endif