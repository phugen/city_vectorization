#include "include/collineargroup.hpp"

CollinearGroup::CollinearGroup()
{
    this->type = 'i';
}

CollinearGroup::~CollinearGroup()
{

}

int CollinearGroup::size()
{
    return (int) this->chars.size();
}
