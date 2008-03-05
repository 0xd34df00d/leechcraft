#ifndef CHANNEL_H
#define CHANNEL_H
#include <QString>

struct Channel
{
    QString Title_;
    QString Link_;
    QString Description_;
};

bool operator== (const Channel& c1, const Channel& c2);

#endif

