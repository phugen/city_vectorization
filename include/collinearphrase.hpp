#ifndef COLLINEARPHRASE_HPP
#define COLLINEARPHRASE_HPP

#include "include/collineargroup.hpp"

class CollinearPhrase
{
public:
    CollinearPhrase();
    ~CollinearPhrase();

    // returns number of words in this phrase
    int size();

    std::vector<CollinearGroup> words; // contains pointers to all words that are part of this phrase
};

#endif // COLLINEARPHRASE_HPP

