#ifndef CHANNEL_H
#define CHANNEL_H
#include <QString>

struct Channel
{
    QString Title_;
    QString Link_;
    QString Description_;
};

bool operator== (const Channel&, const Channel&);
bool operator< (const Channel&, const Channel&);

#endif

