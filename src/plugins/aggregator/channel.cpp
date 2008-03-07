#include "channel.h"

bool operator== (const Channel& c1, const Channel& c2)
{
    return c1.Title_ == c2.Title_ &&
        c1.Link_ == c2.Link_ &&
        c1.Description_ == c2.Description_;
}

bool operator< (const Channel& c1, const Channel& c2)
{
    return c1.Link_ < c2.Link_;
}

