#include "channel.h"

bool operator== (const Channel& c1, const Channel& c2)
{
    return c1.Title_ == c2.Title_ &&
        c1.Link_ == c2.Link_;
}

Channel& Channel::operator= (const Channel& c)
{
    Title_ = c.Title_;
    Link_ = c.Link_;
    Description_ = c.Description_;
    LastBuild_ = c.LastBuild_;
    Items_ = c.Items_;

    return *this;
}

