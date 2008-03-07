#include "feed.h"

bool operator< (const Feed& f1, const Feed& f2)
{
    return f1.URL_ < f2.URL_;
}

