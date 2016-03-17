#include "Stats.hpp"

Stats::Stats(std::string working_dir, std::string server_address)
{
    this->working_dir = working_dir;
    this->server_address = server_address;
}

void Stats::logEvent(std::string event_name, std::string event_data)
{

}
