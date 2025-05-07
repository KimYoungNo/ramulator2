#include "interface.hpp"

namespace ramulator2
{

void Interface::init()
{
    this->cycle_count = 0;
    this->num_reads = 0;
    this->num_writes = 0;
    this->num_reqs = 0;
    this->tot_reads = 0;
    this->tot_writes = 0;

    YAML::Node config =
        Ramulator::Config::parse_config_file(config_path, {});

    this->ramulator2_frontend
        = Ramulator::Factory::create_frontend(config);
    this->ramulator2_memorysystem
        = Ramulator::Factory::create_memory_system(config);
    this->ramulator2_frontend->connect_memory_system(ramulator2_memorysystem);
    this->ramulator2_memorysystem->connect_frontend(ramulator2_frontend);

    const YAML::Node& ifce_config = config["MemorySystem"]["DRAM"];
    std::string impl_name = ifce_config["impl"].as<std::string>("");
    this->std_name = impl_name + "-CH_" + std::to_string(memory_id);
}

void Interface::finish()
{
    this->ramulator2_frontend->finalize();
    this->ramulator2_memorysystem->finalize();

    if(memory_id == 0)
        spdlog::info("{}: avg BW utilization {}% ({} reads, {} writes)", std_name,
                    (tot_reads + tot_writes) * 100 * nbl / (cycle_count), tot_reads,
                     tot_writes);
    else
        spdlog::debug("{}: avg BW utilization {}% ({} reads, {} writes)", std_name,
                     (tot_reads + tot_writes) * 100 * nbl / (cycle_count), tot_reads,
                      tot_writes);
    this->num_reads = 0;
    this->num_writes = 0;
}

void Interface::cycle()
{
    if (!request_queue.empty())
    {
        mem_fetch *mf = this->request_queue.front();
        auto callback = [this, mf](Ramulator::Request& req)
        {
            if (req.type_id == Ramulator::Request::Type::Read) {
                ++(this->num_reads); ++(this->tot_reads);
            } else {
                ++(this->num_writes); ++(this->tot_writes);
            }
            mf->set_reply();
            this->return_queue.push(mf);
        };
        bool success = ramulator2_frontend->receive_external_requests(
            mf->is_write() ? 1 : 0, mf->addr, 0, callback);

        if(success)
            this->request_queue.pop();
    }
    this->ramulator2_memorysystem->tick();

    if((cycle_count%log_interval) == 0)
    {
        if(memory_id == 0)
            spdlog::info("{}: BW utilization {}% ({} reads, {} writes)",
                         std_name,
                        (num_reads + num_writes) * 100 *nbl / (log_interval),
                         num_reads, num_writes);
        else
            spdlog::debug("{}: BW utilization {}% ({} reads, {} writes)",
                         std_name,
                        (num_reads + num_writes) * 100 *nbl / (log_interval),
                         num_reads, num_writes);
        (this->num_reads) = 0;
        (this->num_writes) = 0;
    }
    ++(this->cycle_count);
}

} // namespace ndp