#include "Crashup.hpp"

namespace crashup {

Crashup::Crashup(std::string working_dir, std::string server_address) :
    _stats(working_dir, server_address)
{
    this->working_dir = working_dir;
    this->server_address = server_address;
}

Stats &Crashup::stats()
{
    return _stats;
}
};
