#include "item.h"

bool operator== (const Item& i1, const Item& i2)
{
    return *i1.Parent_ == *i2.Parent_ &&
        i1.Title_ == i2.Title_ &&
        i1.Link_ == i2.Link_ &&
        i1.Description_ == i2.Description_ &&
        i1.Author_ == i2.Author_ &&
        i1.Category_ == i2.Category_ &&
        i1.Comments_ == i2.Comments_ &&
        i1.Guid_ == i2.Guid_ &&
        i1.PubDate_ == i2.PubDate_;
}

